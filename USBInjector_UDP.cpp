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
 * USBInjectorUDP.cpp
 *
 * Created on: Nov 22, 2013
 */
#include "USBInjector_UDP.h"
#include "netinet/in.h"
#include "errno.h"
#include "USBPacket.h"
#include <memory.h>

USBInjector_UDP::USBInjector_UDP(__u16 _port) {
	port=_port;
	sck=0;
	buf=NULL;
}

USBInjector_UDP::~USBInjector_UDP() {
	if (sck) {close(sck);sck=0;}
	if (buf) {free(buf);}
}

void USBInjector_UDP::start_injector() {
	fprintf(stderr,"Opening injection UDP socket on port %d.\n",port);
	sck=socket(AF_INET,SOCK_DGRAM | SOCK_NONBLOCK,IPPROTO_UDP);
	if (socket<0) {
		fprintf(stderr,"Error creating socket.\n");
		sck=0;
	}
	struct sockaddr_in addr;
	memset((char *)&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(port);
	if (bind(sck,(struct sockaddr*)&addr,sizeof(addr))<0) {
		fprintf(stderr,"Error binding to port %d.\n",port);
		sck=0;
	}
	//sized to handle ETHERNET less IP(20 byte)/UDP(8 byte) headers
	buf=(__u8*)malloc(UDP_BUFFER_SIZE);
}

void USBInjector_UDP::stop_injector() {
	if (sck) {close(sck);sck=0;}
	if (buf) {free(buf);}
}

USBPacket* USBInjector_UDP::get_packets() {
	ssize_t len=recv(sck,buf,UDP_BUFFER_SIZE,0);
	if (len>0) {
		__u16 usblen=buf[2]<<8 | buf[3];
		__u8* usbbuf=(__u8*)malloc(usblen);
		memcpy(usbbuf,buf+4,usblen);
		struct USBPacket* p=new USBPacket(buf[0],usbbuf,usblen,buf[1]&0x01?true:false);
		p->transmit=buf[1]&0x02?true:false;
		return p;
	}
	if (len<0 && errno!=EWOULDBLOCK && errno!=EAGAIN ) {
		fprintf(stderr,"Socket read error [%s].\n",strerror(errno));
	}
	return NULL;
}
