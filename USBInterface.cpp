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
 * USBInterface.cpp
 *
 * Created on: Nov 6, 2013
 */

#include "USBInterface.h"
#include <stdlib.h>
#include <memory.h>

USBInterface::USBInterface(__u8* p) {
	//TODO:read interface then
	endpoints=(USBEndpoint**)malloc(sizeof(*endpoints)*descriptor.bNumEndpoints);
	//TODO:read up to next interface
	//TODO:read string descriptors
}
USBInterface::USBInterface(usb_interface_descriptor* _descriptor) {
	descriptor=*_descriptor;
	endpoints=(USBEndpoint**)malloc(sizeof(*endpoints)*descriptor.bNumEndpoints);
}
USBInterface::USBInterface(__u8 bInterfaceNumber,__u8 bAlternateSetting,__u8 bNumEndpoints,__u8 bInterfaceClass,__u8 bInterfaceSubClass,__u8 bInterfaceProtocol,__u8 iInterface) {
	descriptor.bInterfaceNumber=bInterfaceNumber;
	descriptor.bAlternateSetting=bAlternateSetting;
	descriptor.bNumEndpoints=bNumEndpoints;
	descriptor.bInterfaceClass=bInterfaceClass;
	descriptor.bInterfaceSubClass=bInterfaceSubClass;
	descriptor.bInterfaceProtocol=bInterfaceProtocol;
	descriptor.iInterface=iInterface;
	endpoints=(USBEndpoint**)malloc(sizeof(*endpoints)*descriptor.bNumEndpoints);
}
USBInterface::~USBInterface() {
	int i;
	for(i=0;i<descriptor.bNumEndpoints;i++) {delete(endpoints[i]);}
	free(endpoints);
}
const usb_interface_descriptor* USBInterface::getDescriptor() {
	return &descriptor;
}
void USBInterface::getFullDescriptor(__u8** p) {
	memcpy(*p,&descriptor,descriptor.bLength);
	*p=*p+descriptor.bLength;
	//TODO:this will also need to include report descriptors
	int i;
	for(i=0;i<descriptor.bNumEndpoints;i++) {endpoints[i]->getFullDescriptor(p);}
}

//TODO: should this check whether the element already exists?
//TODO: this works in a different way that the other add_x since the endpoints aren't necessarily 0-based, does this need to be made more clear somehow
void USBInterface::add_endpoint(USBEndpoint* endpoint) {
	int i;
	for(i=0;i<descriptor.bNumEndpoints;i++) {
		if (!endpoints[i]) {
			endpoints[i]=endpoint;
			break;
		}
	}
}
//TODO: should this check whether the element already exists?
USBEndpoint* USBInterface::get_endpoint(__u8 index) {
	return endpoints[index];
}

