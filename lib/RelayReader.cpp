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
#include "Packet.h"

RelayReader::RelayReader(Endpoint* _endpoint,Proxy* _proxy,mqd_t _queue) {
	haltSignal=0;
	outQueue=_queue;
	proxy=_proxy;
	endpoint=_endpoint->get_descriptor()->bEndpointAddress;
	attributes=_endpoint->get_descriptor()->bmAttributes;
	maxPacketSize=_endpoint->get_descriptor()->wMaxPacketSize;
}

RelayReader::~RelayReader() {
}

void RelayReader::set_haltsignal(__u8 _haltSignal) {
	haltSignal=_haltSignal;
}

void RelayReader::relay_read() {
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
	Packet *p;

	fprintf(stderr,"Starting reader thread (%ld) for EP%02x.\n",gettid(),endpoint);
	while (!halt) {
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
