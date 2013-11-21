/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
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

/*-------------------------------------------------------------------------*/
/* gadgetfs currently has no chunking (or O_DIRECT/zerocopy) support
 * to turn big requests into lots of smaller ones; so this is "small".
 */
#define	USB_BUFSIZE	(7 * 1024)
/*-------------------------------------------------------------------------*/

USBHostProxy_GadgetFS::USBHostProxy_GadgetFS(const char * _device_path) {
	device_path=_device_path;
	//FINISH
}

USBHostProxy_GadgetFS::~USBHostProxy_GadgetFS() {
	//FINISH
}

int USBHostProxy_GadgetFS::connect(USBDevice* device) {
	//FINISH
	return 1;
}

void USBHostProxy_GadgetFS::disconnect() {
	//FINISH
}

void USBHostProxy_GadgetFS::reset() {
	//FINISH
}

bool USBHostProxy_GadgetFS::is_connected() {
	//FINISH
	return false;
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
