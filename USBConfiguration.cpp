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
 * USBConfiguration.cpp
 *
 * Created on: Nov 6, 2013
 */

#include "USBConfiguration.h"
#include "stdio.h"
#include <stdlib.h>
#include <memory.h>

USBConfiguration::USBConfiguration(USBDeviceProxy* proxy, int idx)
{
	__u8* buf=(__u8 *)malloc(8);
	usb_ctrlrequest setup_packet;
	setup_packet.bRequestType=USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
	setup_packet.bRequest=USB_REQ_GET_DESCRIPTOR;
	setup_packet.wValue=USB_DT_CONFIG<<8;
	setup_packet.wIndex=idx;
	setup_packet.wLength=8;
	int len=0;
	proxy->control_request(&setup_packet,&len,buf);
	len=buf[2];
	buf=(__u8*)realloc(buf,len);
	setup_packet.wLength=len;
	proxy->control_request(&setup_packet,&len,buf);
	//copy descriptor
	memcpy(&descriptor,buf,9);
	interfaceGroups=(USBInterfaceGroup **)calloc(descriptor.bNumInterfaces,sizeof(*interfaceGroups));

	__u8* e=buf+len;
	__u8* p=buf+9;
	while (p<e) {
		add_interface(new USBInterface(&p,e));
	}
	//TODO:read string descriptors
}

USBConfiguration::USBConfiguration(usb_config_descriptor* _descriptor) {
	descriptor=*_descriptor;
	interfaceGroups=(USBInterfaceGroup **)calloc(descriptor.bNumInterfaces,sizeof(*interfaceGroups));
}

USBConfiguration::USBConfiguration(__u16 wTotalLength,__u8 bNumInterfaces,__u8 bConfigurationValue,__u8 iConfiguration,__u8 bmAttributes,__u8 bMaxPower) {
	descriptor.wTotalLength=wTotalLength;
	descriptor.bNumInterfaces=bNumInterfaces;
	descriptor.bConfigurationValue=bConfigurationValue;
	descriptor.iConfiguration=iConfiguration;
	descriptor.bmAttributes=bmAttributes;
	descriptor.bMaxPower=bMaxPower;
	interfaceGroups=(USBInterfaceGroup **)calloc(descriptor.bNumInterfaces,sizeof(*interfaceGroups));
}

USBConfiguration::~USBConfiguration() {
	int i;
	for(i=0;i<descriptor.bNumInterfaces;i++) {delete(interfaceGroups[i]);}
	free(interfaceGroups);
}

const usb_config_descriptor* USBConfiguration::getDescriptor() {
	return &descriptor;
}

const __u8* USBConfiguration::getFullDescriptor() {
	__u8* buf=(__u8*)malloc(descriptor.wTotalLength);
	__u8* p=buf;
	int i;
	for(i=0;i<descriptor.bNumInterfaces;i++) {
		interfaceGroups[i]->getFullDescriptor(&p);
	}
	return buf;
}

void USBConfiguration::add_interface(USBInterface* interface) {
	__u8 number=interface->getDescriptor()->bInterfaceNumber;
	if (!interfaceGroups[number]) {
		fprintf(stderr,"AI%dS\n",number);
		interfaceGroups[number]=new USBInterfaceGroup(number);
		fprintf(stderr,"AI%dE\n",number);
	}
	interfaceGroups[number]->add_interface(interface);
}

USBInterface* USBConfiguration::get_interface(__u8 number,__u8 alternate) {
	if (!interfaceGroups[number]) {return NULL;}
	return interfaceGroups[number]->get_interface(alternate);
}

void USBConfiguration::print(__u8 tabs) {
	unsigned int i;
	for(i=0;i<tabs;i++) {putchar('\t');}
	printf("Config(%d):",descriptor.bConfigurationValue);
	for(i=0;i<sizeof(descriptor);i++) {printf(" %02x",((__u8*)&descriptor)[i]);}
	putchar('\n');
	for(i=0;i<descriptor.bNumInterfaces;i++) {
		interfaceGroups[i]->print(tabs+1);
	}
}
