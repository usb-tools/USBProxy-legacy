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
#include <unistd.h>
#include "HexString.h"
#include "Socket_helpers.h"

int DeviceProxy_TCP::debugLevel = 0;

DeviceProxy_TCP::DeviceProxy_TCP(bool server) {
	p_server = server;
	p_is_connected = false;
}

DeviceProxy_TCP::~DeviceProxy_TCP() {
	
}

/* Open a socket for EP0 - we don't know how many EPs we need yet */
int DeviceProxy_TCP::connect() {
	port = BASE_PORT;
	fprintf(stderr,"Opening base port %d.\n", port);
	sck=socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC, IPPROTO_TCP);
	spoll.fd=sck;
	if (sck<0) {
		fprintf(stderr,"Error creating socket.\n");
		sck=0;
	}
	struct sockaddr_in addr = {};
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(port);
	if (bind(sck,(struct sockaddr*)&addr,sizeof(addr))<0) {
		fprintf(stderr,"Error binding to port %d.\n",port);
		sck=0;
	}
	//sized to handle ETHERNET less IP(20 byte)/TCP(max 24 byte) headers
	buf=(__u8*)malloc(TCP_BUFFER_SIZE);
	p_is_connected = true;
	return 0;
}

void DeviceProxy_TCP::disconnect() {
	if (sck) {close(sck);sck=0;}
	if (buf) {free(buf);buf=NULL;}
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
