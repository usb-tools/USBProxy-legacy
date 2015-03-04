/*
 * This file is part of USBProxy.
 */

#include <iomanip> // setfill etc.
#include <sstream> // ostringstream
#include <iostream>

#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "Manager.h"
#include "TRACE.h"

#include "Device.h"
#include "DeviceQualifier.h"
#include "Endpoint.h"

#include "PluginManager.h"
#include "ConfigParser.h"

#include "DeviceProxy.h"
#include "HostProxy.h"
#include "PacketFilter.h"
#include "RelayReader.h"
#include "RelayWriter.h"
#include "Injector.h"

Manager::Manager(unsigned debug_level)
	: _debug_level(debug_level)
{
	status=USBM_IDLE;
	plugin_manager = new PluginManager();
	deviceProxy=NULL;
	hostProxy=NULL;
	device=NULL;
	filters=NULL;
	filterCount=0;
	injectors=NULL;
	injectorCount=0;

	int i;
	for(i=0;i<16;i++) {
		in_endpoints[i]=NULL;
		in_readers[i]=NULL;
		in_writers[i]=NULL;

		out_endpoints[i]=NULL;
		out_readers[i]=NULL;
		out_writers[i]=NULL;
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
		if (in_readers[i]) {
			if (in_readerThreads[i].joinable()) {
				in_readers[i]->please_stop();
				in_readerThreads[i].join();
			}
			delete(in_readers[i]);
			in_readers[i]=NULL;
		}

		if (in_writers[i]) {
			if (in_writerThreads[i].joinable()) {
				in_writers[i]->please_stop();
				in_writerThreads[i].join();
			}
			delete(in_writers[i]);
			in_writers[i]=NULL;
		}

		if (out_readers[i]) {
			if (out_readerThreads[i].joinable()) {
				out_readers[i]->please_stop();
				out_readerThreads[i].join();
			}
			delete(out_readers[i]);
			out_readers[i]=NULL;
		}

		if (out_writers[i]) {
			if (out_writerThreads[i].joinable()) {
				out_writers[i]->please_stop();
				out_writerThreads[i].join();
			}
			delete(out_writers[i]);
			out_writers[i]=NULL;
		}
	}
	for (i = 0; i < injectorCount; ++i)
		if (injectors[i])
			injectors[i]->please_stop();
	for (auto& i_thread: injectorThreads)
		i_thread.join();
	injectorThreads.clear();
	if (injectors) {
		free(injectors);
		injectors=NULL;
	}
}

