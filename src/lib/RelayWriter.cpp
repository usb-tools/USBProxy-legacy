/*
 * This file is part of USBProxy.
 */

#include <iostream>

#include <unistd.h>
#include <poll.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>
#include "get_tid.h"

#include "RelayWriter.h"

#include "Endpoint.h"
#include "Proxy.h"
#include "DeviceProxy.h"
#include "PacketFilter.h"
#include "Manager.h"


#ifndef NVALGRIND
#define USEVALGRIND
#include "valgrind.h"
#endif //NVALGRIND


RelayWriter::RelayWriter(Endpoint* _endpoint,Proxy* _proxy,mqd_t _recvQueue)
	: _please_stop(false)
{
	recvQueues.push_back(_recvQueue);

	endpoint=_endpoint->get_descriptor()->bEndpointAddress;
	attributes=_endpoint->get_descriptor()->bmAttributes;
	maxPacketSize=_endpoint->get_descriptor()->wMaxPacketSize;

	proxy=_proxy;
	deviceProxy=NULL;
	manager=NULL;
}

RelayWriter::RelayWriter(Endpoint* _endpoint,DeviceProxy* _deviceProxy,Manager* _manager,mqd_t _recvQueue,mqd_t _sendQueue)
	: _please_stop(false)
{
	recvQueues.push_back(_recvQueue);
	sendQueues.push_back(_sendQueue);

	endpoint=_endpoint->get_descriptor()->bEndpointAddress;
	attributes=_endpoint->get_descriptor()->bmAttributes;
	maxPacketSize=_endpoint->get_descriptor()->wMaxPacketSize;

	proxy=NULL;
	deviceProxy=_deviceProxy;
	manager=_manager;
}

RelayWriter::~RelayWriter() {
	filters.clear();
	__u8 i,j;
	for (i=0;i<recvQueues.size();i++) {
		mq_attr mqa;
		mq_getattr(recvQueues[i],&mqa);
		if (mqa.mq_curmsgs>0) {
			Packet *p;
			for (j=0;j<mqa.mq_curmsgs;j++) {
				ssize_t rc = mq_receive(recvQueues[i], (char*)&p, sizeof(p), NULL);
				if (rc != sizeof(p)) {
					std::cerr << "Error receiving from mq!\n";
					continue;
				}
				delete(p);
			}
		}
		mq_close(recvQueues[i]);
	}
	recvQueues.clear();
	for (i=0;i<sendQueues.size();i++) {
		mq_attr mqa;
		mq_getattr(sendQueues[i],&mqa);
		if (mqa.mq_curmsgs>0) {
			Packet *p;
			for (j=0;j<mqa.mq_curmsgs;j++) {
				ssize_t rc = mq_receive(sendQueues[i], (char*)&p, sizeof(p), NULL);
				if (rc != sizeof(p)) {
					std::cerr << "Error receiving from mq!\n";
					continue;
				}
				delete(p);
			}
		}
		mq_close(sendQueues[i]);
	}
	sendQueues.clear();
}

#ifdef USEVALGRIND

