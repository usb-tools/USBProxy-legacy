/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
 *
 * This file is part of USBProxy.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
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
 * Interface.h
 *
 * Created on: Nov 6, 2013
 */
#ifndef USBPROXY_INTERFACE_H
#define USBPROXY_INTERFACE_H

#include <stddef.h>
#include <linux/types.h>
#include <linux/usb/ch9.h>
#include "DefinitionErrors.h"

class DeviceProxy;
class Device;
class Configuration;
class HID;
class USBString;

struct GenericDescriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__u8 bData[1];
} __attribute__ ((packed));

class Configuration;
class Endpoint;
class USBString;

class Interface {
	private:
		usb_interface_descriptor descriptor;
		Endpoint** endpoints;
		Configuration* configuration;
		HID* hid_descriptor;
		GenericDescriptor** generic_descriptors;
		__u8 generic_descriptor_count;

	public:
		Interface(Configuration* _configuration,__u8** p,const __u8* e);
		Interface(Configuration* _configuration,const usb_interface_descriptor* _descriptor);
		Interface(Configuration* _configuration,__u8 bInterfaceNumber,__u8 bAlternateSetting,__u8 bNumEndpoints,__u8 bInterfaceClass,__u8 bInterfaceSubClass,__u8 bInterfaceProtocol,__u8 iInterface);
		~Interface();
		const usb_interface_descriptor* get_descriptor();
		size_t get_full_descriptor_length();
		void get_full_descriptor(__u8** p);
		void add_endpoint(Endpoint* endpoint);
		Endpoint* get_endpoint_by_idx(__u8 index);
		Endpoint* get_endpoint_by_address(__u8 address);
		__u8 get_endpoint_count();
		void print(__u8 tabs=0,bool active=false);
		USBString* get_interface_string(__u16 languageId=0);
		const GenericDescriptor* get_generic_descriptor(__u8 index);
		__u8 get_generic_descriptor_count();
		void add_generic_descriptor(GenericDescriptor* _gd);
		const definition_error is_defined(__u8 configId,__u8 interfaceNum);
		Configuration* get_configuration();
};

#endif /* USBPROXY_INTERFACE_H */
