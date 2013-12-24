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
#include <arpa/inet.h>

#define BASE_PORT 10400
#define TCP_BUFFER_SIZE 1456

int TCP_Helper::debugLevel=0;

TCP_Helper::TCP_Helper(bool server) {
	p_server = server;
	p_address = NULL;
	p_is_connected = false;
}

TCP_Helper::TCP_Helper(bool server, char* address) {
	p_server = server;
	p_address = address;
	p_is_connected = false;
}

TCP_Helper::~TCP_Helper() {
	disconnect();
}

bool TCP_Helper::connect() {
	//sized to handle ETHERNET less IP(20 byte)/TCP(max 24 byte) headers
	buf = (__u8*)malloc(TCP_BUFFER_SIZE);
	
	if(p_server)
		sockfd = server_connect(0);
	else
		sockfd = client_connect(0);
	
	if(sockfd < 0) {
		free(buf);
		p_is_connected = false;
	} else
		p_is_connected = true;

	return p_is_connected;
}

int TCP_Helper::client_connect(int port) {
	int sck;
	struct sockaddr_in serv_addr;
	
	memset(buf, '0', TCP_BUFFER_SIZE);
	if((sck = socket(AF_INET, SOCK_STREAM, 0))< 0)
	{
		printf("\n Error : Could not create socket \n");
		return -1;
	}
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(BASE_PORT + port);
	serv_addr.sin_addr.s_addr = inet_addr(p_address);
	
	if(::connect(sck, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
	{
		fprintf(stderr, "Error : Connect Failed \n");
		return -1;
	}
	return sck;
}

int TCP_Helper::server_connect(int port) {
	int sck;
	struct sockaddr_in serv_addr = {};

	sck = socket(AF_INET, SOCK_STREAM, 0);
	fprintf(stderr, "socket retrieve success\n");
	
	memset(buf, '0', sizeof(TCP_BUFFER_SIZE));

	serv_addr.sin_family = AF_INET;    
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	serv_addr.sin_port = htons(BASE_PORT + port);    
	
	bind(sck, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	
	if(listen(sck, 10) == -1){
		fprintf(stderr, "Failed to listen\n");
		return -1;
	}
	return true;
}

void TCP_Helper::disconnect() {
	if (sockfd) {close(sockfd);sockfd=0;}
	if (buf) {free(buf);buf=NULL;}
}

int TCP_Helper::open_endpoint(__u8 ep) {
	ep_sockets[ep] = -1;
	//sized to handle ETHERNET less IP(20 byte)/TCP(max 24 byte) headers
	ep_buf[ep] = (__u8*) malloc(TCP_BUFFER_SIZE);
	
	if(p_server)
		ep_sockets[ep] = server_connect(ep);
	else
		ep_sockets[ep] = client_connect(ep);
	
	if (ep_sockets[ep] < 0)
		free(ep_buf[ep]);
	return ep_sockets[ep];
}

void TCP_Helper::reset() {
	
}

bool TCP_Helper::is_connected() {
	return p_is_connected;
}