void RelayWriter::relay_write_setup_valgrind() {
	if (!deviceProxy) {fprintf(stderr,"DeviceProxy not initialized for EP00 writer.\n");return;}
	if (sendQueues.size()==0) {fprintf(stderr,"outQueues not initialized for EP00 writer.\n");return;}

	struct pollfd poll_send;
	poll_send.events=POLLOUT;

	__u8 i,j;
	struct pollfd* pollfds=(pollfd*)calloc(recvQueues.size(),sizeof(pollfd));
	for (i=0;i<recvQueues.size();i++) {
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
	while (!_please_stop) {
		idle=true;
		if (!writing) {
			if (!p) {
				if (!numEvents) {
					i=0;
					p=NULL;
					numEvents=poll(pollfds,recvQueues.size(),500);
					idle=!numEvents;
				}
				while(i<numEvents && (!(pollfds[i].revents&POLLIN))) {i++;}
				if (i<numEvents) {
					ssize_t rc = mq_receive(pollfds[i].fd, (char*)&p, sizeof(p), NULL);
					if (rc != sizeof(p)) {
						p = nullptr;
						numEvents = 0;
						std::cerr << "Error receiving from mq!\n";
						continue;
					}
					pollfds[i].revents=0;
					p->source=sendQueues[i];
				}
				if (i>=numEvents) numEvents=0;
			}
			if (p) {
				for(j=0; j<filters.size() && p->filter_out; j++)
					if (filters[j]->test_setup_packet(p,true))
						filters[j]->filter_setup_packet(p,true);
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
					for(; j<filters.size() && p->filter_in; j++)
						if (filters[j]->test_setup_packet(p,false))
							filters[j]->filter_setup_packet(p,false);
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
	}
	fprintf(stderr,"Finished setup writer thread (%ld) for EP%02x.\n",gettid(),endpoint);
	free(pollfds);
	_please_stop = true;
}

void RelayWriter::relay_write_valgrind() {
	if (!endpoint) {
		relay_write_setup_valgrind();
		return;
	}

	__u8 i,j;

	struct pollfd* pollfds=(pollfd*)calloc(recvQueues.size(),sizeof(pollfd));
	for (i=0;i<recvQueues.size();i++) {
		pollfds[i].fd=recvQueues[i];
		pollfds[i].events=POLLIN;
	}

	bool idle=true;
	bool writing=false;
	Packet *p=NULL;
	int numEvents=0;

	fprintf(stderr,"Starting writer thread (%ld) for EP%02x.\n",gettid(),endpoint);
	while (!_please_stop) {
		idle=true;
		if (!writing) {
			if (!p) {
				if (!numEvents) {
					i=0;
					p=NULL;
					numEvents=poll(pollfds,recvQueues.size(),500);
					idle=!numEvents;
				}
				while(i<numEvents && (!(pollfds[i].revents&POLLIN))) {i++;}
				if (i<numEvents) {
					ssize_t rc = mq_receive(pollfds[i].fd, (char*)&p, sizeof(p), NULL);
					if (rc != sizeof(p)) {
						p = nullptr;
						numEvents = 0;
						std::cerr << "Error receiving from mq (thread " << gettid() << ")!\n";
						continue;
					}
					pollfds[i].revents=0;
				}
				if (i>=numEvents) numEvents=0;
			}
			if (p) {
				for(j=0; j<filters.size() && p->filter; j++)
					if (filters[j]->test_packet(p))
						filters[j]->filter_packet(p);
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
	}
	fprintf(stderr,"Finished writer thread (%ld) for EP%02x.\n",gettid(),endpoint);
	free(pollfds);
	_please_stop = false;
}

#endif //USEVALGRIND
void RelayWriter::relay_write_setup() {
	if (!deviceProxy) {
		fprintf(stderr,"DeviceProxy not initialized for EP00 writer.\n");
		return;
	}
	if (sendQueues.size()==0) {
		fprintf(stderr,"outQueues not initialized for EP00 writer.\n");
		return;
	}

	struct pollfd poll_send;
	poll_send.events=POLLOUT;

	__u8 i,j;
	int efd=epoll_create1(EPOLL_CLOEXEC);
	struct epoll_event* events=(epoll_event*)calloc(recvQueues.size(),sizeof(epoll_event));
	if (efd<0) fprintf(stderr,"Error creating epoll fd %d [%s].\n",errno,strerror(errno));
	for (i=0;i<recvQueues.size();i++) {
		struct epoll_event event;
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
	while (!_please_stop) {
		idle=true;
		if (!writing) {
			if (!p) {
				if (!numEvents) {
					i=0;
					p=NULL;
					numEvents=epoll_wait(efd,events,recvQueues.size(),500);
				}
				if (i<numEvents && (events[i].events&EPOLLIN)) {
					int recvQueue=events[i].data.u64>>32;
					int sendQueue=events[i].data.u64&(__u64)0xffffffff;
					ssize_t rc = mq_receive(recvQueue, (char*)&p, sizeof(p), NULL);
					if (rc != sizeof(p)) {
						p = nullptr;
						numEvents = 0;
						std::cerr << "Error receiving from mq (thread " << gettid() << ")!\n";
						continue;
					}
					p->source=sendQueue;
					i++;
				}
				if (i>=numEvents) numEvents=0;
			}
			if (p) {
				for(j=0; j<filters.size() && p->filter_out; j++)
					if (filters[j]->test_setup_packet(p,true))
						filters[j]->filter_setup_packet(p,true);
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
					for(;j<filters.size() && p->filter_in; j++)
						if (filters[j]->test_setup_packet(p,false))
							filters[j]->filter_setup_packet(p,false);
					mq_send(p->source,(char*)&p,4,0);
					poll_send.fd=p->source;
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
	}
	fprintf(stderr,"Finished setup writer thread (%ld) for EP%02x.\n",gettid(),endpoint);
	free(events);
	close(efd);
	_please_stop = false;
}

void RelayWriter::relay_write() {
	if (!endpoint) {
		relay_write_setup();
		return;
	}

	__u8 i, j;
	int efd=epoll_create1(EPOLL_CLOEXEC);
	if (efd<0) fprintf(stderr,"Error creating epoll fd %d [%s].\n",errno,strerror(errno));
	struct epoll_event* events=(epoll_event*)calloc(recvQueues.size(),sizeof(epoll_event));
	for (i=0;i<recvQueues.size();i++) {
		struct epoll_event event;
		event.data.fd=recvQueues[i];
		event.events=EPOLLIN;
		epoll_ctl(efd,EPOLL_CTL_ADD,recvQueues[i],&event);
	}

	bool idle=true;
	bool writing=false;
	Packet *p=NULL;
	int numEvents=0;

	fprintf(stderr,"Starting writer thread (%ld) for EP%02x.\n",gettid(),endpoint);
	while (!_please_stop) {
		idle=true;
		if (!writing) {
			if (!p) {
				if (!numEvents) {
					i=0;
					p=NULL;
					numEvents=epoll_wait(efd,events,recvQueues.size(),500);
					idle=!numEvents;
				}
				if (i<numEvents && (events[i].events&EPOLLIN)) {
					ssize_t rc = mq_receive(events[i++].data.fd, (char*)&p, sizeof(p), NULL);
					if (rc != sizeof(p)) {
						p = nullptr;
						numEvents = 0;
						std::cerr << "Error receiving from mq (thread " << gettid() << ")!\n";
						continue;
					}
				}
				if (i>=numEvents) numEvents=0;
			}
			if (p) {
				for(j=0; j<filters.size(); j++) {
					if (filters[j]->test_packet(p)) {
						filters[j]->filter_packet(p);
					}
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
	}
	fprintf(stderr,"Finished writer thread (%ld) for EP%02x.\n",gettid(),endpoint);
	free(events);
	close(efd);
	_please_stop = false;
}

void RelayWriter::add_filter(PacketFilter* filter) {
	filters.push_back(filter);
}

void RelayWriter::add_queue(mqd_t recvQueue) {
	recvQueues.push_back(recvQueue);
}

void RelayWriter::add_setup_queue(mqd_t recvQueue,mqd_t sendQueue) {
	recvQueues.push_back(recvQueue);
	sendQueues.push_back(sendQueue);
}

void* RelayWriter::relay_write_helper(void* context) {
#ifdef USEVALGRIND
	if (RUNNING_ON_VALGRIND) {
		((RelayWriter*)context)->relay_write_valgrind();
	} else {
		((RelayWriter*)context)->relay_write();
	}
#else //NVALGRIND _IS_ defined
	((RelayWriter*)context)->relay_write();
#endif //NVALGRIND
	return 0;
}

