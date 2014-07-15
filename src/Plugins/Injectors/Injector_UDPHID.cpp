/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
 *
 * This file is part of USBProxy.
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
 * InjectorUDPHID.cpp
 *
 * Created on: Feb 22, 2014
 */
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <poll.h>
#include "errno.h"
#include "netinet/in.h"

#include "Injector_UDPHID.h"
#include "Packet.h"
#include "HexString.h"

Injector_UDPHID::Injector_UDPHID(ConfigParser *cfg) {
	port = cfg->get_as_int("Injector_UDPHID::port");
	sck=0;
	buf=NULL;
	spoll.events=POLLIN;
	this->interface.deviceClass=0xff;
	this->interface.subClass=0x5d;
	this->endpoint.address=0x80;
	this->endpoint.addressMask=0x80;
	reportBuffer[0]=0;
	reportBuffer[1]=0x14;
	reportBuffer[2]=0;
	reportBuffer[3]=0;
	reportBuffer[4]=0;
	reportBuffer[5]=0;
	reportBuffer[6]=0;
	reportBuffer[7]=0;
	reportBuffer[8]=0;
	reportBuffer[9]=0;
	reportBuffer[10]=0;
	reportBuffer[11]=0;
	reportBuffer[12]=0;
	reportBuffer[13]=0;
	reportBuffer[14]=0;
	reportBuffer[15]=0;
	reportBuffer[16]=0;
	reportBuffer[17]=0;
	reportBuffer[18]=0;
	reportBuffer[19]=0;
	cfg->add_pointer("PacketFilter_UDPHID::injector", this);
}

Injector_UDPHID::~Injector_UDPHID() {
	if (sck) {close(sck);sck=0;}
	if (buf) {free(buf);buf=NULL;}
}

void Injector_UDPHID::start_injector() {
	fprintf(stderr,"Opening injection UDP socket on port %d.\n",port);
	sck=socket(AF_INET,SOCK_DGRAM | SOCK_CLOEXEC,IPPROTO_UDP);
	spoll.fd=sck;
	if (sck<0) {
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

int* Injector_UDPHID::get_pollable_fds() {
	int* tmp=(int*)calloc(2,sizeof(int));
	tmp[0]=sck;
	return tmp;
}

void Injector_UDPHID::stop_injector() {
	if (sck) {close(sck);sck=0;}
	if (buf) {free(buf);buf=NULL;}
}
void Injector_UDPHID::get_packets(Packet** packet,SetupPacket** setup,int timeout) {
	*packet=NULL;
	*setup=NULL;

	if (!poll(&spoll, 1, timeout) || !(spoll.revents&POLLIN)) {
		return;
	}

	ssize_t len=recv(sck,buf,UDP_BUFFER_SIZE,0);
	if (len>0) {
		if (buf[0]) {
			__u16 usblen=buf[2]<<8 | buf[3];
			__u8* usbbuf=(__u8*)malloc(usblen);
			memcpy(usbbuf,buf+4,usblen);
			*packet=new Packet(buf[0],usbbuf,usblen,buf[1]&0x01?false:true);
			(*packet)->transmit=buf[1]&0x02?false:true;
			if ((*packet)->wLength==20) {
				__u8* data=(*packet)->data;
				if (data[0]==0 && data[1]==20) {memcpy(reportBuffer+2,data+2,12);}
			}
			return;
		} else {
			struct usb_ctrlrequest ctrl_req;
			memcpy(&ctrl_req,buf+3,8);
			if (ctrl_req.bRequestType&0x80) {
				*setup=new SetupPacket(ctrl_req,NULL,true);
			} else {
				__u8* usbbuf=(__u8*)malloc(ctrl_req.wLength);
				memcpy(usbbuf,buf+11,ctrl_req.wLength);
				*setup=new SetupPacket(ctrl_req,usbbuf,true);
			}
			(*setup)->filter_out=~(buf[1]&0x01);
			(*setup)->transmit_out=~(buf[1]&0x02);
			(*setup)->filter_in=~(buf[1]&0x04);
			(*setup)->transmit_in=~(buf[1]&0x08);
			return;
		}
	}
	if (len<0) {
		fprintf(stderr,"Socket read error [%s].\n",strerror(errno));
	}
	return;
}

void Injector_UDPHID::full_pipe(Packet* p) {fprintf(stderr,"Packet returned due to full pipe & buffer\n");}

static Injector_UDPHID *injector;

extern "C" {
	Injector * get_filter_plugin(ConfigParser *cfg) {
		injector = new Injector_UDPHID(cfg);
		return (Injector *) injector;
	}
	
	void destroy_plugin() {
		delete injector;
	}
}
