/*
 * This file is part of USBProxy.
 */

#include "HostProxy_GadgetFS.h"

#include <cstring>
#include <iostream>

#include <unistd.h>
#include <poll.h>
#include <math.h>

#include "GadgetFS_helpers.h"
#include "errno.h"
#include "TRACE.h"
#include "HexString.h"

#include "DeviceQualifier.h"
#include "Configuration.h"
#include "Interface.h"
#include "Endpoint.h"

HostProxy_GadgetFS::HostProxy_GadgetFS(ConfigParser *cfg)
	: HostProxy(*cfg)
{
	mount_gadget();
	p_is_connected = false;
	p_device_file=0;
	descriptor=NULL;
	descriptorLength=0;
	lastControl.bRequest=0;
	int i;
	for (i=0;i<16;i++) {
		p_epin_async[i]=NULL;
		p_epin_active[i]=false;
		p_epout_async[i]=NULL;
	}
}

HostProxy_GadgetFS::~HostProxy_GadgetFS() {
	if (p_is_connected)
		disconnect();
	if (descriptor) {
		free(descriptor);
		descriptor=NULL;
		descriptorLength=0;
	}
}

int HostProxy_GadgetFS::generate_descriptor(Device* device) {
	char *ptr;
	int i;
	descriptor=(char*)malloc(USB_BUFSIZE);

	ptr = descriptor;
	/* tag for device descriptor format */
	ptr[0] = ptr[1] = ptr[2] = ptr[3] = 0;
	ptr += 4;


	Configuration* cfg;
	for (i=1;i<=device->get_descriptor()->bNumConfigurations;i++) {
		if (device->is_highspeed() && device->get_device_qualifier()) {
			cfg=device->get_device_qualifier()->get_configuration(i);
		} else {
			cfg=device->get_configuration(i);
		}
		if (cfg) {
			int length=cfg->get_full_descriptor_length();
			usb_config_descriptor* buf=(usb_config_descriptor*)(cfg->get_full_descriptor());
			buf->bDescriptorType=USB_DT_CONFIG;
			buf->bmAttributes&=(~USB_CONFIG_ATT_WAKEUP);
			buf->wTotalLength=length;
			memcpy(ptr,buf,length);
			free(buf);
			ptr+=length;
		}
	}

	for (i=1;i<=device->get_descriptor()->bNumConfigurations;i++) {
		if (!device->is_highspeed() && device->get_device_qualifier()) {
			cfg=device->get_device_qualifier()->get_configuration(i);
		} else {
			cfg=device->get_configuration(i);
		}
		if (cfg) {
			int length=cfg->get_full_descriptor_length();
			usb_config_descriptor* buf=(usb_config_descriptor*)(cfg->get_full_descriptor());
			buf->bDescriptorType=USB_DT_CONFIG;
			buf->bmAttributes&=(~USB_CONFIG_ATT_WAKEUP);
			buf->wTotalLength=length;
			/* Adjust polling rate for high speed descriptor */
			if(!device->is_highspeed()) {
			    char* pointer = (char*) buf;
			    pointer += buf->bLength;    //move to end of cfg desc

			    for(unsigned int k=0;k<buf->bNumInterfaces;k++){    
				//move to end of intf desc
				pointer += cfg->get_interface(k)->get_descriptor()->bLength;

				if(cfg->get_interface(k)->has_HID()){
				    pointer+= cfg->get_interface(k)->get_HID_descriptor_length();
				}

				for(int gen_idx = 0; gen_idx < cfg->get_interface(k)->get_generic_descriptor_count();gen_idx++){
				    pointer+= cfg->get_interface(k)->get_generic_descriptor(gen_idx)->bLength;
				}

				for(int ep_idx = 0; ep_idx < cfg->get_interface(k)->get_descriptor()->bNumEndpoints;ep_idx++){
				    usb_endpoint_descriptor* epd = (usb_endpoint_descriptor*) pointer;

				    // conversion is: newValue = log2(8*oldValue)+1
				    int newValue = (log10(8*(epd->bInterval))/log10(2)) + 1;
				    fprintf(stderr,"old bInterval: %02X\ncalculated new bInterval: %02X\n",epd->bInterval,newValue);
				    memset(&epd->bInterval,newValue,1);
				    pointer+= epd->bLength;
				}
			    }
			}
			memcpy(ptr,buf,length);
			free(buf);
			ptr+=length;
		}
	}

	memcpy(ptr, (char *)device->get_descriptor(), sizeof(usb_device_descriptor));
	ptr += sizeof(struct usb_device_descriptor);
	descriptorLength=ptr-descriptor;
	descriptor=(char*)realloc(descriptor,descriptorLength);
	return 0;
}


