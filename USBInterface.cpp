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

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include "USBInterface.h"

//CLEANUP update active interface in interfacegroup upon set interface request
//CLEANUP update active endpoints in proxied device upon set interface request
//CLEANUP handle any endpoints that become inactive upon set interface request

USBInterface::USBInterface(USBConfiguration* _configuration,__u8** p,const __u8* e) {
	configuration=_configuration;
	hid_descriptor=NULL;
	generic_descriptors=(USBGenericDescriptor**)calloc(1,sizeof(*generic_descriptors));

	memcpy(&descriptor,*p,9);
	*p=*p+9;
	endpoints=(USBEndpoint**)calloc(descriptor.bNumEndpoints,sizeof(*endpoints));
	USBEndpoint** ep=endpoints;
	while (*p<e && (*(*p+1))!=4) {
		switch (*(*p+1)) {
			case 5:
				*(ep++)=new USBEndpoint(this,*p);
				break;
			case 0x21:
				hid_descriptor=new USBHID(*p);
				break;
			default:
				USBGenericDescriptor* d=(USBGenericDescriptor*)malloc((*p)[0]);
				memcpy(d,*p,(*p)[0]);
				int i=0;
				while (generic_descriptors[i]) {i++;}
				generic_descriptors[i]=d;
				generic_descriptors=(USBGenericDescriptor**)realloc(generic_descriptors,sizeof(*generic_descriptors)*(i+1));
				generic_descriptors[i+1]=NULL;
				break;
		}
		*p=*p+**p;
	}
}

USBInterface::USBInterface(USBConfiguration* _configuration,const usb_interface_descriptor* _descriptor) {
	configuration=_configuration;
	hid_descriptor=NULL;

	descriptor=*_descriptor;
	endpoints=(USBEndpoint**)calloc(descriptor.bNumEndpoints,sizeof(*endpoints));
	generic_descriptors=(USBGenericDescriptor**)calloc(1,sizeof(*generic_descriptors));
}

USBInterface::USBInterface(USBConfiguration* _configuration,__u8 bInterfaceNumber,__u8 bAlternateSetting,__u8 bNumEndpoints,__u8 bInterfaceClass,__u8 bInterfaceSubClass,__u8 bInterfaceProtocol,__u8 iInterface) {
	configuration=_configuration;
	hid_descriptor=NULL;
	descriptor.bLength=9;
	descriptor.bDescriptorType=USB_DT_INTERFACE;
	descriptor.bInterfaceNumber=bInterfaceNumber;
	descriptor.bAlternateSetting=bAlternateSetting;
	descriptor.bNumEndpoints=bNumEndpoints;
	descriptor.bInterfaceClass=bInterfaceClass;
	descriptor.bInterfaceSubClass=bInterfaceSubClass;
	descriptor.bInterfaceProtocol=bInterfaceProtocol;
	descriptor.iInterface=iInterface;
	endpoints=(USBEndpoint**)calloc(descriptor.bNumEndpoints,sizeof(*endpoints));
	generic_descriptors=(USBGenericDescriptor**)calloc(1,sizeof(*generic_descriptors));
}

USBInterface::~USBInterface() {
	int i;
	if (endpoints) {
		for(i=0;i<descriptor.bNumEndpoints;i++) {
			if (endpoints[i]) {
				delete(endpoints[i]);
				endpoints[i]=NULL;
			}
		}
		free(endpoints);
		endpoints=NULL;
	}
	if (hid_descriptor) {
		delete(hid_descriptor);
		hid_descriptor=NULL;
	}
	i=0;
	if (generic_descriptors) {
		while (generic_descriptors[i]) {
			free(generic_descriptors[i]);
			generic_descriptors[i]=NULL;
			i++;
		}
		free(generic_descriptors);
		generic_descriptors=NULL;
	}
}

const usb_interface_descriptor* USBInterface::get_descriptor() {
	return &descriptor;
}

size_t USBInterface::get_full_descriptor_length() {
	size_t total=descriptor.bLength;
	if (hid_descriptor) {total+=hid_descriptor->get_full_descriptor_length();}
	int i=0;
	while (generic_descriptors[i]) {
		total+=generic_descriptors[i]->bLength;
		i++;
	}
	for(i=0;i<descriptor.bNumEndpoints;i++) {total+=endpoints[i]->get_full_descriptor_length();}
	return total;
}

