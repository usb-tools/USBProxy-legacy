/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
 * 
 * Based on libusb-gadget - Copyright 2009 Daiki Ueno <ueno@unixuser.org>
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

#include "DeviceProxy_TCP.h"
#include <stdio.h>
#include <stdlib.h>
#include "HexString.h"

int DeviceProxy_TCP::debugLevel = 0;

DeviceProxy_TCP::DeviceProxy_TCP(const char* address) {
	network = new TCP_Helper(address);
	p_is_connected = false;
}

DeviceProxy_TCP::~DeviceProxy_TCP() {
	
}

/* Open a socket for EP0 - we don't know how many EPs we need yet */
int DeviceProxy_TCP::connect(int timeout) {
	int rc=network->connect(timeout);
	p_is_connected=rc==0;
	return rc;
}

void DeviceProxy_TCP::disconnect() {
	p_is_connected = false;
}

void DeviceProxy_TCP::reset() {
	
}

bool DeviceProxy_TCP::is_connected() {
	return p_is_connected;
}

bool DeviceProxy_TCP::is_highspeed() {
	return false;
}

//return -1 to stall
int DeviceProxy_TCP::control_request(const usb_ctrlrequest *setup_packet, int *nbytes, __u8 *dataptr, int timeout) {
	if (debugLevel>1) {
		char* hex=hex_string((void*)setup_packet,sizeof(*setup_packet));
		fprintf(stderr, "TCP> %s\n",hex);
		free(hex);
	}
	
	return 0;
}

void DeviceProxy_TCP::send_data(__u8 endpoint,__u8 attributes, __u16 maxPacketSize, __u8* dataptr, int length) {
	
}

void DeviceProxy_TCP::receive_data(__u8 endpoint,__u8 attributes, __u16 maxPacketSize, __u8** dataptr, int* length, int timeout) {
	
}

void DeviceProxy_TCP::setConfig(Configuration* fs_cfg, Configuration* hs_cfg, bool hs) {
	;
}

void DeviceProxy_TCP::claim_interface(__u8 interface) {
	;
}

void DeviceProxy_TCP::release_interface(__u8 interface) {
	;
}

__u8 DeviceProxy_TCP::get_address() {
	return 1;
}
