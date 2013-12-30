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

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "HexString.h"
#include "DeviceProxy_TCP.h"

int DeviceProxy_TCP::debugLevel = 0;

DeviceProxy_TCP::DeviceProxy_TCP(const char* address) {
	network = new TCP_Helper(address);
	p_is_connected = false;
}

DeviceProxy_TCP::~DeviceProxy_TCP() {
	if (network) {
		delete(network);
		network=NULL;
	}
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
	//FINISH
}

bool DeviceProxy_TCP::is_connected() {
	return p_is_connected;
}

bool DeviceProxy_TCP::is_highspeed() {
	return false;
}

//return -1 to stall
int DeviceProxy_TCP::control_request(const usb_ctrlrequest *setup_packet, int *nbytes, __u8 *dataptr, int timeout) {
	//FINISH
	if (debugLevel>1) {
		char* hex=hex_string((void*)setup_packet,sizeof(*setup_packet));
		fprintf(stderr, "TCP> %s\n",hex);
		free(hex);
	}
	int length=8;
	fprintf(stderr,"length1 %d\n",length);
	length+=(setup_packet->bRequestType&0x80)?0:setup_packet->wLength;
	fprintf(stderr,"length2 %d\n",length);
	__u8* buf=(__u8*)malloc(length);
	memcpy(buf,setup_packet,8);
	if (!(setup_packet->bRequestType&0x80)) memcpy(buf+8,dataptr,setup_packet->wLength);
	network->send_data(0,buf,length);
	free(buf);
	network->receive_data(0,&buf,&length,timeout);
	if (length==0 || buf[0]) {return -1;}
	__u16 usblen=buf[1]<<8 | buf[2];
	*nbytes=(usblen>setup_packet->wLength)?setup_packet->wLength:usblen;
	if (debugLevel>1 && *nbytes) {
		char* hex=hex_string((void*)(buf+3),*nbytes);
		fprintf(stderr, "TCP> %s\n",hex);
		free(hex);
	}
	memcpy(dataptr,buf+3,*nbytes);
	free(buf);
}

void DeviceProxy_TCP::send_data(__u8 endpoint,__u8 attributes, __u16 maxPacketSize, __u8* dataptr, int length) {
	//FINISH
}

void DeviceProxy_TCP::receive_data(__u8 endpoint,__u8 attributes, __u16 maxPacketSize, __u8** dataptr, int* length, int timeout) {
	//FINISH
}

void DeviceProxy_TCP::setConfig(Configuration* fs_cfg, Configuration* hs_cfg, bool hs) {
	//FINISH
}

void DeviceProxy_TCP::claim_interface(__u8 interface) {}

void DeviceProxy_TCP::release_interface(__u8 interface) {}

__u8 DeviceProxy_TCP::get_address() {
	return 1;
}
