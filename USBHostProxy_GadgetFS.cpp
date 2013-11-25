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
#include "TRACE.h"
#include "GadgetFS_helpers.h"
#include <fcntl.h>
#include <unistd.h>
#include "errno.h"

/* gadgetfs currently has no chunking (or O_DIRECT/zerocopy) support
 * to turn big requests into lots of smaller ones; so this is "small".
 */
#define	USB_BUFSIZE	(7 * 1024)

USBHostProxy_GadgetFS::USBHostProxy_GadgetFS(const char * _device_path) {
	device_path = _device_path;
	p_is_connected = false;
	//Check path exists, permissions, etc
}

USBHostProxy_GadgetFS::~USBHostProxy_GadgetFS() {
	//FINISH
	
	//FINISH - check if it's open
	close(p_device_file);
}

int USBHostProxy_GadgetFS::connect(USBDevice* device) {
	int i, status;
	char *ptr;
	const char *device_filename;
	const usb_device_descriptor *device_descriptor;
	char descriptor_buf[USB_BUFSIZE];

	if (p_is_connected) {fprintf(stderr,"GadgetFS already connected.\n"); return 0;}

	device_descriptor = device->get_descriptor();
	if (device_descriptor == NULL) {
		fprintf(stderr,"Error, unable to fetch device descriptor.\n");
		return 1;
	}

	ptr = descriptor_buf;
	/* tag for device descriptor format */
	ptr[0] = ptr[1] = ptr[2] = ptr[3] = 0;
	ptr += 4;


	/* FINISH: Add error checking */
	for (i=1;i<=device->get_descriptor()->bNumConfigurations;i++) {
		//	header_ptr = ptr;
		int length=device->get_configuration(i)->get_full_descriptor_length();
		memcpy(ptr,device->get_configuration(i)->get_full_descriptor(),length);
		TRACE1(((usb_config_descriptor *)ptr)->bmAttributes)
		((usb_config_descriptor *)ptr)->bmAttributes=((usb_config_descriptor *)ptr)->bmAttributes & (~USB_CONFIG_ATT_WAKEUP);
		TRACE1(((usb_config_descriptor *)ptr)->bmAttributes)
		ptr+=length;
		// ((usb_config_descriptor *)header_ptr)->wTotalLength = __cpu_to_le16(ptr - header_ptr);
	}

	if (device->is_highspeed()) {
	  for (i=1;i<=device->get_descriptor()->bNumConfigurations;i++) {
	    int length=device->get_device_qualifier()->get_configuration(i)->get_full_descriptor_length();
	    memcpy(ptr,device->get_device_qualifier()->get_configuration(i)->get_full_descriptor(),length);
		((usb_config_descriptor *)ptr)->bDescriptorType=USB_DT_CONFIG;
		((usb_config_descriptor *)ptr)->bmAttributes=((usb_config_descriptor *)ptr)->bmAttributes & (~USB_CONFIG_ATT_WAKEUP);
	    ptr+=length;
	  }
	} else {
		for (i=1;i<=device->get_descriptor()->bNumConfigurations;i++) {
			//	header_ptr = ptr;
			int length=device->get_configuration(i)->get_full_descriptor_length();
			memcpy(ptr,device->get_configuration(i)->get_full_descriptor(),length);
			((usb_config_descriptor *)ptr)->bmAttributes=((usb_config_descriptor *)ptr)->bmAttributes & (~USB_CONFIG_ATT_WAKEUP);
			ptr+=length;
			// ((usb_config_descriptor *)header_ptr)->wTotalLength = __cpu_to_le16(ptr - header_ptr);
		}
	}
	memcpy(ptr, (char *)device_descriptor, sizeof(usb_device_descriptor));
	ptr += sizeof(struct usb_device_descriptor);

	/* FINISH - remove this output */
	for(i=0; descriptor_buf+i != ptr; i++) {
		if(i%8 == 0)
			fprintf(stderr, "\n");
		fprintf(stderr, " %02x", descriptor_buf[i]);
	}
	if(i%8 != 0)
		fprintf(stderr, "\n");

	device_filename = find_gadget(device_path);
	if (device_filename == NULL) {
		fprintf(stderr, "Error, unable to find gadget file in %s.\n", device_path);
		return 1;
	}

	char path[256]={0x0};
	strcat(path, device_path);
	strcat(path, "/");
	strcat(path, device_filename);

	p_device_file = open(path, O_RDWR);
	if (p_device_file < 0)
		fprintf(stderr,"Fail on open %d %s\n",errno,strerror(errno));

	status = write(p_device_file, descriptor_buf, ptr - descriptor_buf);
	if (status < 0)
		fprintf(stderr,"Fail on write %d %s\n",errno,strerror(errno));

	p_is_connected = true;
	return 0;
}

void USBHostProxy_GadgetFS::disconnect() {
	//FINISH
	if (!p_is_connected) {fprintf(stderr,"GadgetFS not connected.\n"); return;}
	
	//FINISH - check if it's open
	close(p_device_file);
	
	p_is_connected = false;
}

void USBHostProxy_GadgetFS::reset() {
	//FINISH
}

bool USBHostProxy_GadgetFS::is_connected() {
	return p_is_connected;
}

//return 0 in usb_ctrlrequest->brequest if there is no request
int USBHostProxy_GadgetFS::control_request(usb_ctrlrequest *setup_packet, int *nbytes, __u8** dataptr) {
	//FINISH
	setup_packet->bRequest=0;
	return 0;
}

void USBHostProxy_GadgetFS::send_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8* dataptr,int length) {
	//FINISH
}

void USBHostProxy_GadgetFS::receive_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8** dataptr, int* length) {
	//FINISH
}
