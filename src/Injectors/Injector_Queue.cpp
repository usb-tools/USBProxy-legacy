/*
 * Copyright 2014 Dominic Spill
 * Copyright 2014 Adam Stasiak
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
 * Injector_Queue.cpp
 *
 * Created on: Nov 22, 2013
 */
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <poll.h>

#include "Injector_Queue.h"
#include "Packet.h"

Injector_Queue::Injector_Queue() {
	spoll.events=POLLIN;
}

Injector_Queue::~Injector_Queue() {
}

void Injector_Queue::start_injector() {
	fprintf(stderr,"Opening Queue Injector.\n");
	// TODO any queue setup required
	
}

int* Injector_Queue::get_pollable_fds() {
	int* tmp=(int*)calloc(2,sizeof(int));
	tmp[0]=sck;
	return tmp;
}

void Injector_Queue::stop_injector() {
	if (sck) {close(sck);sck=0;}
	if (buf) {free(buf);buf=NULL;}
}
void Injector_Queue::get_packets(Packet** packet,SetupPacket** setup,int timeout) {
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

void Injector_Queue::full_pipe(Packet* p) {fprintf(stderr,"Packet returned due to full pipe & buffer\n");}

