/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
 *
 * This file is part of USBProxy.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 *
 * Manager.cpp
 *
 * Created on: Nov 12, 2013
 */
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>

#include "Manager.h"
#include "TRACE.h"
#include "mqueue_helpers.h"

#include "Device.h"
#include "DeviceQualifier.h"
#include "Endpoint.h"
#include "Packet.h"

#include "PluginManager.h"
#include "ConfigParser.h"

#include "DeviceProxy.h"
#include "HostProxy.h"
#include "PacketFilter.h"
#include "RelayReader.h"
#include "RelayWriter.h"
#include "Injector.h"

#include "myDebug.h"

Manager::Manager() {
	haltSignal=0;
	status=USBM_IDLE;
	plugin_manager = new PluginManager();
	deviceProxy=NULL;
	hostProxy=NULL;
	device=NULL;
	filters=NULL;
	filterCount=0;
	injectors=NULL;
	injectorCount=0;
	injectorThreads=NULL;

	int i;
	for(i=0;i<16;i++) {
		in_endpoints[i]=NULL;
		in_readers[i]=NULL;
		in_writers[i]=NULL;
		in_readerThreads[i]=0;
		in_writerThreads[i]=0;

		out_endpoints[i]=NULL;
		out_readers[i]=NULL;
		out_writers[i]=NULL;
		out_readerThreads[i]=0;
		out_writerThreads[i]=0;
	}
}

Manager::~Manager() {
	if (device) {
		delete(device);
		device=NULL;
	}
	if (filters) {
		free(filters);
		filters=NULL;
	}

	int i;
	for (i=0;i<16;i++) {
		if (in_readerThreads[i]) {
			pthread_cancel(in_readerThreads[i]);
			in_readerThreads[i]=0;
		}
		if (in_readers[i]) {
			delete(in_readers[i]);
			in_readers[i]=NULL;
		}

		if (in_writerThreads[i]) {
			pthread_cancel(in_writerThreads[i]);
			in_writerThreads[i]=0;
		}
		if (in_writers[i]) {
			delete(in_writers[i]);
			in_writers[i]=NULL;
		}

		if (out_readerThreads[i]) {
			pthread_cancel(out_readerThreads[i]);
			out_readerThreads[i]=0;
		}
		if (out_readers[i]) {
			delete(out_readers[i]);
			out_readers[i]=NULL;
		}

		if (out_writerThreads[i]) {
			pthread_cancel(out_writerThreads[i]);
			out_writerThreads[i]=0;
		}
		if (out_writers[i]) {
			delete(out_writers[i]);
			out_writers[i]=NULL;
		}
	}
	if (injectorThreads) {
		for (i=0;i<injectorCount;i++) {
			if (injectorThreads[i]) {
				pthread_cancel(injectorThreads[i]);
				injectorThreads[i]=0;
			}
		}
		free(injectorThreads);
		injectorThreads=NULL;
	}
	if (injectors) {
		free(injectors);
		injectors=NULL;
	}
}

void Manager::load_plugins(ConfigParser *cfg) {
	plugin_manager->load_plugins(cfg);
	deviceProxy = plugin_manager->device_proxy;
	hostProxy = plugin_manager->host_proxy;
	for(std::vector<PacketFilter*>::iterator it = plugin_manager->filters.begin();
		it != plugin_manager->filters.end(); ++it) {
		add_filter(*it);
	}
	for(std::vector<Injector*>::iterator it = plugin_manager->injectors.begin();
		it != plugin_manager->injectors.end(); ++it) {
		add_injector(*it);
	}
}

void Manager::add_injector(Injector* _injector){
	if (status!=USBM_IDLE) {fprintf(stderr,"Can't add injectors unless manager is idle.\n");}
	if (injectors) {
		injectors=(Injector**)realloc(injectors,++injectorCount*sizeof(Injector*));
	} else {
		injectorCount=1;
		injectors=(Injector**)malloc(sizeof(Injector*));
	}
	injectors[injectorCount-1]=_injector;
}