void Manager::load_plugins(ConfigParser *cfg) {
	plugin_manager->load_plugins(cfg);
	deviceProxy = plugin_manager->device_proxy;
	if (deviceProxy)
		deviceProxy->debugLevel = _debug_level;
	hostProxy = plugin_manager->host_proxy;
	if (hostProxy)
		hostProxy->debugLevel = _debug_level;
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
	// modified 20141015 atsumi@aizulab.com for reset bust
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
	// modified 20141015 atsumi@aizulab.com for reset bust
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
	// modified 20141015 atsumi@aizulab.com for reset bust
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

// Converts an unsigned to a string with an uppercase hex number
// (same as using %02X in printf)
inline std::string shex(unsigned num)
{
	std::ostringstream os;
	os << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << num;
	return os.str();
}

void Manager::start_control_relaying(){

	//TODO this should exit immediately if already started, and wait (somehow) is stopping or setting up
	status=USBM_SETUP;

	//connect device proxy
	int rc=deviceProxy->connect();
	spinner(0);
	while (rc==ETIMEDOUT && status==USBM_SETUP) {
		spinner(1);
		rc=deviceProxy->connect();
	}
	if (rc!=0) {fprintf(stderr,"Unable to connect to device proxy.\n");status=USBM_IDLE;return;}

	//populate device model
	device=new Device(deviceProxy);
	device->print(0);

	// modified 20141007 atsumi@aizulab.com
  // I think interfaces are claimed soon after connecting device.
	//Claim interfaces
	Configuration* cfg;
	cfg=device->get_active_configuration();
	int ifc_cnt=cfg->get_descriptor()->bNumInterfaces;
	for (int i=0;i<ifc_cnt;i++) {
	 	deviceProxy->claim_interface(i);
	}

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

	//setup EP0 Reader & Writer
	out_readers[0]=new RelayReader(out_endpoints[0],hostProxy, _readersend, _writersend);
	out_writers[0]=new RelayWriter(out_endpoints[0],deviceProxy,this, _readersend, _writersend);

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
			if (out_endpoints[0] && injectors[i]->endpoint.test(out_endpoints[0])) {
/*
				mqname = "/USBProxy(" + std::to_string(getpid()) + ")-00-" + shex(i);
				mqd_t mq_out=mq_open(mqname.c_str(),O_RDWR | O_CREAT,S_IRWXU,&mqa);
				if (mq_out == -1) {
					std::cerr << "Error creating message queue '" << mqname << "'!\n";
					exit(1);
				}
				mqname = "/USBProxy(" + std::to_string(getpid()) + ")-80-" + shex(i);
				mqd_t mq_in=mq_open(mqname.c_str(),O_RDWR | O_CREAT,S_IRWXU,&mqa);
				if (mq_in == -1) {
					std::cerr << "Error creating message queue '" << mqname << "'!\n";
					exit(1);
				}
*/
				// injectors[i]->set_queue(0x80,mq_in); // TODO
				//out_writers[0]->set_send_queue(mq_in); // TODO
				//injectors[i]->set_queue(0, out_writers[0]->get_recv_queue()); // TODO
			}
		}
	}

	//create injector threads
	if (injectorCount) {
		injectorThreads.reserve(injectorCount);
		for(i=0;i<injectorCount;i++) {
			if (status!=USBM_SETUP) {stop_relaying();return;}
			injectorThreads.push_back(std::thread(&Injector::listen, injectors[i]));
		}
	}

	rc=hostProxy->connect(device);
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
		out_readerThreads[0] = std::thread(&RelayReader::relay_read, out_readers[0]);
	}
	if (status!=USBM_SETUP) {status=USBM_SETUP_ABORT;stop_relaying();return;}
	if (out_writers[0]) {
		out_writerThreads[0] = std::thread(&RelayWriter::relay_write, out_writers[0]);
	}
	if (status!=USBM_SETUP) {stop_relaying();return;}
	status=USBM_RELAYING;
}

