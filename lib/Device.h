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
 */
#ifndef USBPROXY_DEVICE_H
#define USBPROXY_DEVICE_H

#include <linux/usb/ch9.h>
#include "DefinitionErrors.h"

//CLEANUP error checking on malloc/calloc/realloc
//CLEANUP leak checking on malloc/calloc/realloc
//CLEANUP bound checking (or resize arrays) on add_*/get_*
//CLEANUP handle control_request errors
//CLEANUP null terminated arrays, vs ones where the count is stored vs ones where count is stored in descriptor, should some of these be changed to different types

class Configuration;
class DeviceQualifier;
class USBString;
class DeviceProxy;

class Device {
private:
	int hostAddress;
	int deviceAddress;
	usb_device_state hostState;
	usb_device_state deviceState;
	bool highspeed;

	int hostConfigurationIndex;
	int deviceConfigurationIndex;

	usb_device_descriptor descriptor;
    Configuration** configurations;
    //this is set up like strings[stringID][array of all languages]
    USBString ***strings;
    int maxStringIdx;
    DeviceProxy* proxy;
    void add_language(__u16);
    DeviceQualifier* qualifier;
    const definition_error is_string_defined(__u8 index);

public:
    Device(DeviceProxy* _proxy);
	Device(const usb_device_descriptor* _descriptor);
	Device(__le16 bcdUSB,	__u8  bDeviceClass,	__u8  bDeviceSubClass,	__u8  bDeviceProtocol,	__u8  bMaxPacketSize0,	__le16 idVendor,	__le16 idProduct,	__le16 bcdDevice,	__u8  iManufacturer,	__u8  iProduct,	__u8  iSerialNumber,	__u8  bNumConfigurations);
	~Device();
	const usb_device_descriptor* get_descriptor();
	void add_configuration(Configuration* config);
	Configuration* get_configuration(__u8 index);
	void set_active_configuration(__u8 index);

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
    Configuration* get_active_configuration();
    DeviceQualifier* get_device_qualifier();
    void set_device_qualifier(DeviceQualifier* _qualifier);
    bool is_highspeed();
    const definition_error is_defined();
};

#endif /* USBPROXY_DEVICE_H */
