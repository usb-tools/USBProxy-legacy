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

	virtual void filter_packet(USBPacket* packet)=0;
	virtual void filter_setup_packet(USBSetupPacket* packet)=0;

	bool test_device(USBDevice* _device);
	bool test_configuration(USBConfiguration* _configuration);
	bool test_interface(USBInterface* _interface);
	bool test_endpoint(USBEndpoint* _endpoint);
	bool test_packet(USBPacket* packet);
	bool test_setup_packet(USBSetupPacket* packet);
	void set_packet_filter(__u8 header[4],__u8 mask[4]);
	virtual char* toString() {return (char*)"Filter";}
};

//writes all traffic to a stream
class USBPacketFilter_streamlog : public USBPacketFilter {
private:
	FILE* file;
public:
	USBPacketFilter_streamlog(FILE* _file) {file=_file;}
	void filter_packet(USBPacket* packet) {
		fprintf(file,"%02x[%d]:",packet->bEndpoint,packet->wLength);
		int i;
		for(i=0;i<packet->wLength;i++) {fprintf(file," %02x",packet->data[i]);}
		fprintf(file,"\n");
	}
	void filter_setup_packet(USBSetupPacket* packet) {
		__u8* req=(__u8*)&(packet->ctrl_req);
		fprintf(file,"SETUP[%02x%02x%02x%02x%02x%02x%02x%02x]:",req[0],req[1],req[2],req[3],req[4],req[5],req[6],req[7]);
		int i;
		for(i=0;i<packet->ctrl_req.wLength;i++) {fprintf(file," %02x",packet->data[i]);}
		fprintf(file,"\n");
	}
	virtual char* toString() {return (char*)"Stream Log Filter";}
};

//uses function pointers to filter packets
class USBPacketFilter_Callback : public USBPacketFilter {
private:
	void (*cb)(USBPacket*);
	void (*cb_setup)(USBSetupPacket*);
public:
	USBPacketFilter_Callback(void (*_cb)(USBPacket*),void (*_cb_setup)(USBSetupPacket*)) {cb=_cb;cb_setup=_cb_setup;}
	void filter_packet(USBPacket* packet) {cb(packet);}
	void filter_setup_packet(USBSetupPacket* packet) {cb_setup(packet);}
	virtual char* toString() {return (char*)"Filter";}

};

#endif /* USBPACKETFILTER_H_ */
