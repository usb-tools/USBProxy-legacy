/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
 * 
 * Based on libusb-gadget - Copyright 2009 Daiki Ueno <ueno@unixuser.org>
 *
 * This file is part of USB-MitM.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
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
 * USBHostProxyGadgetFS.cpp
 *
 * Created on: Nov 21, 2013
 */
#include "USBHostProxy_GadgetFS.h"
#include <cstring>
#include <unistd.h>
#include <poll.h>
#include "GadgetFS_helpers.h"
#include "errno.h"
#include "TRACE.h"

USBHostProxy_GadgetFS::USBHostProxy_GadgetFS(int _debugLevel) {
	mount_gadget();
	p_is_connected = false;
	p_device_file=0;
	debugLevel=_debugLevel;
	descriptor=NULL;
	descriptorLength=0;
	lastControl.bRequest=0;
	int i;
	for (i=0;i<16;i++) {
		p_epin_file[i]=0;
		p_epout_file[i]=0;
	}
}

USBHostProxy_GadgetFS::~USBHostProxy_GadgetFS() {
	if (p_device_file) {
		close(p_device_file);
		p_device_file=0;
	}
	int i;
	for (i=0;i<16;i++) {
		if (p_epin_file[i]) {
			close(p_epin_file[i]);
			p_epin_file[i]=0;
		}
		if (p_epout_file[i]) {
			close(p_epout_file[i]);
			p_epout_file[i]=0;
		}
	}
	if (descriptor) {
		free(descriptor);
		descriptor=NULL;
		descriptorLength=0;
	}
	unmount_gadget();
}

