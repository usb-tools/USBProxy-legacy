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
 * USBHostProxyNull.h
 *
 * Created on: Nov 15, 2013
 */
#ifndef USBHOSTPROXYNULL_H_
#define USBHOSTPROXYNULL_H_

#include "USBHostProxy.h"

class USBHostProxy_Null: public USBHostProxy {
private:
	bool connected=false;
public:
	USBHostProxy_Null() {}
	virtual ~USBHostProxy_Null() {}

	int connect(USBDevice* device) {connected=true;return 0;}
	void disconnect() {connected=false;}
	void reset() {}
	bool is_connected() {return connected;}

	int control_request(const usb_ctrlrequest *setup_packet, int *nbytes, __u8** dataptr) {setup_packet->bRequestType=0;return 0;}
	void send_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8* dataptr,int length) {}
	void receive_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8** dataptr, int* length) {*length=0;}

	const char* toString() {return (char*)"Null Host";}
};

#endif /* USBHOSTPROXYNULL_H_ */
