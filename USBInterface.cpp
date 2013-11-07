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
#include "stdio.h"
#include <stdlib.h>
#include <memory.h>

//TODO:update active interface in interfacegroup upon set interface request
//TODO:update active endpoints in device upon set interface request

USBInterface::USBInterface(__u8** p,__u8* e) {
	memcpy(&descriptor,*p,9);
	fprintf(stderr,"I%d\n",(*p)[2]);
	*p=*p+9;
	endpoints=(USBEndpoint**)calloc(descriptor.bNumEndpoints,sizeof(*endpoints));
	USBEndpoint** ep=endpoints;
	while (*p<e && (*(*p+1))!=4) {
		switch (*(*p+1)) {
			case 5:
				*(ep++)=new USBEndpoint(*p);
				break;
			default:
				int i;
				fprintf(stderr,"Unknown Descriptor:");
				for(i=0;i<**p;i++) {fprintf(stderr," %02x",(*p)[i]);}
				fprintf(stderr,"\n");
				break;
		}
		*p=*p+**p;
	}
	//TODO:read report descriptors
	//TODO:read string descriptors
}

USBInterface::USBInterface(usb_interface_descriptor* _descriptor) {
	descriptor=*_descriptor;
	endpoints=(USBEndpoint**)calloc(descriptor.bNumEndpoints,sizeof(*endpoints));
}

USBInterface::USBInterface(__u8 bInterfaceNumber,__u8 bAlternateSetting,__u8 bNumEndpoints,__u8 bInterfaceClass,__u8 bInterfaceSubClass,__u8 bInterfaceProtocol,__u8 iInterface) {
	descriptor.bInterfaceNumber=bInterfaceNumber;
	descriptor.bAlternateSetting=bAlternateSetting;
	descriptor.bNumEndpoints=bNumEndpoints;
	descriptor.bInterfaceClass=bInterfaceClass;
	descriptor.bInterfaceSubClass=bInterfaceSubClass;
	descriptor.bInterfaceProtocol=bInterfaceProtocol;
	descriptor.iInterface=iInterface;
	endpoints=(USBEndpoint**)calloc(descriptor.bNumEndpoints,sizeof(*endpoints));
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

void USBInterface::add_endpoint(USBEndpoint* endpoint) {
	int i;
	for(i=0;i<descriptor.bNumEndpoints;i++) {
		if (!endpoints[i]) {
			endpoints[i]=endpoint;
			break;
		} else {
			if (endpoints[i]->getDescriptor()->bEndpointAddress==endpoint->getDescriptor()->bEndpointAddress) {
				delete(endpoints[i]);
				endpoints[i]=endpoint;
				break;
			}
		}
	}
	fprintf(stderr,"Ran out of endpoint storage space on interface %d.",descriptor.bInterfaceNumber);
}

USBEndpoint* USBInterface::get_endpoint_by_idx(__u8 index) {
	return endpoints[index];
}

USBEndpoint* USBInterface::get_endpoint_by_address(__u8 address) {
	int i;
	for(i=0;i<descriptor.bNumEndpoints;i++) {
		if (endpoints[i]->getDescriptor()->bEndpointAddress==address) {return endpoints[i];}
	}
	return NULL;
}

__u8 USBInterface::get_endpoint_count() {
	return descriptor.bNumEndpoints;
}

void USBInterface::print(__u8 tabs) {
	int i;
	for(i=0;i<tabs;i++) {putchar('\t');}
	printf("Alt(%d):",descriptor.bAlternateSetting);
	for(i=0;i<sizeof(descriptor);i++) {printf(" %02x",((__u8*)&descriptor)[i]);}
	putchar('\n');
	for(i=0;i<descriptor.bNumEndpoints;i++) {
		endpoints[i]->print(tabs+1);
	}
}

