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

//TODO: 1 fill out these functions.

typedef void (*statusCallback)();

class USBDeviceProxy{
private:
	statusCallback cb_connect=NULL;
	statusCallback cb_disconnect=NULL;

public:
	virtual ~USBDeviceProxy() {}

	//virtual int connect()=0;
	//virtual void disconnect()=0;
	//virtual void reset()=0;

	virtual int control_request(const usb_ctrlrequest *setup_packet, int *nbytes, __u8* dataptr)=0;
	//virtual void transfer_data(__u8 endpoint,__u8* dataptr,int length, int* transferred);

	virtual __u8 get_address()=0;
	bool is_connected();
	virtual const char* toString() {return NULL;}

	//this should be called on a hotplug connect, device should reconnect. how do we verify we can use the same strucutre?
	//or alternately how do we save and restore and filtering settings on the EPs?
	virtual void set_callback_connect(statusCallback cb) {cb_connect=cb;}

	//this should be called on a hotplug disconnect, device will stop all endpoint loops
	virtual void set_callback_disconnect(statusCallback cb) {cb_disconnect=cb;}
};

#endif
