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
}

USBHostProxy_GadgetFS::~USBHostProxy_GadgetFS() {
	if (p_device_file) {
		close(p_device_file);
		p_device_file=0;
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
			memcpy(ptr,cfg->get_full_descriptor(),length);
			((usb_config_descriptor *)ptr)->bmAttributes=((usb_config_descriptor *)ptr)->bmAttributes & (~USB_CONFIG_ATT_WAKEUP);
			ptr+=length;
			((usb_config_descriptor *)header_ptr)->wTotalLength = __cpu_to_le16(ptr - header_ptr);
		}
	}
	if (device->is_highspeed() && device->get_device_qualifier()) {
	  for (i=1;i<=device->get_descriptor()->bNumConfigurations;i++) {
		USBConfiguration* cfg=device->get_device_qualifier()->get_configuration(i);
		if (cfg) {
			 int length=cfg->get_full_descriptor_length();
			memcpy(ptr,cfg->get_full_descriptor(),length);
			((usb_config_descriptor *)ptr)->bDescriptorType=USB_DT_CONFIG;
			((usb_config_descriptor *)ptr)->bmAttributes=((usb_config_descriptor *)ptr)->bmAttributes & (~USB_CONFIG_ATT_WAKEUP);
			ptr+=length;
		}
	  }
	} else {
		for (i=1;i<=device->get_descriptor()->bNumConfigurations;i++) {
			char * header_ptr = ptr;
			USBConfiguration* cfg=device->get_configuration(i);
			if (cfg) {
				int length=cfg->get_full_descriptor_length();
				memcpy(ptr,cfg->get_full_descriptor(),length);
				((usb_config_descriptor *)ptr)->bmAttributes=((usb_config_descriptor *)ptr)->bmAttributes & (~USB_CONFIG_ATT_WAKEUP);
				ptr+=length;
				((usb_config_descriptor *)header_ptr)->wTotalLength = __cpu_to_le16(ptr - header_ptr);
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
			if(i%8 == 0)
				fprintf(stderr, "\n");
			fprintf(stderr, " %02x", descriptor[i]);
		}
		if(i%8 != 0)
			fprintf(stderr, "\n");
	}

	p_device_file = find_gadget();
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

	p_device_file = find_gadget();
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
	
	close(p_device_file);
	p_device_file=0;
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
			//FINISH handle OUT setup with length>0
			//FINISH handle IN setup with length>0
			setup_packet->bRequestType=events[i].u.setup.bRequestType;
			setup_packet->bRequest=events[i].u.setup.bRequest;
			setup_packet->bRequest=events[i].u.setup.bRequest;
			setup_packet->wIndex=events[i].u.setup.wIndex;
			setup_packet->wValue=events[i].u.setup.wValue;
			setup_packet->wLength=events[i].u.setup.wLength;
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

	//FINISH
	setup_packet->bRequest=0;
	return 0;
}


void USBHostProxy_GadgetFS::send_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8* dataptr,int length) {
	fprintf(stderr,"trying to send %d bytes on EP %d\n",length,endpoint);
	//FINISH
}

void USBHostProxy_GadgetFS::receive_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8** dataptr, int* length) {
	//fprintf(stderr,"trying to receive %d bytes on EP %d\n",length,endpoint);
	//FINISH
}
