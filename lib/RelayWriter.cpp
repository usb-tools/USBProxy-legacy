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
#include <unistd.h>
#include <poll.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>
#include "valgrind.h"
#include "get_tid.h"
#include "HaltSignal.h"

#include "RelayWriter.h"

#include "Endpoint.h"
#include "Proxy.h"
#include "DeviceProxy.h"
#include "PacketFilter.h"
#include "Manager.h"

RelayWriter::RelayWriter(Endpoint* _endpoint,Proxy* _proxy,mqd_t _recvQueue) {
	haltSignal=0;
	recvQueues=(mqd_t*)malloc(sizeof(mqd_t));
	recvQueues[0]=_recvQueue;
	sendQueues=NULL;
	queueCount=1;

	filters=NULL;
	filterCount=0;

	endpoint=_endpoint->get_descriptor()->bEndpointAddress;
	attributes=_endpoint->get_descriptor()->bmAttributes;
	maxPacketSize=_endpoint->get_descriptor()->wMaxPacketSize;

	proxy=_proxy;
	deviceProxy=NULL;
	manager=NULL;
}

RelayWriter::RelayWriter(Endpoint* _endpoint,DeviceProxy* _deviceProxy,Manager* _manager,mqd_t _recvQueue,mqd_t _sendQueue) {
	haltSignal=0;
	recvQueues=(mqd_t*)malloc(sizeof(mqd_t));
	recvQueues[0]=_recvQueue;
	sendQueues=(mqd_t*)malloc(sizeof(mqd_t));
	sendQueues[0]=_sendQueue;
	queueCount=1;

	filters=NULL;
	filterCount=0;

	endpoint=_endpoint->get_descriptor()->bEndpointAddress;
	attributes=_endpoint->get_descriptor()->bmAttributes;
	maxPacketSize=_endpoint->get_descriptor()->wMaxPacketSize;

	proxy=NULL;
	deviceProxy=_deviceProxy;
	manager=_manager;
}

RelayWriter::~RelayWriter() {
	if (filters) {
		free(filters);
		filters=NULL;
	}
	if (recvQueues) {
		__u8 i,j;
		for (i=0;i<queueCount;i++) {
			mq_attr mqa;
			mq_getattr(recvQueues[i],&mqa);
			if (mqa.mq_curmsgs>0) {
				Packet *p;
				for (j=0;j<mqa.mq_curmsgs;j++) {
					 mq_receive(recvQueues[i],(char*)&p,4,NULL);
					 delete(p);
				}
			}
			mq_close(recvQueues[i]);
		}
		free(recvQueues);
		recvQueues=NULL;
	}
	if (sendQueues) {
		__u8 i,j;
		for (i=0;i<queueCount;i++) {
			mq_attr mqa;
			mq_getattr(sendQueues[i],&mqa);
			if (mqa.mq_curmsgs>0) {
				Packet *p;
				for (j=0;j<mqa.mq_curmsgs;j++) {
					 mq_receive(sendQueues[i],(char*)&p,4,NULL);
					 delete(p);
				}
			}
			mq_close(sendQueues[i]);
		}
		free(sendQueues);
		sendQueues=NULL;
	}
}

void RelayWriter::set_haltsignal(__u8 _haltSignal) {
	haltSignal=_haltSignal;
}

