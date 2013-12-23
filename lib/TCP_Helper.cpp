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
 * TCP_Helper.cpp
 *
 * Created on: Dec 19, 2013
 */

#include "TCP_Helper.h"
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define BASE_PORT 10400
#define TCP_BUFFER_SIZE 1456

int TCP_Helper::debugLevel=0;

TCP_Helper::TCP_Helper(bool server) {
	p_server = server;
	p_is_connected = false;
}

TCP_Helper::~TCP_Helper() {
	if (sck) {close(sck);sck=0;}
	if (buf) {free(buf);buf=NULL;}
}

bool TCP_Helper::connect() {
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
	return p_is_connected;
}

void TCP_Helper::disconnect() {
	
}

void TCP_Helper::reset() {
	
}

bool TCP_Helper::is_connected() {
	return p_is_connected;
}