void Manager::remove_injector(__u8 index,bool freeMemory){
	if (status!=USBM_IDLE && status != USBM_RESET) {fprintf(stderr,"Can't remove injectors unless manager is idle or reset.\n");}
	if (!injectors || index>=injectorCount) {fprintf(stderr,"Injector index out of bounds.\n");}
	if (freeMemory && injectors[index]) {delete(injectors[index]);/* not needed injectors[index]=NULL;*/}
	if (injectorCount==1) {
		injectorCount=0;
		free(injectors);
		injectors=NULL;
	} else {
		int i;
		for(i=index+1;i<injectorCount;i++) {
			injectors[i-1]=injectors[i];
		}
		injectors=(Injector**)realloc(injectors,--injectorCount*sizeof(Injector*));
	}
}

Injector* Manager::get_injector(__u8 index){
	if (!injectors || index>=injectorCount) {return NULL;}
	return injectors[index];
}

__u8 Manager::get_injector_count(){
	return injectorCount;
}

void Manager::add_filter(PacketFilter* _filter){
	if (status!=USBM_IDLE  && status != USBM_RESET) {fprintf(stderr,"Can't add filters unless manager is idle or reset.\n");}
	if (filters) {
		filters=(PacketFilter**)realloc(filters,++filterCount*sizeof(PacketFilter*));
	} else {
		filterCount=1;
		filters=(PacketFilter**)malloc(sizeof(PacketFilter*));
	}
	filters[filterCount-1]=_filter;
}

void Manager::remove_filter(__u8 index,bool freeMemory){
	dbgMessage(""); fprintf( stderr, "status=%d\n", status);
	if (status!=USBM_IDLE && status != USBM_RESET) {fprintf(stderr,"Can't remove filters unless manager is idle or reset.\n");}
	if (!filters || index>=filterCount) {fprintf(stderr,"Filter index out of bounds.\n");}
	if (freeMemory && filters[index]) {delete(filters[index]);/* not needed filters[index]=NULL;*/}
	if (filterCount==1) {
		filterCount=0;
		free(filters);
		filters=NULL;
	} else {
		int i;
		for(i=index+1;i<filterCount;i++) {
			filters[i-1]=filters[i];
		}
		filters=(PacketFilter**)realloc(filters,--filterCount*sizeof(PacketFilter*));
	}
}

PacketFilter* Manager::get_filter(__u8 index){
	if (!filters || index>=filterCount) {return NULL;}
	return filters[index];
}

__u8 Manager::get_filter_count(){
	return filterCount;
}

void spinner(int dir) {
	static int i;
	if (dir==0) {i=-1;return;}
	const char* spinchar="|/-\\";
	if (i==-1) {i=0;} else {putchar('\x8');}
	putchar(spinchar[i]);
	i+=dir;
	if (i<0) i=3;
	if (i>3) i=0;
	fflush(stdout);
}