void RelayWriter::relay_write_setup_valgrind() {
	if (!deviceProxy) {fprintf(stderr,"DeviceProxy not initialized for EP00 writer.\n");return;}
	if (!sendQueues) {fprintf(stderr,"outQueues not initialized for EP00 writer.\n");return;}

	bool halt=false;
	struct pollfd haltpoll;
	int haltfd;
	if (haltsignal_setup(haltSignal,&haltpoll,&haltfd)!=0) return;

	struct pollfd poll_send;
	poll_send.events=POLLOUT;

	__u8 i,j;
	struct pollfd* pollfds=(pollfd*)calloc(queueCount,sizeof(pollfd));
	for (i=0;i<queueCount;i++) {
		pollfds[i].fd=recvQueues[i];
		pollfds[i].events=POLLIN;
	}

	bool idle=true;
	bool writing=false;
	SetupPacket *p=NULL;
	int numEvents=0;
	int length;
	usb_ctrlrequest ctrl_req;

	fprintf(stderr,"Starting setup writer thread (%ld) for EP%02x.\n",gettid(),endpoint);
	while (!halt) {
		idle=true;
		if (!writing) {
			if (!p) {
				if (!numEvents) {
					i=0;
					p=NULL;
					numEvents=poll(pollfds,queueCount,500);
					idle=!numEvents;
				}
				while(i<numEvents && (!(pollfds[i].revents&POLLIN))) {i++;}
				if (i<numEvents) {
					mq_receive(pollfds[i].fd,(char*)&p,4,NULL);
					pollfds[i].revents=0;
					p->source=sendQueues[i];
				}
				if (i>=numEvents) numEvents=0;
			}
			if (p) {
				j=0;
				while (j<filterCount && p->filter_out) {
					if (filters[j]->test_setup_packet(p,true)) {filters[j]->filter_setup_packet(p,true);}
					j++;
				}
				ctrl_req=p->ctrl_req;
				if (p->transmit_out) {
					if (ctrl_req.bRequestType&0x80) { //device->host
						p->data=(__u8*)malloc(ctrl_req.wLength);
						p->transmit_in = (deviceProxy->control_request(&(p->ctrl_req),&length,p->data,500)>=0);
						j=0;
						p->ctrl_req.wLength=length;
					} else { //host->device
						length=ctrl_req.wLength;
						p->transmit_in = (deviceProxy->control_request(&(p->ctrl_req),&length,p->data,500)>=0);
						if (p->ctrl_req.bRequest==9 && p->ctrl_req.bRequestType==0) {manager->setConfig(p->ctrl_req.wValue);}
						p->ctrl_req.wLength=0;
					}
					while (j<filterCount && p->filter_in) {
						if (filters[j]->test_setup_packet(p,false)) {filters[j]->filter_setup_packet(p,false);}
						j++;
					}
					poll_send.fd=p->source;
					mq_send(p->source,(char*)&p,4,0);
					writing=true;
					idle=false;
				}
				p=NULL;
			}
		} else {
			if (poll(&poll_send,1,500) && poll_send.revents==POLLOUT) {
				writing=false;
				poll_send.revents=0;
				idle=false;
			}
		}
		if (idle) sched_yield();
		halt=haltsignal_check(haltSignal,&haltpoll,&haltfd);
	}
	fprintf(stderr,"Finished setup writer thread (%ld) for EP%02x.\n",gettid(),endpoint);
	free(pollfds);
}

void RelayWriter::relay_write_setup() {
	if (!deviceProxy) {fprintf(stderr,"DeviceProxy not initialized for EP00 writer.\n");return;}
	if (!sendQueues) {fprintf(stderr,"outQueues not initialized for EP00 writer.\n");return;}

	bool halt=false;
	struct pollfd haltpoll;
	int haltfd;
	if (haltsignal_setup(haltSignal,&haltpoll,&haltfd)!=0) return;

	struct pollfd poll_send;
	poll_send.events=POLLOUT;

	__u8 i,j;
	int efd=epoll_create1(EPOLL_CLOEXEC);
	if (efd<0) fprintf(stderr,"Error creating epoll fd %d [%s].\n",errno,strerror(errno));
	struct epoll_event event;
	struct epoll_event* events=(epoll_event*)calloc(queueCount,sizeof(epoll_event));
	for (i=0;i<queueCount;i++) {
		event.data.u64=((__u64)recvQueues[i])<<32 | sendQueues[i];
		event.events=EPOLLIN;
		epoll_ctl(efd,EPOLL_CTL_ADD,recvQueues[i],&event);
	}

	bool idle=true;
	bool writing=false;
	SetupPacket *p=NULL;
	int numEvents=0;
	int length;
	usb_ctrlrequest ctrl_req;

	fprintf(stderr,"Starting setup writer thread (%ld) for EP%02x.\n",gettid(),endpoint);
	while (!halt) {
		fprintf(stderr,"SW loop\n");
		idle=true;
		if (!writing) {
			fprintf(stderr,"SW reading\n");
			if (!p) {
				fprintf(stderr,"SW no packet\n");
				if (!numEvents) {
					i=0;
					p=NULL;
					numEvents=epoll_wait(efd,events,queueCount,500);
					fprintf(stderr,"Got %d epoll events\n",numEvents);
				}
				if (i<numEvents && (events[i].events&EPOLLIN)) {
					fprintf(stderr,"Handling epoll event #%d\n",i);
					int recvQueue=event.data.u64>>32;
					int sendQueue=event.data.u64&(__u64)0xffffffff;
					mq_receive(recvQueue,(char*)&p,4,NULL);
					p->source=sendQueue;
					i++;
				}
				if (i>=numEvents) numEvents=0;
			}
			if (p) {
				fprintf(stderr,"SW packet\n");
				j=0;
				while (j<filterCount && p->filter_out) {
					if (filters[j]->test_setup_packet(p,true)) {filters[j]->filter_setup_packet(p,true);}
					j++;
				}
				ctrl_req=p->ctrl_req;
				if (p->transmit_out) {
					if (ctrl_req.bRequestType&0x80) { //device->host
						p->data=(__u8*)malloc(ctrl_req.wLength);
						p->transmit_in = (deviceProxy->control_request(&(p->ctrl_req),&length,p->data,500)>=0);
						j=0;
						p->ctrl_req.wLength=length;
					} else { //host->device
						length=ctrl_req.wLength;
						p->transmit_in = (deviceProxy->control_request(&(p->ctrl_req),&length,p->data,500)>=0);
						if (p->ctrl_req.bRequest==9 && p->ctrl_req.bRequestType==0) {manager->setConfig(p->ctrl_req.wValue);}
						p->ctrl_req.wLength=0;
					}
					while (j<filterCount && p->filter_in) {
						if (filters[j]->test_setup_packet(p,false)) {filters[j]->filter_setup_packet(p,false);}
						j++;
					}
					mq_send(p->source,(char*)&p,4,0);
					poll_send.fd=p->source;
					writing=true;
					idle=false;
				}
				p=NULL;
			}
		} else {
			fprintf(stderr,"SW writing\n");
			if (poll(&poll_send,1,500) && poll_send.revents==POLLOUT) {
				writing=false;
				poll_send.revents=0;
				idle=false;
			}
		}
		if (idle) sched_yield();
		halt=haltsignal_check(haltSignal,&haltpoll,&haltfd);
	}
	fprintf(stderr,"Finished setup writer thread (%ld) for EP%02x.\n",gettid(),endpoint);
	free(events);
	close(efd);
}

