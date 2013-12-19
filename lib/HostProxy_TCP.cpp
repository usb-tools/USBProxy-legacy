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

#include "HostProxy_TCP.h"

int HostProxy_TCP::debugLevel = 0;

HostProxy_TCP::HostProxy_TCP(bool server) {
	p_server = server;
	p_is_connected = false;
	
}

HostProxy_TCP::~HostProxy_TCP() {
}

int HostProxy_TCP::connect(Device* device) {
	p_is_connected = true;
	return 0;
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
	return 0;
}

void HostProxy_TCP::send_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8* dataptr,int length) {
	
}

bool HostProxy_TCP::send_wait_complete(__u8 endpoint,int timeout) {
	return true;
}

void HostProxy_TCP::receive_data(__u8 endpoint, __u8 attributes, __u16 maxPacketSize, __u8** dataptr, int* length, int timeout) {
	
}

void HostProxy_TCP::control_ack() {
	
}

void HostProxy_TCP::stall_ep(__u8 endpoint) {
	
}

void HostProxy_TCP::setConfig(Configuration* fs_cfg,Configuration* hs_cfg,bool hs) {
	
}