void Manager::start_control_relaying(){
	clean_mqueue();

	haltSignal=SIGRTMIN;
	//TODO this should exit immediately if already started, and wait (somehow) is stopping or setting up
	status=USBM_SETUP;

	//connect device proxy
	dbgMessage(""); fprintf( stderr, "deviceProxy: %x\n", deviceProxy);
	int rc=deviceProxy->connect();
	spinner(0);
	while (rc==ETIMEDOUT && status==USBM_SETUP) {
		spinner(1);
		rc=deviceProxy->connect();
	}
	if (rc!=0) {fprintf(stderr,"Unable to connect to device proxy.\n");status=USBM_IDLE;return;}

	//populate device model
	dbgMessage("");
	device=new Device(deviceProxy);
	dbgMessage("");
	device->print(0);

	dbgMessage("");
	if (status!=USBM_SETUP) {stop_relaying();return;}

	//create EP0 endpoint object
	usb_endpoint_descriptor desc_ep0;
	desc_ep0.bLength=7;
	desc_ep0.bDescriptorType=USB_DT_ENDPOINT;
	desc_ep0.bEndpointAddress=0;
	desc_ep0.bmAttributes=0;
	desc_ep0.wMaxPacketSize=device->get_descriptor()->bMaxPacketSize0;
	desc_ep0.bInterval=0;
	out_endpoints[0]=new Endpoint((Interface*)NULL,&desc_ep0);

	if (status!=USBM_SETUP) {stop_relaying();return;}
	//setup EP0 message queues
	char mqname[16];
	struct mq_attr mqa;
	mqa.mq_maxmsg=1;
	mqa.mq_msgsize=4;
	sprintf(mqname,"/USBProxy(%d)-%02X-EP",getpid(),0);
	mqd_t mq_readersend=mq_open(mqname,O_RDWR | O_CREAT,S_IRWXU,&mqa);
	sprintf(mqname,"/USBProxy(%d)-%02X-EP",getpid(),0x80);
	mqd_t mq_writersend=mq_open(mqname,O_RDWR | O_CREAT,S_IRWXU,&mqa);

	if (status!=USBM_SETUP) {stop_relaying();return;}
	//setup EP0 Reader & Writer
	out_readers[0]=new RelayReader(out_endpoints[0],hostProxy,mq_readersend,mq_writersend);
	out_writers[0]=new RelayWriter(out_endpoints[0],deviceProxy,this,mq_readersend,mq_writersend);

	//apply filters to relayers
	int i;
	for(i=0;i<filterCount;i++) {
		if (status!=USBM_SETUP) {stop_relaying();return;}
		if (filters[i]->device.test(device)) {
			if (out_endpoints[0] && filters[i]->endpoint.test(out_endpoints[0]) ) {
				out_writers[0]->add_filter(filters[i]);
			}
		}
	}

	//apply injectors to relayers
	for(i=0;i<injectorCount;i++) {
		if (status!=USBM_SETUP) {stop_relaying();return;}
		if (injectors[i]->device.test(device)) {
			char mqname[16];
			struct mq_attr mqa;
			mqa.mq_maxmsg=1;
			mqa.mq_msgsize=4;
			if (out_endpoints[0] && injectors[i]->endpoint.test(out_endpoints[0])) {
				sprintf(mqname,"/USBProxy(%d)-%02X-%02X",getpid(),0,i);
				mqd_t mq_out=mq_open(mqname,O_RDWR | O_CREAT,S_IRWXU,&mqa);
				sprintf(mqname,"/USBProxy(%d)-%02X-%02X",getpid(),0x80,i);
				mqd_t mq_in=mq_open(mqname,O_RDWR | O_CREAT,S_IRWXU,&mqa);
				injectors[i]->set_queue(0x80,mq_in);
				injectors[i]->set_queue(0,mq_out);
				out_writers[0]->add_setup_queue(mq_out,mq_in);
			}
		}
	}

	//create injector threads
	if (injectorCount) {
		injectorThreads=(pthread_t *)calloc(injectorCount,sizeof(pthread_t));
		for(i=0;i<injectorCount;i++) {
			if (status!=USBM_SETUP) {stop_relaying();return;}
			injectors[i]->set_haltsignal(haltSignal);
			pthread_create(&injectorThreads[i],NULL,&Injector::listen_helper,injectors[i]);
		}
	}


	dbgMessage("");
	rc=hostProxy->connect(device);
	dbgMessage("");
	spinner(0);
	while (rc==ETIMEDOUT && status==USBM_SETUP) {
		spinner(1);
		rc=hostProxy->connect(device);
	}
	if (rc!=0) {
		status=USBM_SETUP_ABORT;
		stop_relaying();
		return;
	}

	if (out_readers[0]) {
		out_readers[0]->set_haltsignal(haltSignal);
		pthread_create(&out_readerThreads[0],NULL,&RelayReader::relay_read_helper,out_readers[0]);
	}
	if (status!=USBM_SETUP) {status=USBM_SETUP_ABORT;stop_relaying();return;}
	if (out_writers[0]) {
		out_writers[0]->set_haltsignal(haltSignal);
		pthread_create(&out_writerThreads[i],NULL,&RelayWriter::relay_write_helper,out_writers[0]);
	}
	if (status!=USBM_SETUP) {stop_relaying();return;}
	status=USBM_RELAYING;
}

