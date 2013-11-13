/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
 *
 * This file is part of USB-MitM.
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
 * USBRelayer.cpp
 *
 * Created on: Nov 11, 2013
 */
#include "USBRelayer.h"

USBRelayer::USBRelayer(USBEndpoint* _endpoint,USBDeviceProxy* _device,USBHostProxy* _host,boost::lockfree::queue<USBPacket*>* _queue) {
	endpoint=_endpoint;
	device=_device;
	host=_host;
	queue=_queue;
	filters=NULL;
	filterCount=0;
	halt=false;
}

USBRelayer::~USBRelayer() {
	if (filters) {free(filters);}
}

void USBRelayer::add_filter(USBPacketFilter* filter) {
	if (filterCount) {
		filters=(USBPacketFilter**)realloc(filters,(filterCount+1)*sizeof(USBPacketFilter*));
	} else {
		filters=(USBPacketFilter**)malloc(sizeof(USBPacketFilter*));
	}
	filters[filterCount]=filter;
	filterCount++;
}

void USBRelayer::relay() {
	__u8 epAddress=endpoint->get_descriptor()->bEndpointAddress;
	__u8* buf;
	int length;
	USBPacket* p;
	USBPacketFilter* f;
	if (epAddress&0x80) { //device->host
		while (!halt) {
			device->receive_data(epAddress,&buf,&length);
			if (length) {
				p=new USBPacket(epAddress,buf,length);
				__u8 i=0;
				while (i<filterCount && p->filter) {
					filters[i]->filter_packet(p,NULL);
					i++;
				}
				if (p->transmit) {host->send_data(epAddress,p->data,p->wLength);}
				delete(p);
			}
			if (queue->pop(p)) {
				__u8 i=0;
				while (i<filterCount && p->filter) {
					filters[i]->filter_packet(p,NULL);
					i++;
				}
				if (p->transmit) {host->send_data(epAddress,p->data,p->wLength);}
				delete(p);
			}
		}
	} else {
		while (!halt) { //host->device
			host->receive_data(epAddress,&buf,&length);
			if (length) {
				p=new USBPacket(epAddress,buf,length);
				__u8 i=0;
				while (i<filterCount && p->filter) {
					filters[i]->filter_packet(p,NULL);
					i++;
				}
				if (p->transmit) {device->send_data(epAddress,p->data,p->wLength);}
				delete(p);
			}
			if (queue->pop(p)) {
				__u8 i=0;
				while (i<filterCount && p->filter) {
					filters[i]->filter_packet(p,NULL);
					i++;
				}
				if (p->transmit) {device->send_data(epAddress,p->data,p->wLength);}
				delete(p);
			}
		}
	}
}

void* USBRelayer::relay_helper(void* context) {
	((USBRelayer*)context)->relay();
	return 0;
};
