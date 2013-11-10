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

#ifndef _USBDeviceProxy_
#define _USBDeviceProxy_

#include <linux/usb/ch9.h>

class USBDeviceProxy{
	//send packet
	//recv packet
	//reset
	//recv setup

public:
	virtual ~USBDeviceProxy() {}
	virtual int control_request(const usb_ctrlrequest *setup_packet, int *nbytes, __u8* dataptr)=0;
	virtual __u8 get_address()=0;
	virtual const char* toString() {return NULL;}
};

#endif