void Manager::start_data_relaying() {
	dbgMessage("");
	//enumerate endpoints
	Configuration* cfg;
	cfg=device->get_active_configuration();
	int ifc_idx;
	dbgMessage("");
	int ifc_cnt=cfg->get_descriptor()->bNumInterfaces;
	dbgMessage("");
	for (ifc_idx=0;ifc_idx<ifc_cnt;ifc_idx++) {
		dbgMessage("");
		Interface* ifc=cfg->get_interface(ifc_idx);
		int ep_idx;
		dbgMessage(""); fprintf( stderr, "ifc=%x, ifc_idx=%d, ifc_cnt=%d\n", ifc, ifc_idx, ifc_cnt);
		int ep_cnt=ifc->get_endpoint_count();
		dbgMessage("");
		for(ep_idx=0;ep_idx<ep_cnt;ep_idx++) {
			dbgMessage("");
			Endpoint* ep=ifc->get_endpoint_by_idx(ep_idx);
			dbgMessage("");
			const usb_endpoint_descriptor* epd=ep->get_descriptor();
			dbgMessage("");
			if (epd->bEndpointAddress & 0x80) { //IN EP
				dbgMessage("");
				in_endpoints[epd->bEndpointAddress&0x0f]=ep;
			} else { //OUT EP
				dbgMessage("");
				out_endpoints[epd->bEndpointAddress&0x0f]=ep;
			}
			dbgMessage("");
		}
	}

	int i,j;
	dbgMessage("");
	for (i=1;i<16;i++) {
		char mqname[16];
		struct mq_attr mqa;
		mqa.mq_maxmsg=1;
		mqa.mq_msgsize=4;

		dbgMessage("");
		if (in_endpoints[i]) {
			dbgMessage("");
			sprintf(mqname,"/USBProxy(%d)-%02X-EP",getpid(),i|0x80);
			dbgMessage("");
			mqd_t mq=mq_open(mqname,O_RDWR | O_CREAT,S_IRWXU,&mqa);
			//RelayReader(Endpoint* _endpoint,Proxy* _proxy,mqd_t _queue);
			dbgMessage("");
			in_readers[i]=new RelayReader(in_endpoints[i],(Proxy*)deviceProxy,mq);
			//RelayWriter(Endpoint* _endpoint,Proxy* _proxy,mqd_t _queue);
			dbgMessage("");
			in_writers[i]=new RelayWriter(in_endpoints[i],(Proxy*)hostProxy,mq);
			dbgMessage("");
		}
		dbgMessage("");
		if (out_endpoints[i]) {
			dbgMessage("");
			sprintf(mqname,"/USBProxy(%d)-%02X-EP",getpid(),i);
			dbgMessage("");
			mqd_t mq=mq_open(mqname,O_RDWR | O_CREAT,S_IRWXU,&mqa);
			//RelayReader(Endpoint* _endpoint,Proxy* _proxy,mqd_t _queue);
			dbgMessage("");
			out_readers[i]=new RelayReader(out_endpoints[i],(Proxy*)hostProxy,mq);
			//RelayWriter(Endpoint* _endpoint,Proxy* _proxy,mqd_t _queue);
			dbgMessage("");
			out_writers[i]=new RelayWriter(out_endpoints[i],(Proxy*)deviceProxy,mq);
			dbgMessage("");
		}
		dbgMessage("");
	}

	//apply filters to relayers
	dbgMessage("");
	for(i=0;i<filterCount;i++) {
		dbgMessage("");
		if (filters[i]->device.test(device) && filters[i]->configuration.test(cfg)) {
			dbgMessage("");
			for (j=1;j<16;j++) {
				dbgMessage("");
				if (in_endpoints[j] && filters[i]->endpoint.test(in_endpoints[j]) && filters[i]->interface.test(in_endpoints[j]->get_interface())) {
					dbgMessage("");
					in_writers[j]->add_filter(filters[i]);
				}
				dbgMessage("");
				if (out_endpoints[j] && filters[i]->endpoint.test(out_endpoints[j]) && filters[i]->interface.test(out_endpoints[j]->get_interface())) {
					dbgMessage("");
					out_writers[j]->add_filter(filters[i]);
				}
				dbgMessage("");
			}
		}
	}

	//apply injectors to relayers
	dbgMessage("");
	for(i=0;i<injectorCount;i++) {
		dbgMessage("");
		if (injectors[i]->device.test(device) && injectors[i]->configuration.test(cfg)) {
			char mqname[16];
			struct mq_attr mqa;
			dbgMessage("");
			mqa.mq_maxmsg=1;
			mqa.mq_msgsize=4;
			dbgMessage("");
			for (j=1;j<16;j++) {
				dbgMessage("");
				if (in_endpoints[j] && injectors[i]->endpoint.test(in_endpoints[j]) && injectors[i]->interface.test(in_endpoints[j]->get_interface())) {
					dbgMessage("");
					sprintf(mqname,"/USBProxy(%d)-%02X-%02X",getpid(),j|0x80,i);
					dbgMessage("");
					mqd_t mq=mq_open(mqname,O_RDWR | O_CREAT,S_IRWXU,&mqa);
					dbgMessage("");
					injectors[i]->set_queue(j|0x80,mq);
					dbgMessage("");
					in_writers[j]->add_queue(mq);
					dbgMessage("");
				}
				dbgMessage("");
				if (out_endpoints[j] && injectors[i]->endpoint.test(out_endpoints[j]) && injectors[i]->interface.test(out_endpoints[j]->get_interface())) {
					dbgMessage("");
					sprintf(mqname,"/USBProxy(%d)-%02X-%02X",getpid(),j,i);
					dbgMessage("");
					mqd_t mq=mq_open(mqname,O_RDWR | O_CREAT,S_IRWXU,&mqa);
					dbgMessage("");
					injectors[i]->set_queue(j,mq);
					dbgMessage("");
					out_writers[j]->add_queue(mq);
					dbgMessage("");
				}
				dbgMessage("");
			}
		}
	}

	//Claim interfaces
	// deleted 20141002 atsumi@aizulab.com
	// claim_interface should be called when an endpoint in each interface.
	// dbgMessage("");
	// for (ifc_idx=0;ifc_idx<ifc_cnt;ifc_idx++) {
	//	dbgMessage("");
	//	deviceProxy->claim_interface(ifc_idx);
	// }

	dbgMessage("");
	for(i=1;i<16;i++) {
		dbgMessage("");
		if (in_readers[i]) {
			dbgMessage("");
			in_readers[i]->set_haltsignal(haltSignal);
			dbgMessage("");
			pthread_create(&in_readerThreads[i],NULL,&RelayReader::relay_read_helper,in_readers[i]);
			dbgMessage("");
		}
		dbgMessage("");
		if (in_writers[i]) {
			dbgMessage("");
			in_writers[i]->set_haltsignal(haltSignal);
			dbgMessage("");
			pthread_create(&in_writerThreads[i],NULL,&RelayWriter::relay_write_helper,in_writers[i]);
			dbgMessage("");
		}
		dbgMessage("");
		if (out_readers[i]) {
			dbgMessage("");
			out_readers[i]->set_haltsignal(haltSignal);
			dbgMessage("");
			pthread_create(&out_readerThreads[i],NULL,&RelayReader::relay_read_helper,out_readers[i]);
			dbgMessage("");
		}
		dbgMessage("");
		if (out_writers[i]) {
			dbgMessage("");
			out_writers[i]->set_haltsignal(haltSignal);
			dbgMessage("");
			pthread_create(&out_writerThreads[i],NULL,&RelayWriter::relay_write_helper,out_writers[i]);
			dbgMessage("");
		}
		dbgMessage("");
	}
	dbgMessage("");
}