int HostProxy_GadgetFS::connect(Device* device,int timeout) {
	int status;

	if (p_is_connected) {fprintf(stderr,"GadgetFS already connected.\n"); return 0;}

	if (generate_descriptor(device)!=0) {return 1;}

	if (debugLevel>0) {
		char* hex=hex_string((void*)descriptor,descriptorLength);
		fprintf(stderr,"%s\n",hex);
		free(hex);
	}

	device_filename = find_gadget_filename();
	p_device_file = open_gadget(device_filename);
	if (p_device_file < 0) {
		fprintf(stderr,"Fail on open %d %s\n",errno,strerror(errno));
		return 1;
	}

	status = write(p_device_file, descriptor, descriptorLength);
	if (status < 0) {
		fprintf(stderr,"Fail on write %d %s\n",errno,strerror(errno));
		close(p_device_file);
		p_device_file=0;
		return 1;
	}

	p_is_connected = true;
	return 0;
}

int HostProxy_GadgetFS::reconnect() {
	int status;

	if (p_is_connected) {fprintf(stderr,"GadgetFS already connected.\n"); return 0;}
	if (!descriptor) {return 1;}

	if (debugLevel>0) {
		char* hex=hex_string((void*)descriptor,descriptorLength);
		fprintf(stderr,"%s\n",hex);
		free(hex);
	}

	device_filename = find_gadget_filename();
	p_device_file = open_gadget(device_filename);
	if (p_device_file < 0) {
		fprintf(stderr,"Fail on open %d %s\n",errno,strerror(errno));
		return 1;
	}

	status = write(p_device_file, descriptor, descriptorLength);
	if (status < 0) {
		fprintf(stderr,"Fail on write %d %s\n",errno,strerror(errno));
		close(p_device_file);
		p_device_file=0;
		return 1;
	}

	p_is_connected = true;
	return 0;
}

void HostProxy_GadgetFS::disconnect() {
	if (!p_is_connected) {fprintf(stderr,"GadgetFS not connected.\n"); return;}

	if (p_device_file) {
		close(p_device_file);
		p_device_file=0;
	}
	int i;
	for (i=0;i<16;i++) {
		if (p_epin_async[i]) {
			aiocb* aio=p_epin_async[i];
			if (p_epin_active[i]) {aio_cancel(aio->aio_fildes,aio);}
			if (aio->aio_fildes) {close(aio->aio_fildes);aio->aio_fildes=0;}
			if (aio->aio_buf) {free((void*)(aio->aio_buf));aio->aio_buf=NULL;}
			delete(aio);
			p_epin_async[i]=NULL;
		}
		if (p_epout_async[i]) {
			aiocb* aio=p_epout_async[i];
			aio_cancel(aio->aio_fildes,aio);
			if (aio->aio_fildes) {close(aio->aio_fildes);aio->aio_fildes=0;}
			if (aio->aio_buf) {free((void*)(aio->aio_buf));aio->aio_buf=NULL;}
			delete(aio);
			p_epout_async[i]=NULL;
		}
	}

	unmount_gadget();
	
	p_is_connected = false;
}

void HostProxy_GadgetFS::reset() {
	disconnect();
	reconnect();
}

bool HostProxy_GadgetFS::is_connected() {
	return p_is_connected;
}

#define NEVENT 5