void Manager::start_data_relaying() {
	//enumerate endpoints
	Configuration* cfg;
	cfg=device->get_active_configuration();

	int ifc_idx;
	int ifc_cnt=cfg->get_descriptor()->bNumInterfaces;
	for (ifc_idx=0;ifc_idx<ifc_cnt;ifc_idx++) {
		// modified 20141010 atsumi@aizulab.com
		// for considering alternate interface
		// begin
		int aifc_idx;
		int aifc_cnt = cfg->get_interface_alternate_count( ifc_idx);
		for ( aifc_idx=0; aifc_idx < aifc_cnt; aifc_idx++) {
			Interface* aifc=cfg->get_interface_alternate(ifc_idx, aifc_idx);
			int ep_idx;
			int ep_cnt=aifc->get_endpoint_count();
			for(ep_idx=0;ep_idx<ep_cnt;ep_idx++) {
				Endpoint* ep=aifc->get_endpoint_by_idx(ep_idx);
				const usb_endpoint_descriptor* epd=ep->get_descriptor();
				if (epd->bEndpointAddress & 0x80) { //IN EP
					in_endpoints[epd->bEndpointAddress&0x0f]=ep;
					in_queues[epd->bEndpointAddress&0x0f] = new PacketQueue;
				} else { //OUT EP
					out_endpoints[epd->bEndpointAddress&0x0f]=ep;
					out_queues[epd->bEndpointAddress&0x0f] = new PacketQueue;
				}
			}
		}
		// end
	}

	int i,j;
	for (i=1;i<16;i++) {
		if (in_endpoints[i]) {
			//RelayReader(Endpoint* _endpoint,Proxy* _proxy,mqd_t _queue);
			in_readers[i]=new RelayReader(in_endpoints[i],(Proxy*)deviceProxy, *in_queues[i]);
			//RelayWriter(Endpoint* _endpoint,Proxy* _proxy,mqd_t _queue);
			//in_writers[i]=new RelayWriter(in_endpoints[i],(Proxy*)hostProxy,mq);
			in_writers[i]=new RelayWriter(in_endpoints[i],(Proxy*)hostProxy, *in_queues[i]);
		}
		if (out_endpoints[i]) {
			//RelayReader(Endpoint* _endpoint,Proxy* _proxy,mqd_t _queue);
			//out_readers[i]=new RelayReader(out_endpoints[i],(Proxy*)hostProxy,mq);
			out_readers[i]=new RelayReader(out_endpoints[i],(Proxy*)hostProxy, *out_queues[i]);
			//RelayWriter(Endpoint* _endpoint,Proxy* _proxy,mqd_t _queue);
			out_writers[i]=new RelayWriter(out_endpoints[i],(Proxy*)deviceProxy, *out_queues[i]);
		}
	}

	//apply filters to relayers
	for(i=0;i<filterCount;i++) {
		if (filters[i]->device.test(device) && filters[i]->configuration.test(cfg)) {
			for (j=1;j<16;j++) {
				if (in_endpoints[j] && filters[i]->endpoint.test(in_endpoints[j]) && filters[i]->interface.test(in_endpoints[j]->get_interface())) {
					in_writers[j]->add_filter(filters[i]);
				}
				if (out_endpoints[j] && filters[i]->endpoint.test(out_endpoints[j]) && filters[i]->interface.test(out_endpoints[j]->get_interface())) {
					out_writers[j]->add_filter(filters[i]);
				}
			}
		}
	}

	//apply injectors to relayers
	for(i=0;i<injectorCount;i++) {
		if (injectors[i]->device.test(device) && injectors[i]->configuration.test(cfg)) {
			for (j=1;j<16;j++) {
				if (in_endpoints[j] && injectors[i]->endpoint.test(in_endpoints[j]) && injectors[i]->interface.test(in_endpoints[j]->get_interface())) {
/*
					mqname = "/USBProxy(" + std::to_string(getpid()) + ")-" + shex(j|0x80) + '-' + shex(i);
					mqd_t mq=mq_open(mqname.c_str(),O_RDWR | O_CREAT,S_IRWXU,&mqa);
					if (mq == -1) {
						std::cerr << "Error creating message queue '" << mqname << "'!\n";
						exit(1);
					}
*/
					//injectors[i]->set_queue(j|0x80,mq); TODO
					//in_writers[j]->add_queue(mq); // TODO
				}
				if (out_endpoints[j] && injectors[i]->endpoint.test(out_endpoints[j]) && injectors[i]->interface.test(out_endpoints[j]->get_interface())) {
/*
					mqname = "/USBProxy(" + std::to_string(getpid()) + ")-" + shex(j) + '-' + shex(i);
					mqd_t mq=mq_open(mqname.c_str(),O_RDWR | O_CREAT,S_IRWXU,&mqa);
					if (mq == -1) {
						std::cerr << "Error creating message queue '" << mqname << "'!\n";
						exit(1);
					}
*/
					//injectors[i]->set_queue(j,mq); // TODO
					// out_writers[j]->add_queue(mq); // TODO
				}
			}
		}
	}

	//Claim interfaces
	for (ifc_idx=0;ifc_idx<ifc_cnt;ifc_idx++) {
		deviceProxy->claim_interface(ifc_idx);
	}

	for(i=1;i<16;i++) {
		if (in_readers[i]) {
			in_readerThreads[i] = std::thread(&RelayReader::relay_read, in_readers[i]);
		}
		if (in_writers[i]) {
			in_writerThreads[i] = std::thread(&RelayWriter::relay_write, in_writers[i]);
		}
		if (out_readers[i]) {
			out_readerThreads[i] = std::thread(&RelayReader::relay_read, out_readers[i]);
		}
		if (out_writers[i]) {
			out_writerThreads[i] = std::thread(&RelayWriter::relay_write, out_writers[i]);
		}
	}
}

