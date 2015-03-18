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

#include "Packet.h"
#include "RelayWriter.h"

#include "Endpoint.h"
#include "Proxy.h"
#include "DeviceProxy.h"
#include "PacketFilter.h"
#include "Manager.h"

#define TRANSMIT_TIMEOUT_MS 500
#define READ_TIMEOUT_MS 500

RelayWriter::RelayWriter(Endpoint* _endpoint,Proxy* _proxy, PacketQueue& recvQueue)
	: _please_stop(false)
	, _recvQueue(&recvQueue)
	, _sendQueue(0)
{

	endpoint=_endpoint->get_descriptor()->bEndpointAddress;
	attributes=_endpoint->get_descriptor()->bmAttributes;
	maxPacketSize=_endpoint->get_descriptor()->wMaxPacketSize;

	proxy=_proxy;
	deviceProxy=NULL;
	manager=NULL;
}

RelayWriter::RelayWriter(Endpoint* _endpoint,DeviceProxy* _deviceProxy,Manager* _manager, PacketQueue& recvQueue, PacketQueue& sendQueue)
	: _please_stop(false)
	, _recvQueue(&recvQueue)
	, _sendQueue(&sendQueue)
{
	//_sendQueues.push_back(&sendQueue);

	endpoint=_endpoint->get_descriptor()->bEndpointAddress;
	attributes=_endpoint->get_descriptor()->bmAttributes;
	maxPacketSize=_endpoint->get_descriptor()->wMaxPacketSize;

	proxy=NULL;
	deviceProxy=_deviceProxy;
	manager=_manager;
}

RelayWriter::~RelayWriter() {
	filters.clear();
}

void RelayWriter::relay_write_setup() {
	if (!deviceProxy) {
		fprintf(stderr,"DeviceProxy not initialized for EP00 writer.\n");
		return;
	}
	if (!_sendQueue) {
		fprintf(stderr,"outQueue not initialized for EP00 writer.\n");
		return;
	}
	if (!_recvQueue) {
		fprintf(stderr,"inQueue not initialized for EP00 writer.\n");
		return;
	}


	__u8 j;

	PacketPtr p;
	int length;
	usb_ctrlrequest ctrl_req;

	fprintf(stderr,"Starting setup writer thread (%ld) for EP%02x.\n",gettid(),endpoint);
	while (!_please_stop) {
		p = _recvQueue->dequeue();
		if (_please_stop)
			break;
		if (!p)
			continue;
		SetupPacket* s = dynamic_cast<SetupPacket*>(p.get());
		if (!s)
			continue;
		//int sendQueue=events[i].data.u64&(__u64)0xffffffff;
		//s->source=sendQueue; TODO
		for(j=0; j<filters.size() && s->filter_out; j++)
			if (filters[j]->test_setup_packet(s, true))
				filters[j]->filter_setup_packet(s, true);
		ctrl_req=s->ctrl_req;
		if (!s->transmit_out)
			continue;
		if (ctrl_req.bRequestType&0x80) { //device->host
			s->data=(__u8*)malloc(ctrl_req.wLength);
			s->transmit_in = (deviceProxy->control_request(&(s->ctrl_req), &length, s->data, TRANSMIT_TIMEOUT_MS) >= 0);
			j=0;
			s->ctrl_req.wLength=length;
		} else { //host->device
			length=ctrl_req.wLength;
			s->transmit_in = (deviceProxy->control_request(&(s->ctrl_req), &length, s->data, TRANSMIT_TIMEOUT_MS) >= 0);
			if (s->ctrl_req.bRequest==9 && s->ctrl_req.bRequestType==0) {manager->setConfig(s->ctrl_req.wValue);}
			s->ctrl_req.wLength=0;
		}
		for(;j<filters.size() && s->filter_in; j++)
			if (filters[j]->test_setup_packet(s, false))
				filters[j]->filter_setup_packet(s, false);
		_sendQueue->enqueue(p);
	}
	fprintf(stderr,"Finished setup writer thread (%ld) for EP%02x.\n",gettid(),endpoint);
	_please_stop = false;
}

void RelayWriter::relay_write() {
	if (!endpoint) {
		relay_write_setup();
		return;
	}

	bool writing=false;
	PacketPtr p;

	fprintf(stderr,"Starting writer thread (%ld) for EP%02x.\n",gettid(),endpoint);
	while (!_please_stop) {
		if (!writing) {
			p = _recvQueue->dequeue();
			if (!p)
				continue;
			for(size_t j=0; j<filters.size(); j++) {
				if (filters[j]->test_packet(p.get())) {
					filters[j]->filter_packet(p.get());
				}
			}
			if (p->transmit) {
				proxy->send_data(endpoint,attributes,maxPacketSize,p->data,p->wLength);
				writing=true;
			}
		} else {
			writing=!(proxy->send_wait_complete(endpoint, READ_TIMEOUT_MS));
		}
	}
	fprintf(stderr,"Finished writer thread (%ld) for EP%02x.\n",gettid(),endpoint);
	_please_stop = false;
}

void RelayWriter::add_filter(PacketFilter* filter) {
	filters.push_back(filter);
}