void USBInterface::get_full_descriptor(__u8** p) {
	memcpy(*p,&descriptor,descriptor.bLength);
	*p=*p+descriptor.bLength;
	if (hid_descriptor) {hid_descriptor->get_full_descriptor(p);}
	int i=0;
	while (generic_descriptors[i]) {
		memcpy(*p,generic_descriptors[i],generic_descriptors[i]->bLength);
		i++;
	}
	for(i=0;i<descriptor.bNumEndpoints;i++) {endpoints[i]->get_full_descriptor(p);}
}

void USBInterface::add_endpoint(USBEndpoint* endpoint) {
	int i;
	for(i=0;i<descriptor.bNumEndpoints;i++) {
		if (!endpoints[i]) {
			endpoints[i]=endpoint;
			break;
		} else {
			if (endpoints[i]->get_descriptor()->bEndpointAddress==endpoint->get_descriptor()->bEndpointAddress) {
				delete(endpoints[i]);
				/* not needed endpoints[i]=NULL; */
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
		if (endpoints[i]->get_descriptor()->bEndpointAddress==address) {return endpoints[i];}
	}
	return NULL;
}

__u8 USBInterface::get_endpoint_count() {
	return descriptor.bNumEndpoints;
}

void USBInterface::print(__u8 tabs,bool active) {
	unsigned int i;
	for(i=0;i<tabs;i++) {putchar('\t');}
	if (active) {putchar('*');}
	printf("Alt(%d):",descriptor.bAlternateSetting);
	for(i=0;i<sizeof(descriptor);i++) {printf(" %02x",((__u8*)&descriptor)[i]);}
	putchar('\n');
	if (descriptor.iInterface) {
		USBString* s=get_interface_string();
		if (s) {
			for(i=0;i<tabs;i++) {putchar('\t');}
			printf("  Name: ");
			s->print_ascii(stdout);
			putchar('\n');
		}
	}
	if (hid_descriptor) {hid_descriptor->print(tabs+1);}
	int j=0;
	while (generic_descriptors[j]) {
		for(i=0;i<(tabs+1);i++) {putchar('\t');}
		printf("Other(%02x):",generic_descriptors[j]->bDescriptorType);
		for(i=0;i<generic_descriptors[j]->bLength;i++) {printf(" %02x",((__u8*)generic_descriptors[j])[i]);}
		putchar('\n');
		j++;
	}
	for(i=0;i<descriptor.bNumEndpoints;i++) {
		if (endpoints[i]) {endpoints[i]->print(tabs+1);}
	}
}

USBString* USBInterface::get_interface_string(__u16 languageId) {
	if (!descriptor.iInterface) {return NULL;}
	return configuration->get_device()->get_string(descriptor.iInterface,languageId);
}

const USBGenericDescriptor* USBInterface::get_generic_descriptor(__u8 index) {
	return generic_descriptors[index];
}

__u8 USBInterface::get_generic_descriptor_count(__u8 index) {
	int i=0;
	while (generic_descriptors[i]) {i++;}
	return i;
}

void USBInterface::add_generic_descriptor(USBGenericDescriptor* _gd) {
	USBGenericDescriptor* d=(USBGenericDescriptor*)malloc(_gd->bLength);
	memcpy(d,_gd,_gd->bLength);
	int i=0;
	while (generic_descriptors[i]) {i++;}
	generic_descriptors[i]=d;
	generic_descriptors=(USBGenericDescriptor**)realloc(generic_descriptors,sizeof(*generic_descriptors)*(i+1));
	generic_descriptors[i+1]=NULL;
}

const definition_error USBInterface::is_defined(__u8 configId,__u8 interfaceNum) {
	if (descriptor.bLength!=9) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x01, DE_OBJ_INTERFACE,configId,interfaceNum,descriptor.bAlternateSetting);}
	if (descriptor.bDescriptorType!=USB_DT_INTERFACE) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x02, DE_OBJ_INTERFACE,configId,interfaceNum,descriptor.bAlternateSetting);}
	//__u8  bInterfaceNumber;
	//__u8  bAlternateSetting;
	//__u8  bNumEndpoints;
	//__u8  bInterfaceClass;
	//__u8  bInterfaceSubClass;
	//__u8  bInterfaceProtocol;
	//__u8  iInterface;

	int i;
	for (i=0;i<descriptor.bNumEndpoints;i++) {
		if (!endpoints[i]) {return definition_error(DE_ERR_NULL_OBJECT,0x0, DE_OBJ_ENDPOINT,configId,interfaceNum,descriptor.bAlternateSetting,i);}
		definition_error rc=endpoints[i]->is_defined(configId,interfaceNum,descriptor.bAlternateSetting);
		if (rc.error) {return rc;}
	}
	return definition_error();

}

USBConfiguration* USBInterface::get_configuration() {return configuration;}
