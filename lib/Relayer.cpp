/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
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
 * Relayer.cpp
 *
 * Created on: Nov 11, 2013
 */
#include "TRACE.h"
#include "get_tid.h"

#include "Relayer.h"

#include "Endpoint.h"

#include "DeviceProxy.h"
#include "HostProxy.h"
#include "Packet.h"
#include "PacketFilter.h"
#include "Manager.h"

#define SLEEP_US 1000

Relayer::Relayer(Endpoint* _endpoint,DeviceProxy* _device,HostProxy* _host,boost::lockfree::queue<Packet*>* _queue) {
	endpoint=_endpoint;
	if (!endpoint->get_descriptor()->bEndpointAddress) {fprintf(stderr,"Wrong queue type for EP%d relayer.\n",endpoint->get_descriptor()->bEndpointAddress);}
	device=_device;
	host=_host;
	queue_ep0=NULL;
	queue=_queue;
	filters=NULL;
	filterCount=0;
	halt=false;
	manager=NULL;
}

Relayer::Relayer(Manager* _manager,Endpoint* _endpoint,DeviceProxy* _device,HostProxy* _host,boost::lockfree::queue<SetupPacket*>* _queue) {
	endpoint=_endpoint;
	if (endpoint->get_descriptor()->bEndpointAddress) {fprintf(stderr,"Wrong queue type for EP0 relayer.\n");}
	device=_device;
	host=_host;
	queue_ep0=_queue;
	queue=NULL;
	filters=NULL;
	filterCount=0;
	halt=false;
	manager=_manager;
}

Relayer::~Relayer() {
	if (filters) {
		free(filters);
		filters=NULL;
	}
}

void Relayer::add_filter(PacketFilter* filter) {
	if (filterCount) {
		filters=(PacketFilter**)realloc(filters,(filterCount+1)*sizeof(PacketFilter*));
	} else {
		filters=(PacketFilter**)malloc(sizeof(PacketFilter*));
	}
	filters[filterCount]=filter;
	filterCount++;
}

void Relayer::relay_ep0() {
	fprintf(stderr,"Starting relaying thread (%ld) for EP00.\n",gettid());
	__u8 bmAttributes=endpoint->get_descriptor()->bmAttributes;
	__u16 maxPacketSize=endpoint->get_descriptor()->wMaxPacketSize;
	SetupPacket* p;
	usb_ctrlrequest ctrl_req;
	while (!halt) {
		bool idle=true;
		__u8* buf=NULL;
		int length=0;
		host->control_request(&ctrl_req,&length,&buf);
		if (ctrl_req.bRequest) {
			p=new SetupPacket(ctrl_req,buf);
			__u8 i=0;
			while (i<filterCount && p->filter) {
				if (filters[i]->test_setup_packet(p)) {filters[i]->filter_setup_packet(p);}
				i++;
			}
			if (p->transmit) {
				if (ctrl_req.bRequestType&0x80) { //device->host
					p->data=(__u8*)malloc(ctrl_req.wLength);
					if (device->control_request(&(p->ctrl_req),&length,p->data)==-1) {
						host->stall_ep(0);
					} else {
						i=0;
						p->ctrl_req.wLength=length;
						while (i<filterCount && p->filter) {
							if (filters[i]->test_setup_packet(p)) {filters[i]->filter_setup_packet(p);}
							i++;
						}
						if (p->transmit) {
							if (length) {
								host->send_data(0,bmAttributes,maxPacketSize,p->data,length);
							} else {
								host->control_ack();
							}
						} else {
							host->stall_ep(0);
						}
					}
				} else { //host->device
					length=ctrl_req.wLength;
					if (device->control_request(&(p->ctrl_req),&length,p->data)==-1) {
						host->stall_ep(0);
					} else {
						if (p->ctrl_req.bRequest==9 && p->ctrl_req.bRequestType==0) {manager->setConfig(p->ctrl_req.wValue);}
						host->control_ack();
					}
				}
			}
			delete(p);
			/* not needed p=NULL; */
			idle=false;
		}
		if (queue_ep0->pop(p)) {
			__u8 i=0;
			while (i<filterCount && p->filter) {
				if (filters[i]->test_setup_packet(p)) {filters[i]->filter_setup_packet(p);}
				i++;
			}
			if (p->transmit) {
				if (ctrl_req.bRequestType&0x80) { //device->host
					p->data=(__u8*)malloc(ctrl_req.wLength);
					device->control_request(&(p->ctrl_req),&length,p->data);
				} else { //host->device
					length=ctrl_req.wLength;
					device->control_request(&(p->ctrl_req),&length,p->data);
					length=0;
				}
			}
			if (length) {
				host->send_data(0,bmAttributes,maxPacketSize,p->data,length);
			} else {
				host->send_data(0,bmAttributes,maxPacketSize,NULL,0);
			}

			//TODO send this data back to the injector somehow
			delete(p);
			/* not needed p=NULL; */
			idle=false;
		}
		if (idle) usleep(SLEEP_US);
	}
	fprintf(stderr,"Finished relaying thread(%ld) for EP00.\n",gettid());
}

