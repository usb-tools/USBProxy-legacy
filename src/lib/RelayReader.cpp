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

#include "myDebug.h"

RelayReader::RelayReader(Endpoint* _endpoint,Proxy* _proxy,mqd_t _sendQueue) {
	haltSignal=0;
	sendQueue=_sendQueue;
	recvQueue=0;
	proxy=_proxy;
	hostProxy=NULL;
	endpoint=_endpoint->get_descriptor()->bEndpointAddress;
	attributes=_endpoint->get_descriptor()->bmAttributes;
	maxPacketSize=_endpoint->get_descriptor()->wMaxPacketSize;
}

RelayReader::RelayReader(Endpoint* _endpoint,HostProxy* _hostProxy,mqd_t _sendQueue,mqd_t _recvQueue) {
	haltSignal=0;
	sendQueue=_sendQueue;
	recvQueue=_recvQueue;
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
	if (!recvQueue) {fprintf(stderr,"inQueue not initialized for EP00 reader.\n");return;}
	bool halt=false;
	struct pollfd haltpoll;
	int haltfd;
	if (haltsignal_setup(haltSignal,&haltpoll,&haltfd)!=0) return;

	struct pollfd poll_send;
	poll_send.fd=sendQueue;
	poll_send.events=POLLOUT;

	struct pollfd poll_recv;
	poll_recv.fd=recvQueue;
	poll_recv.events=POLLIN;

	bool idle=true;
	__u8* buf;
	int length;
	SetupPacket *p=NULL;

	bool direction_out=true;
	usb_ctrlrequest ctrl_req;

	fprintf(stderr,"Starting setup reader thread (%ld) for EP%02x.\n",gettid(),endpoint);
	while (!halt) {
		idle=true;
		if (direction_out) {
			if (!p) {
				buf=NULL;
				length=0;

				hostProxy->control_request(&ctrl_req,&length,&buf,500);
				if (ctrl_req.bRequest) {
					p=new SetupPacket(ctrl_req,buf);
				}
			}
			if (p && poll(&poll_send, 1, 500) && (poll_send.revents&POLLOUT)) {
				mq_send(sendQueue,(char*)&p,sizeof(SetupPacket*),0);
				direction_out=false;
				poll_send.revents=0;
				p=NULL;
				idle=false;
			}
		} else {
			if (!p) {
				if (poll(&poll_recv,1,500) && (poll_recv.revents&POLLIN)) {
					mq_receive(recvQueue,(char*)&p,sizeof(SetupPacket*),0);
					poll_recv.revents=0;
					if (p->transmit_in) {
						if (p->ctrl_req.wLength) {
							hostProxy->send_data(endpoint,attributes,maxPacketSize,p->data,p->ctrl_req.wLength);
						} else {
							hostProxy->control_ack();
						}
					} else {
						hostProxy->stall_ep(endpoint);
					}
				}
			}
			if (p) {
				if (hostProxy->send_wait_complete(endpoint,500)) {
					direction_out=true;
					delete(p);
					p=NULL;
					idle=false;
				}
			}
		}
		if (idle) sched_yield();
		halt=haltsignal_check(haltSignal,&haltpoll,&haltfd);
	}
	fprintf(stderr,"Finished setup reader thread (%ld) for EP%02x.\n",gettid(),endpoint);
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
	poll_out.fd=sendQueue;
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
			if (endpoint=0x01) dbgMessage("proxy->receive_data(endpoint,attributes,maxPacketSize,&buf,&length,500);");
			if (length) {
				p=new Packet(endpoint,buf,length);
				idle=false;
			}
		}
		if (p && poll(&poll_out, 1, 500) && (poll_out.revents&POLLOUT)) {
			mq_send(sendQueue,(char*)&p,sizeof(Packet*),0);
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
