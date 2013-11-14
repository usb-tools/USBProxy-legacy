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
 * USBDeviceQualifier.cpp
 *
 * Created on: Nov 9, 2013
 */

#include "USBDeviceQualifier.h"
#include "DefinitionErrors.h"

USBDeviceQualifier::USBDeviceQualifier(USBDevice* _device,USBDeviceProxy* proxy) {
	device=_device;

	usb_ctrlrequest setup_packet;
	setup_packet.bRequestType=USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
	setup_packet.bRequest=USB_REQ_GET_DESCRIPTOR;
	setup_packet.wValue=USB_DT_DEVICE_QUALIFIER<<8;
	setup_packet.wIndex=0;
	setup_packet.wLength=10;
	int len=0;
	if (proxy->control_request(&setup_packet,&len,(__u8 *)&descriptor)) {
		descriptor.bLength=0;
		configurations=NULL;
		return;
	}
	int i;
	configurations=(USBConfiguration **)calloc(descriptor.bNumConfigurations,sizeof(*configurations));

	for(i=0;i<descriptor.bNumConfigurations;i++) {
		configurations[i]=new USBConfiguration(device,proxy,i,true);
		__u8 iConfiguration=configurations[i]->get_descriptor()->iConfiguration;
		if (iConfiguration) {device->add_string(iConfiguration);}
		int j;
		for (j=0;j<configurations[i]->get_descriptor()->bNumInterfaces;j++) {
			int k;
			for (k=0;k<configurations[i]->get_interface_alernate_count(j);k++) {
				__u8 iInterface=configurations[i]->get_interface_alternate(j,k)->get_descriptor()->iInterface;
				if (iInterface) {device->add_string(iInterface);}
			}
		}
	}
}

USBDeviceQualifier::USBDeviceQualifier(USBDevice* _device,const usb_qualifier_descriptor* _descriptor) {
	device=_device;
	descriptor=*_descriptor;
	configurations=(USBConfiguration **)calloc(descriptor.bNumConfigurations,sizeof(*configurations));
}

USBDeviceQualifier::USBDeviceQualifier(USBDevice* _device,__le16 bcdUSB,	__u8  bDeviceClass,	__u8  bDeviceSubClass,	__u8  bDeviceProtocol,	__u8  bMaxPacketSize0, __u8 bNumConfigurations) {
	device=_device;
	descriptor.bLength=10;
	descriptor.bDescriptorType=USB_DT_DEVICE_QUALIFIER;
	descriptor.bcdUSB=bcdUSB;
	descriptor.bDeviceClass=bDeviceClass;
	descriptor.bDeviceSubClass=bDeviceSubClass;
	descriptor.bDeviceProtocol=bDeviceProtocol;
	descriptor.bMaxPacketSize0=bMaxPacketSize0;
	descriptor.bNumConfigurations=bNumConfigurations;
	configurations=(USBConfiguration **)calloc(descriptor.bNumConfigurations,sizeof(*configurations));
}

USBDeviceQualifier::~USBDeviceQualifier() {
	int i;
	if (configurations) {
		for(i=0;i<descriptor.bNumConfigurations;i++) {delete(configurations[i]);}
		free(configurations);
	}
}

const usb_qualifier_descriptor* USBDeviceQualifier::get_descriptor() {
	return &descriptor;
}

void USBDeviceQualifier::add_configuration(USBConfiguration* config) {
	int value=config->get_descriptor()->bConfigurationValue;
	if (value>descriptor.bNumConfigurations) {return;} else {value--;}
	if (configurations[value]) {delete(configurations[value]);}
	configurations[value]=config;
}

USBConfiguration* USBDeviceQualifier::get_configuration(__u8 index) {
	if (index>descriptor.bNumConfigurations) {return NULL;}
	return configurations[index-1];
}

void USBDeviceQualifier::print(__u8 tabs) {
	int i;
	for(i=0;i<tabs;i++) {putchar('\t');}
	printf("HS Qualifier:");
	for(i=0;i<sizeof(descriptor);i++) {printf(" %02x",((__u8 *)&descriptor)[i]);}
	putchar('\n');
	for(i=0;i<descriptor.bNumConfigurations;i++) {
		if (configurations[i]) {configurations[i]->print(tabs+1,(configurations[i]==device->get_active_configuration())?true:false);}
	}
}

void USBDeviceQualifier::set_device(USBDevice* _device) {
	device=_device;
}

const definition_error USBDeviceQualifier::is_defined() {
	if (descriptor.bLength!=10) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x01, DE_OBJ_QUALIFIER);}
	if (descriptor.bDescriptorType!=USB_DT_DEVICE_QUALIFIER) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x02, DE_OBJ_QUALIFIER);}
	//__le16 bcdUSB;
	//__u8  bDeviceClass;
	//__u8  bDeviceSubClass;
	//__u8  bDeviceProtocol;
	if (descriptor.bMaxPacketSize0!=8&&descriptor.bMaxPacketSize0!=16&&descriptor.bMaxPacketSize0!=32&&descriptor.bMaxPacketSize0!=64) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x08, DE_OBJ_QUALIFIER);}
	if (!descriptor.bNumConfigurations) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x09, DE_OBJ_QUALIFIER);}
	//__u8  bRESERVED;

	int i;
	for (i=0;i<descriptor.bNumConfigurations;i++) {
		if (!configurations[i]) {return definition_error(DE_ERR_NULL_OBJECT,0x0, DE_OBJ_OS_CONFIG,i+1);}
		if (configurations[i]->get_descriptor()->bConfigurationValue!=(i+1)) {return definition_error(DE_ERR_MISPLACED_OBJECT,configurations[i]->get_descriptor()->bConfigurationValue, DE_OBJ_OS_CONFIG,i+1);}
		definition_error rc=configurations[i]->is_defined(true);
		if (rc.error) {return rc;}
	}

	return definition_error();
}
