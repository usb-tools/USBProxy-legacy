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
 * USBInterface.h
 *
 * Created on: Nov 6, 2013
 */
#ifndef USBINTERFACE_H_
#define USBINTERFACE_H_

#include <linux/usb/ch9.h>
#include "USBDeviceProxy.h"

#include "USBConfiguration.h"
#include "USBEndpoint.h"
#include "USBHID.h"
#include "USBString.h"
#include "DefinitionErrors.h"

class USBDevice;

struct USBGenericDescriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__u8 bData[1];
} __attribute__ ((packed));

class USBConfiguration;
class USBEndpoint;

class USBInterface {
	private:
		usb_interface_descriptor descriptor;
		USBEndpoint** endpoints;
		USBConfiguration* configuration;
		USBHID* hid_descriptor;
		USBGenericDescriptor** generic_descriptors;

	public:
		USBInterface(USBConfiguration* _configuration,__u8** p,const __u8* e);
		USBInterface(USBConfiguration* _configuration,const usb_interface_descriptor* _descriptor);
		USBInterface(USBConfiguration* _configuration,__u8 bInterfaceNumber,__u8 bAlternateSetting,__u8 bNumEndpoints,__u8 bInterfaceClass,__u8 bInterfaceSubClass,__u8 bInterfaceProtocol,__u8 iInterface);
		~USBInterface();
		const usb_interface_descriptor* get_descriptor();
		size_t get_full_descriptor_length();
		void get_full_descriptor(__u8** p);
		void add_endpoint(USBEndpoint* endpoint);
		USBEndpoint* get_endpoint_by_idx(__u8 index);
		USBEndpoint* get_endpoint_by_address(__u8 address);
		__u8 get_endpoint_count();
		void print(__u8 tabs=0,bool active=false);
		USBString* get_interface_string(__u16 languageId=0);
		const USBGenericDescriptor* get_generic_descriptor(__u8 index);
		__u8 get_generic_descriptor_count(__u8 index);
		void add_generic_descriptor(USBGenericDescriptor* _gd);
		const definition_error is_defined(__u8 configId,__u8 interfaceNum);
		USBConfiguration* get_configuration();
};

#endif /* USBINTERFACE_H_ */