void Manager::stop_relaying(){
	if (status==USBM_SETUP) {status=USBM_SETUP_ABORT;return;}
	if (status!=USBM_RELAYING && status!=USBM_SETUP_ABORT) return;
	status=USBM_STOPPING;

	int i;
	//signal all injector threads to stop ASAP
	for(i=0;i<injectorCount;i++) {
		if (injectors[i]) injectors[i]->please_stop();
	}

	//signal all relayer threads to stop ASAP
	for(i=0;i<16;i++) {
		if (in_readerThreads[i].joinable()) {in_readers[i]->please_stop();}
		if (in_writerThreads[i].joinable()) {in_writers[i]->please_stop();}
		if (out_readerThreads[i].joinable()) {out_readers[i]->please_stop();}
		if (out_writerThreads[i].joinable()) {out_writers[i]->please_stop();}
	}

	//wait for all injector threads to stop
	for (auto& i_thread: injectorThreads)
		i_thread.join();
	injectorThreads.clear();


	//wait for all relayer threads to stop, then delete relayer objects
	for(i=0;i<16;i++) {
		if (in_endpoints[i]) {in_endpoints[i]=NULL;}
		if (in_readers[i]) {
			if (in_readerThreads[i].joinable()) {
				in_readerThreads[i].join();
			}
			delete(in_readers[i]);
			in_readers[i]=NULL;
		}
		if (in_writers[i]) {
			if (in_writerThreads[i].joinable()) {
				in_writerThreads[i].join();
			}
			delete(in_writers[i]);
			in_writers[i]=NULL;
		}

		if (out_endpoints[i]) {out_endpoints[i]=NULL;}
		if (out_readers[i]) {
			if (out_readerThreads[i].joinable()) {
				out_readerThreads[i].join();
			}
			delete(out_readers[i]);
			out_readers[i]=NULL;
		}
		if (out_writers[i]) {
			if (out_writerThreads[i].joinable()) {
				out_writerThreads[i].join();
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
	hostProxy->disconnect();

	//disconnect device proxy
	deviceProxy->disconnect();

	//clean up device model & endpoints
	if (device) {
		// modified 20141001 atsumi@aizulab.com
		// temporary debug because it's invalid pointer for free()
		// delete(device);
		device=NULL;
	}

	status=USBM_IDLE;
}

void Manager::setConfig(__u8 index) {
	device->set_active_configuration(index);
	DeviceQualifier* qualifier=device->get_device_qualifier();
	if (qualifier) {
		if (device->is_highspeed()) {
			deviceProxy->setConfig(device->get_device_qualifier()->get_configuration(index),device->get_configuration(index),true);
			hostProxy->setConfig(device->get_device_qualifier()->get_configuration(index),device->get_configuration(index),true);
		} else {
			deviceProxy->setConfig(device->get_configuration(index),device->get_device_qualifier()->get_configuration(index),false);
			hostProxy->setConfig(device->get_configuration(index),device->get_device_qualifier()->get_configuration(index),false);
		}
	} else {
		deviceProxy->setConfig(device->get_configuration(index),NULL,device->is_highspeed());
		hostProxy->setConfig(device->get_configuration(index),NULL,device->is_highspeed());
	}
	start_data_relaying();
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
