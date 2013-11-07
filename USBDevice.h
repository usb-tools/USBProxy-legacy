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
 */

#ifndef _USBDevice_
#define _USBDevice_

#include <linux/usb/ch9.h>
#include "USBDeviceProxy.h"
#include "USBConfiguration.h"

class USBDevice {
private:
	int activeConfigurationIndex=-1;
	int address=-1;
    usb_device_descriptor descriptor;
	USBConfiguration* activeConfiguration=NULL;
    USBConfiguration** configurations;
    //TODO: USBVendor deviceVendor;
    //TODO: array(of endpoints) endpoints;

public:
    USBDevice(USBDeviceProxy* proxy);
	USBDevice(usb_device_descriptor* _descriptor);
	USBDevice(__le16 bcdUSB,	__u8  bDeviceClass,	__u8  bDeviceSubClass,	__u8  bDeviceProtocol,	__u8  bMaxPacketSize0,	__le16 idVendor,	__le16 idProduct,	__le16 bcdDevice,	__u8  iManufacturer,	__u8  iProduct,	__u8  iSerialNumber,	__u8  bNumConfigurations);
	~USBDevice();
	const usb_device_descriptor* getDescriptor();
	void add_configuration(USBConfiguration* config);
	USBConfiguration* get_configuration(__u8 index);
	void print(__u8 tabs=0);
};

#endif
