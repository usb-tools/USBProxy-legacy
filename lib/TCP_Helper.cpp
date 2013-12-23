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
#include <memory.h>

#define BASE_PORT 10400
#define TCP_BUFFER_SIZE 1456

int TCP_Helper::debugLevel=0;

TCP_Helper::TCP_Helper(bool server) {
	p_server = server;
	p_is_connected = false;
}

TCP_Helper::~TCP_Helper() {
	disconnect();
}

bool TCP_Helper::connect() {
	if(p_server)
		p_is_connected = server_connect();
	else
		p_is_connected = client_connect();

	return p_is_connected;
}

bool TCP_Helper::client_connect() {
	return false;
}

bool TCP_Helper::server_connect() {
  
	struct sockaddr_in serv_addr;

	//sized to handle ETHERNET less IP(20 byte)/TCP(max 24 byte) headers
	buf = (__u8*)malloc(TCP_BUFFER_SIZE);

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	printf("socket retrieve success\n");
	
	memset(&serv_addr, '0', sizeof(serv_addr));
	memset(buf, '0', sizeof(TCP_BUFFER_SIZE));

	serv_addr.sin_family = AF_INET;    
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	serv_addr.sin_port = htons(BASE_PORT);    
	
	bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	
	if(listen(listen_fd, 10) == -1){
		printf("Failed to listen\n");
		return false;
	}
	return true;
}

void TCP_Helper::disconnect() {
	if (listen_fd) {close(listen_fd);listen_fd=0;}
	if (buf) {free(buf);buf=NULL;}
}

void TCP_Helper::reset() {
	
}

bool TCP_Helper::is_connected() {
	return p_is_connected;
}
