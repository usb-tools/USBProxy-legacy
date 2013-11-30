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
#include "FDInfo.h"

USBHostProxy_GadgetFS::USBHostProxy_GadgetFS() {
	mount_gadget();
	p_is_connected = false;
	//Check path exists, permissions, etc
}

USBHostProxy_GadgetFS::~USBHostProxy_GadgetFS() {
	//FINISH
	
	//FINISH - check if it's open
	close(p_device_file);
	unmount_gadget();
}

int USBHostProxy_GadgetFS::connect(USBDevice* device) {
	int i, status;
	char *ptr;
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
		((usb_config_descriptor *)ptr)->bmAttributes=((usb_config_descriptor *)ptr)->bmAttributes & (~USB_CONFIG_ATT_WAKEUP);
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

	showFDInfo();
	p_device_file = find_gadget();
	showFDInfo();
	if (p_device_file < 0) {
		fprintf(stderr,"Fail on open %d %s\n",errno,strerror(errno));
		return 1;
	}

	status = write(p_device_file, descriptor_buf, ptr - descriptor_buf);
	if (status < 0) {
		fprintf(stderr,"Fail on write %d %s\n",errno,strerror(errno));
		close(p_device_file);
		return 1;
	}

	p_is_connected = true;
	return 0;
}

void USBHostProxy_GadgetFS::disconnect() {
	//FINISH
	if (!p_is_connected) {fprintf(stderr,"GadgetFS not connected.\n"); return;}
	
	//FINISH - check if it's open
	close(p_device_file);
	showFDInfo();
	unmount_gadget();
	showFDInfo();
	
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
