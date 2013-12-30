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
 * HostProxy_TCP.cpp
 *
 * Created on: Dec 12, 2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "HexString.h"
#include "HostProxy_TCP.h"

int HostProxy_TCP::debugLevel = 0;

HostProxy_TCP::HostProxy_TCP(const char* address) {
	network = new TCP_Helper(address);
	p_is_connected = false;
}

HostProxy_TCP::~HostProxy_TCP() {
	if (network) {
		delete(network);
		network=NULL;
	}
}

int HostProxy_TCP::connect(Device* device,int timeout) {
	int rc=network->connect(timeout);
	p_is_connected=rc==0;
	return rc;
}

void HostProxy_TCP::disconnect() {
	p_is_connected = false;
}

void HostProxy_TCP::reset() {
	
}

bool HostProxy_TCP::is_connected() {
	return p_is_connected;
}


//return 0 in usb_ctrlrequest->brequest if there is no request
int HostProxy_TCP::control_request(usb_ctrlrequest *setup_packet, int *nbytes, __u8** dataptr, int timeout) {
	int length=0;
	__u8* buf=NULL;
	network->receive_data(0,&buf,&length,timeout);
	if (!length) {
		setup_packet->bRequest=0;
		return 0;
	}

	memcpy(setup_packet,buf,8);
	if (!(setup_packet->bRequestType&0x80) && setup_packet->wLength) {
		*dataptr=(__u8*)malloc(length-8);
		memcpy(dataptr,buf+8,length-8);
		*nbytes=length-8;
	} else {
		*dataptr=NULL;
		*nbytes=0;
	}
	free(buf);

	if (debugLevel>1) {
		char* hex=hex_string((void*)setup_packet,sizeof(*setup_packet));
		fprintf(stderr, "TCP< %s\n",hex);
		free(hex);
	}
	return 0;
}

void HostProxy_TCP::send_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8* dataptr,int length) {
	if (!endpoint) {
		__u8* buf=(__u8*)malloc(length+3);
		buf[0]=0;
		buf[1]=length>>8;
		buf[2]=length&0xff;
		memcpy(buf+3,dataptr,length);
		network->send_data(0,buf,length+3);
		free(buf);
	}
	//FINISH
}

bool HostProxy_TCP::send_wait_complete(__u8 endpoint,int timeout) {
	//FINISH
	return true;
}

void HostProxy_TCP::receive_data(__u8 endpoint, __u8 attributes, __u16 maxPacketSize, __u8** dataptr, int* length, int timeout) {
	//FINISH
}

void HostProxy_TCP::control_ack() {
	__u8 val[]={0,0,0};
	network->send_data(0,val,3);
}

void HostProxy_TCP::stall_ep(__u8 endpoint) {
	if (!endpoint) {
		__u8 val[]={1};
		network->send_data(0,val,1);
	}
	//FINISH
}

void HostProxy_TCP::setConfig(Configuration* fs_cfg,Configuration* hs_cfg,bool hs) {
	//FINISH
}
