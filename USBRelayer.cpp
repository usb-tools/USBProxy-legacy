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
#include "TRACE.h"

#define SLEEP_US 1000

USBRelayer::USBRelayer(USBEndpoint* _endpoint,USBDeviceProxy* _device,USBHostProxy* _host,boost::lockfree::queue<USBPacket*>* _queue) {
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

USBRelayer::USBRelayer(USBManager* _manager,USBEndpoint* _endpoint,USBDeviceProxy* _device,USBHostProxy* _host,boost::lockfree::queue<USBSetupPacket*>* _queue) {
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

USBRelayer::~USBRelayer() {
	if (filters) {
		free(filters);
		filters=NULL;
	}
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

void USBRelayer::relay_ep0() {
	fprintf(stderr,"Starting relaying for EP00.\n");
	__u8 bmAttributes=endpoint->get_descriptor()->bmAttributes;
	__u16 maxPacketSize=endpoint->get_descriptor()->wMaxPacketSize;
	__u8* buf=NULL;
	int response_length=0;
	USBSetupPacket* p;
	usb_ctrlrequest ctrl_req;
	while (!halt) {
		bool idle=true;
		host->control_request(&ctrl_req,&response_length,&buf);
		if (ctrl_req.bRequest) {
			p=new USBSetupPacket(ctrl_req,buf);
			__u8 i=0;
			while (i<filterCount && p->filter) {
				if (filters[i]->test_setup_packet(p)) {filters[i]->filter_setup_packet(p);}
				i++;
			}
			if (p->transmit) {
				if (ctrl_req.bRequestType&0x80) { //device->host
					p->data=(__u8*)malloc(ctrl_req.wLength);
					if (device->control_request(&(p->ctrl_req),&response_length,p->data)==-1) {
						host->stall_ep(0);
					} else {
						i=0;
						p->ctrl_req.wLength=response_length;
						while (i<filterCount && p->filter) {
							if (filters[i]->test_setup_packet(p)) {filters[i]->filter_setup_packet(p);}
							i++;
						}
						if (p->transmit) {
							if (response_length) {
								host->send_data(0,bmAttributes,maxPacketSize,p->data,response_length);
							} else {
								host->control_ack();
							}
						} else {
							host->stall_ep(0);
						}
					}
				} else { //host->device
					response_length=ctrl_req.wLength;
					if (device->control_request(&(p->ctrl_req),&response_length,p->data)==-1) {
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
					device->control_request(&(p->ctrl_req),&response_length,p->data);
				} else { //host->device
					response_length=ctrl_req.wLength;
					device->control_request(&(p->ctrl_req),&response_length,p->data);
					response_length=0;
				}
			}
			if (response_length) {
				host->send_data(0,bmAttributes,maxPacketSize,p->data,response_length);
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
	fprintf(stderr,"Finished relaying for EP00.\n");
}

void USBRelayer::relay() {
	__u8 epAddress=endpoint->get_descriptor()->bEndpointAddress;
	if (!epAddress) {relay_ep0();return;}
	fprintf(stderr,"Starting relaying for EP%02x.\n",epAddress);
	__u8 bmAttributes=endpoint->get_descriptor()->bmAttributes;
	__u16 maxPacketSize=endpoint->get_descriptor()->wMaxPacketSize;
	__u8* buf=NULL;
	int length=0;
	USBPacket* p;
	if (epAddress&0x80) { //device->host
		while (!halt) {
			bool idle=true;
			device->receive_data(epAddress,bmAttributes,maxPacketSize,&buf,&length);
			if (length) {
				p=new USBPacket(epAddress,buf,length);
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
			host->receive_data(epAddress,bmAttributes,maxPacketSize,&buf,&length);
			if (length) {
				p=new USBPacket(epAddress,buf,length);
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
	fprintf(stderr,"Finished relaying for EP%02x.\n",epAddress);
}

void* USBRelayer::relay_helper(void* context) {
	((USBRelayer*)context)->relay();
	return 0;
}