//return 0 in usb_ctrlrequest->brequest if there is no request
int HostProxy_GadgetFS::control_request(usb_ctrlrequest *setup_packet, int *nbytes, __u8** dataptr,int timeout) {
	struct usb_gadgetfs_event events[NEVENT];
	int ret, nevent, i;
	struct pollfd fds;

	fds.fd = p_device_file;
	fds.events = POLLIN;
	if (!poll(&fds, 1, timeout) || !(fds.revents&POLLIN)) {
		setup_packet->bRequest=0;
		return 0;
	}

	ret = read (p_device_file, &events, sizeof(events));
	if (ret < 0) {
		setup_packet->bRequest=0;
		fprintf(stderr,"Read error %d\n",ret);
		return 0;
	}

	nevent = ret / sizeof(events[0]);
	if (debugLevel>1) fprintf(stderr, "gadgetfs: %d events received\n", nevent);

	for (i = 0; i < nevent; i++) {
		if (debugLevel>0 && events[i].type!=GADGETFS_SETUP) fprintf(stderr,"gadgetfs: event %d\n", events[i].type);
		switch (events[i].type) {
		case GADGETFS_SETUP:
			lastControl=events[i].u.setup;
			setup_packet->bRequestType=lastControl.bRequestType;
			setup_packet->bRequest=lastControl.bRequest;
			setup_packet->wIndex=lastControl.wIndex;
			setup_packet->wValue=lastControl.wValue;
			setup_packet->wLength=lastControl.wLength;
			if (lastControl.bRequest == USB_REQ_SET_CONFIGURATION
				&& !(lastControl.bRequestType&0x80))
			{
				control_ack();
			}

			if (!(lastControl.bRequestType&0x80) && lastControl.wLength) {
				*dataptr=(__u8*)malloc(lastControl.wLength);
				*nbytes=read(p_device_file,*dataptr,lastControl.wLength);
			} else {
				*dataptr=NULL;
				*nbytes=0;
			}
			return 0;
			break;
		case GADGETFS_NOP:
			break;
		case GADGETFS_CONNECT:
			/*
			if (handle->event_cb) {
				event.type = USG_EVENT_CONNECT;
				handle->speed = events[i].u.speed;
				debug (handle, 2, "gadgetfs: connected with speed %d\n",
				handle->speed);
				handle->event_cb (handle, &event, handle->event_arg);
			}
			*/
			break;
		case GADGETFS_DISCONNECT:
			/*
			if (handle->event_cb) {
				handle->speed = USB_SPEED_UNKNOWN;
				event.type = USG_EVENT_DISCONNECT;
				handle->event_cb (handle, &event, handle->event_arg);
			}
			*/
			break;
		case GADGETFS_SUSPEND:
			/*
			if (handle->event_cb) {
				event.type = USG_EVENT_SUSPEND;
				handle->event_cb (handle, &event, handle->event_arg);
			}
			*/
			break;
		default:
			break;
		}
	}

	setup_packet->bRequest=0;
	return 0;
}


void HostProxy_GadgetFS::send_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8* dataptr,int length) {
	if (!endpoint) {
		int rc=write(p_device_file,dataptr,length);
		if (rc<0) {
			fprintf(stderr,"Fail on EP00 write %d %s\n",errno,strerror(errno));
		} else {
			//fprintf(stderr,"Sent %d bytes on EP00\n",rc);
		}
		return;
	}
	if (!(endpoint & 0x80)) {
		fprintf(stderr,"trying to send %d bytes on an out EP%02x\n",length,endpoint);
		return;
	}
	int number=endpoint&0x0f;
	if (!p_epin_async[number]) {
		fprintf(stderr,"trying to send %d bytes on a non-open EP%02x\n",length,endpoint);
		return;
	}

	aiocb* aio=p_epin_async[number];
	aio->aio_buf=malloc(length);
	memcpy((void*)(aio->aio_buf),dataptr,length);
	aio->aio_nbytes=length;

	int rc=aio_write(aio);
	if (rc) {
		fprintf(stderr,"Error submitting aio for EP%02x %d %s\n",endpoint,errno,strerror(errno));
	} else {
		if (debugLevel > 2)
			std::cerr << "Submitted " << length << " bytes to gadgetfs EP" << std::hex << (unsigned)endpoint << std::dec << '\n';
		p_epin_active[number]=true;
	}
}

