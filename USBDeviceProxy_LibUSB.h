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

#ifndef _LibUSBDeviceProxy_
#define _LibUSBDeviceProxy_

#include "USBDeviceProxy.h"

class USBDeviceProxy_LibUSB:public USBDeviceProxy {
private:
	libusb_context* context;
	libusb_device_handle* dev_handle;
	bool privateContext=true;
	bool privateDevice=true;
public:
	static int debugLevel;
	USBDeviceProxy_LibUSB(int vendorId=LIBUSB_HOTPLUG_MATCH_ANY,int productId=LIBUSB_HOTPLUG_MATCH_ANY,bool includeHubs=false);
	USBDeviceProxy_LibUSB( libusb_context* _context,libusb_device* dvc);
	USBDeviceProxy_LibUSB( libusb_context* _context,libusb_device_handle* devh);
	~USBDeviceProxy_LibUSB();
	int control_request(const usb_ctrlrequest *setup_packet, int *nbytes, __u8* dataptr);
	__u8 get_address();
	bool is_open();
	const char* toString();
};

#endif
