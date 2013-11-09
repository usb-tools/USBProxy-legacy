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
 * USBDeviceQualifier.h
 *
 * Created on: Nov 9, 2013
 */
#ifndef USBDEVICEQUALIFIER_H_
#define USBDEVICEQUALIFIER_H_

#include <linux/usb/ch9.h>
#include "USBConfiguration.h"

class USBDevice;
class USBConfiguration;

class USBDeviceQualifier {
private:
    usb_qualifier_descriptor descriptor;
    USBConfiguration** configurations;
    USBDevice* device;

public:
    USBDeviceQualifier(USBDeviceProxy* _proxy,USBDevice* _device);
    USBDeviceQualifier(usb_qualifier_descriptor* _descriptor);
    USBDeviceQualifier(__le16 bcdUSB,	__u8  bDeviceClass,	__u8  bDeviceSubClass,	__u8  bDeviceProtocol,	__u8  bMaxPacketSize0, __u8 bNumConfigurations);
	~USBDeviceQualifier();
	const usb_qualifier_descriptor* get_descriptor();
	void add_configuration(USBConfiguration* config);
	USBConfiguration* get_configuration(__u8 index);
	void print(__u8 tabs=0);
	void set_device(USBDevice* _device);
};

#endif /* USBDEVICEQUALIFIER_H_ */
