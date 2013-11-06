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
#include "USBEndpoint.h"

class USBInterface {
	private:
		usb_interface_descriptor descriptor;
		USBEndpoint** endpoints;

	public:
		USBInterface(__u8* p);
		USBInterface(usb_interface_descriptor* _descriptor);
		USBInterface(__u8 bInterfaceNumber,__u8 bAlternateSetting,__u8 bNumEndpoints,__u8 bInterfaceClass,__u8 bInterfaceSubClass,__u8 bInterfaceProtocol,__u8 iInterface);
		~USBInterface();
		const usb_interface_descriptor* getDescriptor();
		void getFullDescriptor(__u8** p);
		void add_endpoint(USBEndpoint* endpoint);
		USBEndpoint* get_endpoint(__u8 index);
};

#endif /* USBINTERFACE_H_ */