void RelayWriter::relay_write_valgrind() {
	if (!endpoint) {
		relay_write_setup_valgrind();
		return;
	}
	bool halt=false;
	struct pollfd haltpoll;
	int haltfd;
	if (haltsignal_setup(haltSignal,&haltpoll,&haltfd)!=0) return;

	__u8 i,j;

	struct pollfd* pollfds=(pollfd*)calloc(queueCount,sizeof(pollfd));
	for (i=0;i<queueCount;i++) {
		pollfds[i].fd=recvQueues[i];
		pollfds[i].events=POLLIN;
	}

	bool idle=true;
	bool writing=false;
	Packet *p=NULL;
	int numEvents=0;

	fprintf(stderr,"Starting writer thread (%ld) for EP%02x.\n",gettid(),endpoint);
	while (!halt) {
		idle=true;
		if (!writing) {
			if (!p) {
				if (!numEvents) {
					i=0;
					p=NULL;
					numEvents=poll(pollfds,queueCount,500);
					idle=!numEvents;
				}
				while(i<numEvents && (!(pollfds[i].revents&POLLIN))) {i++;}
				if (i<numEvents) {
					mq_receive(pollfds[i].fd,(char*)&p,4,NULL);
					pollfds[i].revents=0;
				}
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
	free(pollfds);
}

void RelayWriter::relay_write() {
	if (!endpoint) {
		relay_write_setup();
		return;
	}
	bool halt=false;
	struct pollfd haltpoll;
	int haltfd;
	if (haltsignal_setup(haltSignal,&haltpoll,&haltfd)!=0) return;

	__u8 i,j;
	int efd=epoll_create1(EPOLL_CLOEXEC);
	if (efd<0) fprintf(stderr,"Error creating epoll fd %d [%s].\n",errno,strerror(errno));
	struct epoll_event event;
	struct epoll_event* events=(epoll_event*)calloc(queueCount,sizeof(epoll_event));
	for (i=0;i<queueCount;i++) {
		event.data.fd=recvQueues[i];
		event.events=EPOLLIN;
		epoll_ctl(efd,EPOLL_CTL_ADD,recvQueues[i],&event);
	}

	bool idle=true;
	bool writing=false;
	Packet *p=NULL;
	int numEvents=0;


	fprintf(stderr,"Starting writer thread (%ld) for EP%02x.\n",gettid(),endpoint);
	while (!halt) {
		idle=true;
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
	free(events);
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

void RelayWriter::add_queue(mqd_t recvQueue) {
	recvQueues=(mqd_t*)realloc(recvQueues,(queueCount+1)*sizeof(mqd_t));
	recvQueues[queueCount]=recvQueue;
	queueCount++;
}

void RelayWriter::add_setup_queue(mqd_t recvQueue,mqd_t sendQueue) {
	recvQueues=(mqd_t*)realloc(recvQueues,(queueCount+1)*sizeof(mqd_t));
	recvQueues[queueCount]=recvQueue;
	sendQueues=(mqd_t*)realloc(sendQueues,(queueCount+1)*sizeof(mqd_t));
	sendQueues[queueCount]=sendQueue;
	queueCount++;
}

void* RelayWriter::relay_write_helper(void* context) {
	if (RUNNING_ON_VALGRIND) {
		((RelayWriter*)context)->relay_write_valgrind();
	} else {
		((RelayWriter*)context)->relay_write();
	}
	return 0;
}