void Manager::stop_relaying(){
	if (status==USBM_SETUP) {status=USBM_SETUP_ABORT;return;}
	if (status!=USBM_RELAYING && status!=USBM_SETUP_ABORT) return;
	status=USBM_STOPPING;

	int i;
	//signal all injector threads to stop ASAP
	for(i=0;i<injectorCount;i++) {
		if (injectorThreads && injectorThreads[i]) pthread_kill(injectorThreads[i],haltSignal);
	}

	//signal all relayer threads to stop ASAP
	for(i=0;i<16;i++) {
		if (in_readerThreads[i]) {pthread_kill(in_readerThreads[i],haltSignal);}
		if (in_writerThreads[i]) {pthread_kill(in_writerThreads[i],haltSignal);}
		if (out_readerThreads[i]) {pthread_kill(out_readerThreads[i],haltSignal);}
		if (out_writerThreads[i]) {pthread_kill(out_writerThreads[i],haltSignal);}
	}

	//wait for all injector threads to stop
	if (injectorThreads) {
		for(i=0;i<injectorCount;i++) {
			if (injectorThreads[i]) {
				pthread_join(injectorThreads[i],NULL);
				injectorThreads[i]=0;
			}
		}
		free(injectorThreads);
		injectorThreads=NULL;
	}


	//wait for all relayer threads to stop, then delete relayer objects
	for(i=0;i<16;i++) {
		if (in_endpoints[i]) {in_endpoints[i]=NULL;}
		if (in_readers[i]) {
			if (in_readerThreads[i]) {
				pthread_join(in_readerThreads[i],NULL);
				in_readerThreads[i]=0;
			}
			delete(in_readers[i]);
			in_readers[i]=NULL;
		}
		if (in_writers[i]) {
			if (in_writerThreads[i]) {
				pthread_join(in_writerThreads[i],NULL);
				in_writerThreads[i]=0;
			}
			delete(in_writers[i]);
			in_writers[i]=NULL;
		}

		if (out_endpoints[i]) {out_endpoints[i]=NULL;}
		if (out_readers[i]) {
			if (out_readerThreads[i]) {
				pthread_join(out_readerThreads[i],NULL);
				out_readerThreads[i]=0;
			}
			delete(out_readers[i]);
			out_readers[i]=NULL;
		}
		if (out_writers[i]) {
			if (out_writerThreads[i]) {
				pthread_join(out_writerThreads[i],NULL);
				out_writerThreads[i]=0;
			}
			delete(out_writers[i]);
			out_writers[i]=NULL;
		}
	}

	if (out_endpoints[0]) {delete(out_endpoints[0]);out_endpoints[0]=NULL;}

	//Release interfaces
	int ifc_idx;
	if (device) {
		Configuration* cfg=device->get_active_configuration();
		int ifc_cnt=cfg->get_descriptor()->bNumInterfaces;
		for (ifc_idx=0;ifc_idx<ifc_cnt;ifc_idx++) {
			deviceProxy->release_interface(ifc_idx);
		}
	}

	//disconnect from host
	dbgMessage("");
	hostProxy->disconnect();

	//disconnect device proxy
	dbgMessage("");
	deviceProxy->disconnect();

	//clean up device model & endpoints
	dbgMessage("");
	if (device) {
		dbgMessage("");
		// modified 20141001 atsumi@aizulab.com
		// temporary debug because it's invalid pointer for free()
		// delete(device);
		dbgMessage("");
		device=NULL;
	}

	dbgMessage("");
	clean_mqueue();

	dbgMessage("");
	status=USBM_IDLE;
}

