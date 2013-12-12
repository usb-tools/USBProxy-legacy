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
 * RelayReader.cpp
 *
 * Created on: Dec 8, 2013
 */
#include <stdio.h>
#include <sched.h>
#include <poll.h>
#include "get_tid.h"
#include "HaltSignal.h"

#include "RelayReader.h"

#include "Endpoint.h"

#include "Proxy.h"
#include "HostProxy.h"
#include "Packet.h"

RelayReader::RelayReader(Endpoint* _endpoint,Proxy* _proxy,mqd_t _outQueue) {
	haltSignal=0;
	outQueue=_outQueue;
	inQueue=0;
	proxy=_proxy;
	hostProxy=NULL;
	endpoint=_endpoint->get_descriptor()->bEndpointAddress;
	attributes=_endpoint->get_descriptor()->bmAttributes;
	maxPacketSize=_endpoint->get_descriptor()->wMaxPacketSize;
}

RelayReader::RelayReader(Endpoint* _endpoint,HostProxy* _hostProxy,mqd_t _outQueue,mqd_t _inQueue) {
	haltSignal=0;
	outQueue=_outQueue;
	inQueue=_inQueue;
	proxy=NULL;
	hostProxy=_hostProxy;
	endpoint=_endpoint->get_descriptor()->bEndpointAddress;
	attributes=_endpoint->get_descriptor()->bmAttributes;
	maxPacketSize=_endpoint->get_descriptor()->wMaxPacketSize;
}

RelayReader::~RelayReader() {
}

void RelayReader::set_haltsignal(__u8 _haltSignal) {
	haltSignal=_haltSignal;
}

void RelayReader::relay_read_setup() {
	if (!hostProxy) {fprintf(stderr,"HostProxy not initialized for EP00 reader.\n");return;}
	if (!inQueue) {fprintf(stderr,"inQueue not initialized for EP00 reader.\n");return;}
	bool halt=false;
	struct pollfd haltpoll;
	int haltfd;
	if (haltsignal_setup(haltSignal,&haltpoll,&haltfd)!=0) return;

	struct pollfd poll_out;
	poll_out.fd=outQueue;
	poll_out.events=POLLOUT;

	struct pollfd poll_in;
	poll_out.fd=inQueue;
	poll_out.events=POLLIN;

	bool idle=true;
	__u8* buf;
	int length;
	SetupPacket *p=NULL;

	//FINISH
	/*
	    write a packet to outQueue
	    wait to read from inQueue
	    send it back to the host, either data, ack, or stall, depending on the data in the packet.
	*/

	bool direction_out=true;
	usb_ctrlrequest ctrl_req;

	fprintf(stderr,"Starting reader thread (%ld) for EP%02x.\n",gettid(),endpoint);
	while (!halt) {
		idle=true;
		if (direction_out) {
			if (!p) {
				buf=NULL;
				length=0;

				hostProxy->control_request(&ctrl_req,&length,&buf,500);
				if (length) {
					p=new SetupPacket(ctrl_req,buf);
				}
			}
			if (p && poll(&poll_out, 1, 500) && (poll_out.revents&POLLOUT)) {
				mq_send(outQueue,(char*)&p,sizeof(SetupPacket*),0);
				direction_out=false;
				poll_out.revents=0;
				p=NULL;
				idle=false;
			}
		} else {
			if (p) {
				if (poll(&poll_in,1,500) && (poll_in.revents&POLLIN)) {
					mq_receive(inQueue,(char*)&p,sizeof(SetupPacket*),0);
					poll_in.revents=0;
					if (p->transmit_out) {
						if (p->ctrl_req.wLength) {
							hostProxy->send_data(endpoint,attributes,maxPacketSize,p->data,p->ctrl_req.wLength);
						} else {
							hostProxy->control_ack();
						}
					} else {
						hostProxy->stall_ep(endpoint);
					}
					p=NULL;
				}
			}
			if (!p) {
				if (hostProxy->send_wait_complete(endpoint,500)) {
					direction_out=true;
					idle=false;
				}
			}
		}
		if (idle) sched_yield();
		halt=haltsignal_check(haltSignal,&haltpoll,&haltfd);
	}
	fprintf(stderr,"Finished reader thread (%ld) for EP%02x.\n",gettid(),endpoint);
}

void RelayReader::relay_read() {
	if (!endpoint) {
		relay_read_setup();
		return;
	}
	bool halt=false;
	struct pollfd haltpoll;
	int haltfd;
	if (haltsignal_setup(haltSignal,&haltpoll,&haltfd)!=0) return;

	struct pollfd poll_out;
	poll_out.fd=outQueue;
	poll_out.events=POLLOUT;
	bool idle=true;
	__u8* buf;
	int length;
	Packet *p=NULL;

	fprintf(stderr,"Starting reader thread (%ld) for EP%02x.\n",gettid(),endpoint);
	while (!halt) {
		idle=true;
		if (!p) {
			buf=NULL;
			length=0;
			proxy->receive_data(endpoint,attributes,maxPacketSize,&buf,&length,500);
			if (length) {
				p=new Packet(endpoint,buf,length);
				idle=false;
			}
		}
		if (p && poll(&poll_out, 1, 500) && (poll_out.revents&POLLOUT)) {
			mq_send(outQueue,(char*)&p,sizeof(Packet*),0);
			poll_out.revents=0;
			p=NULL;
		}
		if (idle) sched_yield();
		halt=haltsignal_check(haltSignal,&haltpoll,&haltfd);
	}
	fprintf(stderr,"Finished reader thread (%ld) for EP%02x.\n",gettid(),endpoint);
}

void* RelayReader::relay_read_helper(void* context) {
	((RelayReader*)context)->relay_read();
	return 0;
}
