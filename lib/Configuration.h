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
 * Configuration.h
 *
 * Created on: Nov 6, 2013
 */
#ifndef USBPROXY_CONFIGURATION_H
#define USBPROXY_CONFIGURATION_H

#include <stddef.h>
#include <linux/types.h>
#include <linux/usb/ch9.h>
#include "DefinitionErrors.h"

class Device;
class InterfaceGroup;
class Interface;

class DeviceProxy;
class USBString;

class Configuration {
	private:
		usb_config_descriptor descriptor;
		InterfaceGroup** interfaceGroups;
		Device* device;

	public:
		Configuration(Device* _device,DeviceProxy* proxy, int index,bool highSpeed=false);
		Configuration(Device* _device,const usb_config_descriptor* _descriptor);
		Configuration(Device* _device,__u16 wTotalLength,__u8 bNumInterfaces,__u8 bConfigurationValue,__u8 iConfiguration,__u8 bmAttributes,__u8 bMaxPower,bool highSpeed=false);
		~Configuration();
		const usb_config_descriptor* get_descriptor();
		__u8* get_full_descriptor();
		size_t get_full_descriptor_length();
		void add_interface(Interface* interface);
		Interface* get_interface_alternate(__u8 number,__u8 alternate);
		Interface* get_interface(__u8 number);
		void print(__u8 tabs=0,bool active=false);
		USBString* get_config_string(__u16 languageId=0);
		__u8 get_interface_alernate_count(__u8 number);
		bool is_highspeed();
		const definition_error is_defined(bool highSpeed=false);
		Device* get_device();
};
#endif /* USBPROXY_CONFIGURATION_H */