void Relayer::relay() {
	__u8 epAddress=endpoint->get_descriptor()->bEndpointAddress;
	if (!epAddress) {relay_ep0();return;}
	fprintf(stderr,"Starting relaying thread (%ld) for EP%02x.\n",gettid(),epAddress);
	__u8 bmAttributes=endpoint->get_descriptor()->bmAttributes;
	__u16 maxPacketSize=endpoint->get_descriptor()->wMaxPacketSize;
	Packet* p;
	if (epAddress&0x80) { //device->host
		while (!halt) {
			bool idle=true;
			if (host->send_wait_complete(epAddress)) {
				__u8* buf=NULL;
				int length=0;
				device->receive_data(epAddress,bmAttributes,maxPacketSize,&buf,&length);
				if (length) {
					p=new Packet(epAddress,buf,length);
					__u8 i=0;
					while (i<filterCount && p->filter) {
						if (filters[i]->test_packet(p)) {filters[i]->filter_packet(p);}
						i++;
					}
					if (p->transmit) {host->send_data(epAddress,bmAttributes,maxPacketSize,p->data,p->wLength);}
					delete(p);
					/* not needed p=NULL; */
					idle=false;
				}
			}
			if (queue->pop(p)) {
				__u8 i=0;
				while (i<filterCount && p->filter) {
					if (filters[i]->test_packet(p)) {filters[i]->filter_packet(p);}
					i++;
				}
				if (p->transmit) {host->send_data(epAddress,bmAttributes,maxPacketSize,p->data,p->wLength);}
				delete(p);
				/* not needed p=NULL; */
				idle=false;
			}
			if (idle) usleep(SLEEP_US);
		}
	} else {
		while (!halt) { //host->device
			bool idle=true;
			if (device->send_wait_complete(epAddress)) {
				__u8* buf=NULL;
				int length=0;
				host->receive_data(epAddress,bmAttributes,maxPacketSize,&buf,&length,10);
				if (length) {
					p=new Packet(epAddress,buf,length);
					__u8 i=0;
					while (i<filterCount && p->filter) {
						if (filters[i]->test_packet(p)) {filters[i]->filter_packet(p);}
						i++;
					}
					if (p->transmit) {device->send_data(epAddress,bmAttributes,maxPacketSize,p->data,p->wLength);}
					delete(p);
					/* not needed p=NULL; */
					idle=false;
				}
			}
			if (queue->pop(p)) {
				__u8 i=0;
				while (i<filterCount && p->filter) {
					if (filters[i]->test_packet(p)) {filters[i]->filter_packet(p);}
					i++;
				}
				if (p->transmit) {device->send_data(epAddress,bmAttributes,maxPacketSize,p->data,p->wLength);}
				delete(p);
				/* not needed p=NULL; */
				idle=false;
			}
			if (idle) usleep(SLEEP_US);
		}
	}
	fprintf(stderr,"Finished relaying thread (%ld) for EP%02x.\n",gettid(),epAddress);
}

void* Relayer::relay_helper(void* context) {
	((Relayer*)context)->relay();
	return 0;
}
