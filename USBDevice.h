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
#include "USBString.h"

//TODO:error checking on malloc/calloc/realloc

class USBConfiguration;

class USBDevice {
private:
	int activeConfigurationIndex=-1;
	int address=-1;
    usb_device_descriptor descriptor;
	USBConfiguration* activeConfiguration=NULL;
    USBConfiguration** configurations;
    //TODO: USBVendor deviceVendor;
    //this is set up like strings[stringID][array of all languages]
    USBString ***strings;
    int maxStringIdx=0;
    USBDeviceProxy* proxy;
    void add_language(__u16);

public:
    USBDevice(USBDeviceProxy* _proxy);
	USBDevice(usb_device_descriptor* _descriptor);
	USBDevice(__le16 bcdUSB,	__u8  bDeviceClass,	__u8  bDeviceSubClass,	__u8  bDeviceProtocol,	__u8  bMaxPacketSize0,	__le16 idVendor,	__le16 idProduct,	__le16 bcdDevice,	__u8  iManufacturer,	__u8  iProduct,	__u8  iSerialNumber,	__u8  bNumConfigurations);
	~USBDevice();
	const usb_device_descriptor* get_descriptor();
	void add_configuration(USBConfiguration* config);
	USBConfiguration* get_configuration(__u8 index);
	void print(__u8 tabs=0);
	void add_string(USBString* string);
	//adds via proxy
	void add_string(__u8 index,__u16 languageId);
	//adds for all languages
	void add_string(__u8 index);
	USBString* get_string(__u8 index,__u16 languageId);
	USBString* get_manufacturer_string(__u16 languageId=0);
	USBString* get_product_string(__u16 languageId=0);
	USBString* get_serial_string(__u16 languageId=0);
    __u16 get_language_by_index(__u8 index);
    int get_language_count();
};

#endif
