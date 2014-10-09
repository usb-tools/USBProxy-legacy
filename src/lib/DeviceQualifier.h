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
 * DeviceQualifier.h
 *
 * Created on: Nov 9, 2013
 */
#ifndef USBPROXY_DEVICEQUALIFIER_H
#define USBPROXY_DEVICEQUALIFIER_H

#include <linux/usb/ch9.h>
#include "DefinitionErrors.h"

class Device;
class Configuration;
class DeviceProxy;

class DeviceQualifier {
private:
    usb_qualifier_descriptor descriptor;
    Configuration** configurations;
    Device* device;

public:
    DeviceQualifier(Device* _device,DeviceProxy* _proxy);
    DeviceQualifier(Device* _device,const usb_qualifier_descriptor* _descriptor);
    DeviceQualifier(Device* _device,__le16 bcdUSB,	__u8  bDeviceClass,	__u8  bDeviceSubClass,	__u8  bDeviceProtocol,	__u8  bMaxPacketSize0, __u8 bNumConfigurations);
	~DeviceQualifier();
	const usb_qualifier_descriptor* get_descriptor();
	void add_configuration(Configuration* config);
	Configuration* get_configuration(__u8 index);
	void print(__u8 tabs=0);
	void set_device(Device* _device);
	const definition_error is_defined();
};

#endif /* USBPROXY_DEVICEQUALIFIER_H */
