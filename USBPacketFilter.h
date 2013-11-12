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
 * USBPacketFilter.h
 *
 * Created on: Nov 11, 2013
 */
#ifndef USBPACKETFILTER_H_
#define USBPACKETFILTER_H_

#include <stdlib.h>
#include "USBDevice.h"
#include "USBConfiguration.h"
#include "USBInterface.h"
#include "USBEndpoint.h"
#include "USBPacket.h"

struct packet_filter_endpoint {
	__u8 address=0;
	__u8 addressMask=0;
	__u8 attributes=0;
	__u8 attributesMask=0;
	__u16 packetSizeMin=0;
	__u16 packetSizeMax=65535;
	__u8 intervalMin=0;
	__u8 intervalMax=255;
};

struct packet_filter_interface {
	short number=-1;
	short alternate=-1;
	short deviceClass=-1;
	short subClass=-1;
	short protocol=-1;
};

struct packet_filter_configuration {
	short number=-1;
	__u8 attributes=0;
	__u8 attributesMask=0;
	__u8 highSpeed=255;
};

struct packet_filter_device {
	short deviceClass=-1;
	short subClass=-1;
	short protocol=-1;
	__u8 ep0packetSizeMin=0;
	__u8 ep0packetSizeMax=255;
	int vendor=-1;
	int product=-1;
	int release=-1;
};

class USBPacketFilter {
private:
	__u8 packetHeader[8]={0,0,0,0,0,0,0,0};
	__u8 packetHeaderMask[8]={0,0,0,0,0,0,0,0};
	__u8 packetHeaderMaskLength=0;

public:
	packet_filter_endpoint endpoint;
	packet_filter_interface interface;
	packet_filter_configuration configuration;
	packet_filter_device device;


	USBPacketFilter();
	virtual ~USBPacketFilter();

	virtual void filter_packet(USBPacket* packet,usb_ctrlrequest *setup_packet=NULL)=0;

	bool test_device(USBDevice* _device);
	bool test_configuration(USBConfiguration* _configuration);
	bool test_interface(USBInterface* _interface);
	bool test_endpoint(USBEndpoint* _endpoint);
	bool test_packet(USBPacket* packet,usb_ctrlrequest *setup_packet=NULL);
	void set_packet_filter(__u8 header[4],__u8 mask[4]);
};

class USBPacketFilter_Callback : public USBPacketFilter {
private:
	void (*cb)(USBPacket*,usb_ctrlrequest*);
public:
	USBPacketFilter_Callback(void (*_cb)(USBPacket*,usb_ctrlrequest*)) {cb=_cb;}
	void filter_packet(USBPacket* packet,usb_ctrlrequest *setup_packet=NULL) {cb(packet,setup_packet);}
	virtual char* toString() {return "Filter";}

};

#endif /* USBPACKETFILTER_H_ */