int USBHostProxy_GadgetFS::generate_descriptor(USBDevice* device) {
	char *ptr;
	int i;
	descriptor=(char*)malloc(USB_BUFSIZE);

	ptr = descriptor;
	/* tag for device descriptor format */
	ptr[0] = ptr[1] = ptr[2] = ptr[3] = 0;
	ptr += 4;

	for (i=1;i<=device->get_descriptor()->bNumConfigurations;i++) {
		char* header_ptr = ptr;
		USBConfiguration* cfg=device->get_configuration(i);
		if (cfg) {
			int length=cfg->get_full_descriptor_length();
			usb_config_descriptor* buf=(usb_config_descriptor*)(cfg->get_full_descriptor());
			buf->bmAttributes&=(~USB_CONFIG_ATT_WAKEUP);
			buf->wTotalLength=length;
			memcpy(ptr,buf,length);
			free(buf);
			ptr+=length;
		}
	}
	if (device->is_highspeed() && device->get_device_qualifier()) {
	  for (i=1;i<=device->get_descriptor()->bNumConfigurations;i++) {
		USBConfiguration* cfg=device->get_device_qualifier()->get_configuration(i);
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
	} else {
		for (i=1;i<=device->get_descriptor()->bNumConfigurations;i++) {
			char * header_ptr = ptr;
			USBConfiguration* cfg=device->get_configuration(i);
			if (cfg) {
				int length=cfg->get_full_descriptor_length();
				usb_config_descriptor* buf=(usb_config_descriptor*)(cfg->get_full_descriptor());
				buf->bmAttributes&=(~USB_CONFIG_ATT_WAKEUP);
				buf->wTotalLength=length;
				memcpy(ptr,buf,length);
				free(buf);
				ptr+=length;
			}
		}
	}
	memcpy(ptr, (char *)device->get_descriptor(), sizeof(usb_device_descriptor));
	ptr += sizeof(struct usb_device_descriptor);
	descriptorLength=ptr-descriptor;
	descriptor=(char*)realloc(descriptor,descriptorLength);
	return 0;
}


int USBHostProxy_GadgetFS::connect(USBDevice* device) {
	int i, status;

	if (p_is_connected) {fprintf(stderr,"GadgetFS already connected.\n"); return 0;}

	if (generate_descriptor(device)!=0) {return 1;}

	if (debugLevel>0) {
		for(i=0; i<descriptorLength; i++) {
			if(i%16 == 0)
				fprintf(stderr, "\n");
			fprintf(stderr, " %02x", descriptor[i]);
		}
		if(i%16 != 1)
			fprintf(stderr, "\n");
	}

	p_device_file = open_gadget();
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

int USBHostProxy_GadgetFS::reconnect() {
	int i, status;

	if (p_is_connected) {fprintf(stderr,"GadgetFS already connected.\n"); return 0;}
	if (!descriptor) {return 1;}

	if (debugLevel>0) {
		for(i=0; i<descriptorLength; i++) {
			if(i%8 == 0)
				fprintf(stderr, "\n");
			fprintf(stderr, " %02x", descriptor[i]);
		}
		if(i%8 != 0)
			fprintf(stderr, "\n");
	}

	p_device_file = open_gadget();
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

void USBHostProxy_GadgetFS::disconnect() {
	if (!p_is_connected) {fprintf(stderr,"GadgetFS not connected.\n"); return;}

	if (p_device_file) {
		close(p_device_file);
		p_device_file=0;
	}
	int i;
	for (i=0;i<16;i++) {
		if (p_epin_file[i]) {
			close(p_epin_file[i]);
			p_epin_file[i]=0;
		}
		if (p_epout_file[i]) {
			close(p_epout_file[i]);
			p_epout_file[i]=0;
		}
	}

	unmount_gadget();
	
	p_is_connected = false;
}

void USBHostProxy_GadgetFS::reset() {
	disconnect();
	reconnect();
}

bool USBHostProxy_GadgetFS::is_connected() {
	return p_is_connected;
}

#define NEVENT 5

//return 0 in usb_ctrlrequest->brequest if there is no request
int USBHostProxy_GadgetFS::control_request(usb_ctrlrequest *setup_packet, int *nbytes, __u8** dataptr) {
	struct usb_gadgetfs_event events[NEVENT];
	int ret, nevent, i;
	//FINISH we should make this timeout if possible, otherwise this relayer thread won't end nicely.
	//FINISH we also may not be able to do reads, because a read may be interpreted as an ack, which we may need to handle explicitly
	//FINISH this would likely include passing them through the relayer to the device so it can ack
	struct pollfd fds;
	fds.fd = p_device_file;
	fds.events = POLLIN;
	if (!poll(&fds, 1, 0) || !(fds.revents&POLLIN)) {
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
	if (debugLevel>1) fprintf(stderr, "libusb-gadget: %d events received\n", nevent);

	for (i = 0; i < nevent; i++) {
		if (debugLevel>0) fprintf(stderr,"libusb-gadget: event %d\n", events[i].type);
		switch (events[i].type) {
		case GADGETFS_SETUP:
			lastControl=events[i].u.setup;
			setup_packet->bRequestType=lastControl.bRequestType;
			setup_packet->bRequest=lastControl.bRequest;
			setup_packet->wIndex=lastControl.wIndex;
			setup_packet->wValue=lastControl.wValue;
			setup_packet->wLength=lastControl.wLength;
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
				debug (handle, 2, "libusb-gadget: connected with speed %d\n",
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


void USBHostProxy_GadgetFS::send_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8* dataptr,int length) {
	if (!endpoint) {
		int rc=write(p_device_file,dataptr,length);
		if (rc<0) {
			fprintf(stderr,"Fail on EP0 write %d %s\n",errno,strerror(errno));
		} else {
			fprintf(stderr,"Send %d bytes on EP0\n",rc);
		}
		return;
	}
	if (!(endpoint & 0x80)) {
		fprintf(stderr,"trying to send %d bytes on an out EP%d\n",length,endpoint);
		return;
	}
	int number=endpoint&0x0f;
	if (!p_epin_file[number]) {
		fprintf(stderr,"trying to send %d bytes on a non-open EP%d\n",length,endpoint);
		return;
	}

	int rc=write(p_epin_file[number],dataptr,length);
	if (rc<0) {
		fprintf(stderr,"Fail on EP%d write %d %s\n",number,errno,strerror(errno));
	} else {
		fprintf(stderr,"Send %d bytes on EP%d\n",rc,number);
	}
}

void USBHostProxy_GadgetFS::receive_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8** dataptr, int* length) {
	if (!endpoint) {
		fprintf(stderr,"trying to receive %d bytes on EP0\n",length);
		return;
	}
	if (endpoint & 0x80) {
		fprintf(stderr,"trying to receive %d bytes on an in EP%d\n",length,endpoint);
		return;
	}
	int number=endpoint&0x0f;
	if (!p_epout_file[number]) {
		fprintf(stderr,"trying to receive %d bytes on a non-open EP%d\n",length,endpoint);
		return;
	}

	//FIXME THIS DOES NOT WORK!!
	struct pollfd fds;
	fds.fd = p_epout_file[number];
	fds.events = POLLIN;
	if (!poll(&fds, 1, 0) || !(fds.revents&POLLIN)) {return;}

	*dataptr=(__u8*)malloc(USB_BUFSIZE);
	int rc=read(p_epout_file[number],*dataptr,USB_BUFSIZE);
	if (rc<0) {
		fprintf(stderr,"Fail on EP%d read %d %s\n",number,errno,strerror(errno));
	} else {
		*dataptr=(__u8*)realloc(*dataptr,rc);
		*length=rc;
		fprintf(stderr,"Read %d bytes on EP%d\n",rc,number);
	}

}

void USBHostProxy_GadgetFS::control_ack() {
	if (debugLevel) fprintf(stderr,"Sending ACK\n");
	if (lastControl.bRequestType&0x80) {
		write(p_device_file,0,0);
	} else {
		read(p_device_file,0,0);
	}
}

void USBHostProxy_GadgetFS::stall_ep(__u8 endpoint) {
	if (debugLevel) fprintf(stderr,"Stalling EP%d\n",endpoint);
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

void USBHostProxy_GadgetFS::setConfig(USBConfiguration* fs_cfg,USBConfiguration* hs_cfg,bool hs) {
TRACE;
	int ifc_idx;
	__u8 ifc_count=fs_cfg->get_descriptor()->bNumInterfaces;
	for (ifc_idx=0;ifc_idx<ifc_count;ifc_idx++) {
		USBInterface* fs_ifc=fs_cfg->get_interface(ifc_idx);
		USBInterface* hs_ifc=hs_cfg?hs_cfg->get_interface(ifc_idx):fs_ifc;
		hs_ifc=hs_ifc?hs_ifc:fs_ifc;
		__u8 ep_count=fs_ifc->get_endpoint_count();
		int ep_idx;
		for (ep_idx=0;ep_idx<ep_count;ep_idx++) {
			const usb_endpoint_descriptor* fs_ep=fs_ifc->get_endpoint_by_idx(ep_idx)->get_descriptor();
			const usb_endpoint_descriptor* hs_ep=(hs_ifc->get_endpoint_by_idx(ep_idx))?hs_ifc->get_endpoint_by_idx(ep_idx)->get_descriptor():fs_ep;
			__u8 bufSize=4+fs_ep->bLength+hs_ep->bLength;
			__u8* buf=(__u8*)calloc(1,bufSize);
			buf[0]=1;
			__u8* p=buf+4;

			memcpy(buf+4,fs_ep,fs_ep->bLength);
			memcpy(buf+4+fs_ep->bLength,hs_ep,hs_ep->bLength);

			__u8 epAddress=fs_ep->bEndpointAddress;

			int fd=open_endpoint(epAddress);
			if (fd<0) {
				fprintf(stderr,"Fail on open EP%d %d %s\n",epAddress,errno,strerror(errno));
				return;
			}
			if (epAddress & 0x80) {
				p_epin_file[epAddress&0x0f]=fd;
			} else {
				p_epout_file[epAddress&0x0f]=fd;
			}

			write(fd,buf,bufSize);
			free(buf);
			fprintf(stderr,"Opened EP%02x\n",epAddress);
		}
	}
}