void Manager::setConfig(__u8 index) {
	dbgMessage("");
	device->set_active_configuration(index);
	dbgMessage("");
	DeviceQualifier* qualifier=device->get_device_qualifier();
	dbgMessage("");
	if (qualifier) {
		dbgMessage("");
		if (device->is_highspeed()) {
			dbgMessage("");
			deviceProxy->setConfig(device->get_device_qualifier()->get_configuration(index),device->get_configuration(index),true);
			dbgMessage("");
			hostProxy->setConfig(device->get_device_qualifier()->get_configuration(index),device->get_configuration(index),true);
			dbgMessage("");
		} else {
			dbgMessage("");
			deviceProxy->setConfig(device->get_configuration(index),device->get_device_qualifier()->get_configuration(index),false);
			dbgMessage("");
			hostProxy->setConfig(device->get_configuration(index),device->get_device_qualifier()->get_configuration(index),false);
			dbgMessage("");
		}
	} else {
		dbgMessage("");
		deviceProxy->setConfig(device->get_configuration(index),NULL,device->is_highspeed());
		dbgMessage("");
		hostProxy->setConfig(device->get_configuration(index),NULL,device->is_highspeed());
		dbgMessage("");
	}
	dbgMessage("");
	start_data_relaying();
	dbgMessage("");
}

/* Delete all injectors and filters - easier to manage */
void Manager::cleanup() {
	while(injectorCount)
		remove_injector(injectorCount-1, true);
	while(filterCount)
		remove_filter(filterCount-1, true);
	delete deviceProxy;
	deviceProxy = NULL;
	delete hostProxy;
	hostProxy = NULL;
}
