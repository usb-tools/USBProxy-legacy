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
 * USBConfiguration.h
 *
 * Created on: Nov 6, 2013
 */
#ifndef USBCONFIGURATION_H_
#define USBCONFIGURATION_H_

#include <linux/usb/ch9.h>
#include "USBDeviceProxy.h"
#include "USBDevice.h"
#include "USBInterfaceGroup.h"
#include "USBInterface.h"
#include "USBString.h"

class USBDevice;
class USBInterfaceGroup;

class USBConfiguration {
	private:
		usb_config_descriptor descriptor;
		USBInterfaceGroup** interfaceGroups;
		USBDevice* device;

	public:
		USBConfiguration(USBDeviceProxy* proxy, int index,bool highSpeed=false);
		USBConfiguration(usb_config_descriptor* _descriptor);
		USBConfiguration(__u16 wTotalLength,__u8 bNumInterfaces,__u8 bConfigurationValue,__u8 iConfiguration,__u8 bmAttributes,__u8 bMaxPower,bool highSpeed=false);
		~USBConfiguration();
		const usb_config_descriptor* get_descriptor();
		const __u8* get_full_descriptor();
		size_t get_full_descriptor_length();
		void add_interface(USBInterface* interface);
		USBInterface* get_interface(__u8 number,__u8 alternate);
		void print(__u8 tabs=0,bool active=false);
		void set_usb_device(USBDevice* _device);
		USBString* get_config_string(__u16 languageId=0);
		__u8 get_interface_alernate_count(__u8 number);
		bool is_highspeed();
};
#endif /* USBCONFIGURATION_H_ */
