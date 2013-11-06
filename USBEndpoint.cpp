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
 * USBEndpoint.cpp
 *
 * Created on: Nov 6, 2013
 */

#include "USBEndpoint.h"
#include <stdlib.h>
#include <memory.h>

USBEndpoint::USBEndpoint(__u8* p) {
	//TODO:read endpoing
}

USBEndpoint::USBEndpoint(usb_endpoint_descriptor* _descriptor) {
	descriptor=*_descriptor;
}

USBEndpoint::USBEndpoint(__u8 bEndpointAddress,__u8 bmAttributes,__u16 wMaxPacketSize,__u8 bInterval) {
	descriptor.bEndpointAddress=bEndpointAddress;
	descriptor.bmAttributes=bmAttributes;
	descriptor.wMaxPacketSize=wMaxPacketSize;
	descriptor.bInterval=bInterval;
}

USBEndpoint::~USBEndpoint() {

}

const usb_endpoint_descriptor* USBEndpoint::getDescriptor() {
	return &descriptor;
}

void USBEndpoint::getFullDescriptor(__u8** p) {
	memcpy(*p,&descriptor,descriptor.bLength);
	*p=*p+descriptor.bLength;
}

