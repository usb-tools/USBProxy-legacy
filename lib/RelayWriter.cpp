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
 * RelayWriter.cpp
 *
 * Created on: Dec 8, 2013
 */
#include <stdio.h>
#include <sched.h>
#include <sys/epoll.h>
#include "get_tid.h"
#include "HaltSignal.h"

#include "RelayWriter.h"

#include "Endpoint.h"
#include "Proxy.h"
#include "PacketFilter.h"

RelayWriter::RelayWriter(Endpoint* _endpoint,Proxy* _proxy,mqd_t _queue) {
	haltSignal=0;
	inQueues=(mqd_t*)malloc(sizeof(mqd_t));
	inQueues[0]=_queue;
	queueCount=1;

	filters=NULL;
	filterCount=0;

	endpoint=_endpoint->get_descriptor()->bEndpointAddress;
	attributes=_endpoint->get_descriptor()->bmAttributes;
	maxPacketSize=_endpoint->get_descriptor()->wMaxPacketSize;

	proxy=_proxy;
}

RelayWriter::~RelayWriter() {
	if (filters) {
		free(filters);
		filters=NULL;
	}
	if (inQueues) {
		__u8 i,j;
		for (i=0;i<queueCount;i++) {
			mq_attr mqa;
			mq_getattr(inQueues[i],&mqa);
			if (mqa.mq_curmsgs>0) {
				Packet *p;
				for (j=0;j<mqa.mq_curmsgs;j++) {
					 mq_receive(inQueues[i],(char*)&p,4,NULL);
					 delete(p);
				}
			}
			mq_close(inQueues[i]);
		}
		free(inQueues);
		inQueues=NULL;
	}
}

void RelayWriter::set_haltsignal(__u8 _haltSignal) {
	haltSignal=_haltSignal;
}

void RelayWriter::relay_write() {
	bool halt=false;
	struct pollfd haltpoll;
	int haltfd;
	if (haltsignal_setup(haltSignal,&haltpoll,&haltfd)!=0) return;

	__u8 i,j;
	int efd=epoll_create1(EPOLL_CLOEXEC);
	struct epoll_event event;
	struct epoll_event* events=(epoll_event*)calloc(queueCount,sizeof(epoll_event));
	for (i=0;i<queueCount;i++) {
		event.data.fd=inQueues[i];
		event.events=EPOLLIN;
		epoll_ctl(efd,EPOLL_CTL_ADD,inQueues[i],&event);
	}

	bool idle=true;
	bool writing=false;
	Packet *p;
	int numEvents=0;


	fprintf(stderr,"Starting writer thread (%ld) for EP%02x.\n",gettid(),endpoint);
	while (!halt) {
		if (!writing) {
			if (!p) {
				if (!numEvents) {
					i=0;
					p=NULL;
					numEvents=epoll_wait(efd,events,queueCount,500);
					idle=!numEvents;
				}
				if (i<numEvents && (events[i].events&EPOLLIN)) mq_receive(events[i++].data.fd,(char*)&p,4,NULL);
				if (i>=numEvents) numEvents=0;
			}
			if (p) {
				j=0;
				while (j<filterCount && p->filter) {
					if (filters[j]->test_packet(p)) {filters[j]->filter_packet(p);}
					j++;
				}
				if (p->transmit) {
					proxy->send_data(endpoint,attributes,maxPacketSize,p->data,p->wLength);
					writing=true;
				}
				delete(p);
				p=NULL;
			}
		} else {
			writing=!(proxy->send_wait_complete(endpoint,500));
		}
		if (idle) sched_yield();
		halt=haltsignal_check(haltSignal,&haltpoll,&haltfd);
	}
	fprintf(stderr,"Finished writer thread (%ld) for EP%02x.\n",gettid(),endpoint);
	close(efd);
}

void RelayWriter::add_filter(PacketFilter* filter) {
	if (filterCount) {
		filters=(PacketFilter**)realloc(filters,(filterCount+1)*sizeof(PacketFilter*));
	} else {
		filters=(PacketFilter**)malloc(sizeof(PacketFilter*));
	}
	filters[filterCount]=filter;
	filterCount++;
}

void RelayWriter::add_queue(mqd_t inQueue) {
	inQueues=(mqd_t*)realloc(inQueues,(queueCount+1)*sizeof(mqd_t));
	inQueues[queueCount]=inQueue;
	queueCount++;
}

void* RelayWriter::relay_write_helper(void* context) {
	((RelayWriter*)context)->relay_write();
	return 0;
}