bool HostProxy_GadgetFS::send_wait_complete(__u8 endpoint,int timeout) {
	if (!endpoint) return true;
	if (!(endpoint & 0x80)) {
		fprintf(stderr,"trying to check send on an out EP%02x\n",endpoint);
		return false;
	}
	int number=endpoint&0x0f;
	if (!p_epin_async[number]) {
		fprintf(stderr,"trying to check send on a non-open EP%02x\n",endpoint);
		return false;
	}

	if (!p_epin_active[number]) return true;

	aiocb* aio=p_epin_async[number];

	int rc=aio_error(aio);
	if (rc==EINPROGRESS && timeout) {
		struct timespec ts;
		ts.tv_sec = timeout/1000;
		ts.tv_nsec = 1000000L * (timeout%1000);
		if (aio_suspend(&aio,1,&ts)) {
			rc=0;
		} else {
			rc=aio_error(aio);
		}
	}
	if (rc==EINPROGRESS) return false;
	free((void*)(aio->aio_buf));
	aio->aio_buf=NULL;
	aio->aio_nbytes=0;
	if (rc) {
		fprintf(stderr,"Error during async aio on EP %02x %d %s (%s)\n",endpoint,rc,strerror(rc), __func__);
		p_epin_active[number]=false;
		return true;
	} else {
		rc=aio_return(aio);
		if (!rc) return true;
		if (rc == -1) {
			std::cerr << "Bad aio_return (rc " << errno << ", '" << std::strerror(errno) << "')\n";
			return false;
		}
		if (debugLevel > 2)
			std::cerr << "Sent " << rc << " bytes to gadgetfs EP" << std::hex << (unsigned)endpoint << std::dec << '\n';
		p_epin_active[number]=false;
		return true;
	}
}

void HostProxy_GadgetFS::receive_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8** dataptr, int* length, int timeout) {
	if (!endpoint) {
		fprintf(stderr,"trying to receive %d bytes on EP00\n",*length);
		return;
	}
	if (endpoint & 0x80) {
		fprintf(stderr,"trying to receive %d bytes on an in EP%02x\n",*length,endpoint);
		return;
	}
	int number=endpoint&0x0f;
	if (!p_epout_async[number]) {
		fprintf(stderr,"trying to receive %d bytes on a non-open EP%02x\n",*length,endpoint);
		return;
	}

	aiocb* aio=p_epout_async[number];

	int rc=aio_error(aio);
	if (rc==EINPROGRESS && timeout) {
		struct timespec ts;
		ts.tv_sec = timeout/1000;
		ts.tv_nsec = 1000000L * (timeout%1000);
		if (aio_suspend(&aio,1,&ts)) {
			return;
		} else {
			rc=aio_error(aio);
		}
	}
	if (rc) {
		if (rc==EINPROGRESS) {return;}
		fprintf(stderr,"Error during async aio on EP %02x %d %s (%s)\n",endpoint,rc,strerror(rc), __func__);
	} else {
		rc=aio_return(aio);
		if (rc == -1) {
			std::cerr << "Bad aio_return (rc " << errno << ", '" << std::strerror(errno) << "')\n";
			return;
		}
		if (debugLevel > 2)
			std::cerr << "Received " << rc << " bytes from gadgetfs EP" << std::hex << (unsigned)endpoint << std::dec << '\n';
		*dataptr=(__u8*)malloc(rc);
		memcpy(*dataptr,(void*)(aio->aio_buf),rc);
		*length=rc;
		rc=aio_read(aio);
		if (rc) {
			delete(aio);fprintf(stderr,"Error submitting aio for EP%02x %d %s\n",endpoint,errno,strerror(errno));
			p_epout_async[number]=NULL;
		}
	}

}

void HostProxy_GadgetFS::control_ack() {
	if (debugLevel) fprintf(stderr,"Sending ACK\n");
	if (lastControl.bRequestType&0x80) {
		write(p_device_file,0,0);
	} else {
		read(p_device_file,0,0);
	}
}

void HostProxy_GadgetFS::stall_ep(__u8 endpoint) {
	if (debugLevel) fprintf(stderr,"Stalling EP%02x\n",endpoint);
	if (endpoint) {
		//FINISH for nonzero endpoint
	} else {
		if (lastControl.bRequestType&0x80) {
			read(p_device_file,0,0);
		} else {
			write(p_device_file,0,0);
		}
	}
}

void HostProxy_GadgetFS::setConfig(Configuration* fs_cfg,Configuration* hs_cfg,bool hs) {
	int ifc_idx, aifc_idx;
	__u8 ifc_count=fs_cfg->get_descriptor()->bNumInterfaces;
	for (ifc_idx=0;ifc_idx<ifc_count;ifc_idx++) {
		// modified 20141010 atsumi@aizulab.com
		// for considering alternate interface
		// begin
		int aifc_cnt = fs_cfg->get_interface_alternate_count( ifc_idx);
		for ( aifc_idx=0; aifc_idx < aifc_cnt; aifc_idx++) {
			Interface* fs_aifc=fs_cfg->get_interface_alternate(ifc_idx, aifc_idx);
			Interface* hs_aifc=hs_cfg?hs_cfg->get_interface_alternate(aifc_idx, aifc_idx):fs_aifc;
			hs_aifc=hs_aifc?hs_aifc:fs_aifc;
			__u8 ep_count=fs_aifc->get_endpoint_count();
			int ep_idx;
			for (ep_idx=0;ep_idx<ep_count;ep_idx++) {
				const usb_endpoint_descriptor* fs_ep=fs_aifc->get_endpoint_by_idx(ep_idx)->get_descriptor();
				const usb_endpoint_descriptor* hs_ep=(hs_aifc->get_endpoint_by_idx(ep_idx))?hs_aifc->get_endpoint_by_idx(ep_idx)->get_descriptor():fs_ep;
				__u8 bufSize=4+fs_ep->bLength+hs_ep->bLength;
				__u8* buf=(__u8*)calloc(1,bufSize);
				buf[0]=1;

				memcpy(buf+4,fs_ep,fs_ep->bLength);
				memcpy(buf+4+fs_ep->bLength,hs_ep,hs_ep->bLength);

				__u8 epAddress=fs_ep->bEndpointAddress;

				int fd=open_endpoint(epAddress, device_filename);
				if (fd<0) {
					fprintf(stderr,"Fail on open EP%02x %d %s\n",epAddress,errno,strerror(errno));
					free(buf);
					return;
				}
				int rc = write(fd, buf, bufSize);
				free(buf);
				if (rc != bufSize)
					std::cerr << "Error writing to EP 0x" << std::hex << epAddress << std::dec << '\n';
				aiocb* aio=new aiocb;
				std::memset(aio, 0, sizeof(struct aiocb));
				aio->aio_fildes = fd;
				aio->aio_sigevent.sigev_notify = SIGEV_NONE;
				if (epAddress & 0x80) {
					p_epin_async[epAddress&0x0f]=aio;
				} else {
					if (hs) {
						aio->aio_nbytes=(hs_ep->bmAttributes&0x02)?hs_ep->wMaxPacketSize:hs_ep->wMaxPacketSize;
					} else {
						aio->aio_nbytes=(fs_ep->bmAttributes&0x02)?fs_ep->wMaxPacketSize:fs_ep->wMaxPacketSize;
					}
					if (debugLevel > 2)
						std::cerr << "gadgetfs: max. packet size is " << aio->aio_nbytes << " bytes for EP" << std::hex << (unsigned)epAddress << std::dec << '\n';
					aio->aio_buf=malloc(aio->aio_nbytes);
					rc=aio_read(aio);
					if (rc) {
						delete(aio);fprintf(stderr,"Error submitting aio for EP%02x %d %s\n",epAddress,errno,strerror(errno));
					} else {
						p_epout_async[epAddress&0x0f]=aio;
					}
				}
				fprintf(stderr,"Opened EP%02x\n",epAddress);
			}
		}
		// end
	}
}

static HostProxy_GadgetFS *proxy;

extern "C" {
	HostProxy * get_hostproxy_plugin(ConfigParser *cfg) {
		proxy = new HostProxy_GadgetFS(cfg);
		return (HostProxy *) proxy;
	}
	
	void destroy_plugin() {
		delete proxy;
	}
}
