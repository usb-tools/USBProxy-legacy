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
 * PacketFilter.h
 *
 * Created on: Nov 11, 2013
 */
#ifndef USBPROXY_PACKETFILTER_H
#define USBPROXY_PACKETFILTER_H

#include <stdlib.h>
#include "Device.h"
#include "Configuration.h"
#include "Interface.h"
#include "Endpoint.h"
#include "Packet.h"
#include "HexString.h"

struct packet_filter_endpoint {
	__u8 address;
	__u8 addressMask;
	__u8 attributes;
	__u8 attributesMask;
	__u16 packetSizeMin;
	__u16 packetSizeMax;
	__u8 intervalMin;
	__u8 intervalMax;

	packet_filter_endpoint():
		address(0),
		addressMask(0),
		attributes(0),
		attributesMask(0),
		packetSizeMin(0),
		packetSizeMax(65535),
		intervalMin(0),
		intervalMax(255) {}
};

struct packet_filter_interface {
	short number;
	short alternate;
	short deviceClass;
	short subClass;
	short protocol;

	packet_filter_interface():
		number(-1),
		alternate(-1),
		deviceClass(-1),
		subClass(-1),
		protocol(-1) {}
};

struct packet_filter_configuration {
	short number;
	__u8 attributes;
	__u8 attributesMask;
	__u8 highSpeed;

	packet_filter_configuration():
		number(-1),
		attributes(0),
		attributesMask(0),
		highSpeed(255) {}
};

struct packet_filter_device {
	short deviceClass;
	short subClass;
	short protocol;
	__u8 ep0packetSizeMin;
	__u8 ep0packetSizeMax;
	int vendor;
	int product;
	int release;

	packet_filter_device():
		deviceClass(-1),
		subClass(-1),
		protocol(-1),
		ep0packetSizeMin(0),
		ep0packetSizeMax(255),
		vendor(-1),
		product(-1),
		release(-1) {}
};

class PacketFilter {
private:
	__u8 packetHeader[8];
	__u8 packetHeaderMask[8];
	__u8 packetHeaderMaskLength;

public:
	packet_filter_endpoint endpoint;
	packet_filter_interface interface;
	packet_filter_configuration configuration;
	packet_filter_device device;


	PacketFilter() {
		int i;
		for (i=0;i<8;i++) {packetHeader[i]=0;packetHeaderMask[i]=0;}
		packetHeaderMaskLength=0;
	}
	virtual ~PacketFilter() {};

	virtual void filter_packet(Packet* packet) {}
	virtual void filter_setup_packet(SetupPacket* packet) {}

	bool test_device(Device* _device);
	bool test_configuration(Configuration* _configuration);
	bool test_interface(Interface* _interface);
	bool test_endpoint(Endpoint* _endpoint);
	bool test_packet(Packet* packet);
	bool test_setup_packet(SetupPacket* packet);
	void set_packet_filter(__u8 header[4],__u8 mask[4]);
	virtual char* toString() {return (char*)"Filter";}
};

//writes all traffic to a stream
class PacketFilter_streamlog : public PacketFilter {
private:
	FILE* file;
public:
	PacketFilter_streamlog(FILE* _file) {file=_file;}
	void filter_packet(Packet* packet) {
		if (packet->wLength<=64) {
			char* hex=hex_string((void*)packet->data,packet->wLength);
			fprintf(file,"%02x[%d]: %s\n",packet->bEndpoint,packet->wLength,hex);
			free(hex);
		}
	}
	void filter_setup_packet(SetupPacket* packet) {
		if (packet->ctrl_req.wLength && packet->data) {
			char* hex_setup=hex_string(&(packet->ctrl_req),sizeof(packet->ctrl_req));
			char* hex_data=hex_string((void*)(packet->data),packet->ctrl_req.wLength);
			fprintf(file,"[%s]: %s\n",hex_setup,hex_data);
			free(hex_data);
			free(hex_setup);
		} else {
			char* hex_setup=hex_string(&(packet->ctrl_req),sizeof(packet->ctrl_req));
			fprintf(file,"[%s]\n",hex_setup);
			free(hex_setup);
		}
	}
	virtual char* toString() {return (char*)"Stream Log Filter";}
};

//uses function pointers to filter packets
class PacketFilter_Callback : public PacketFilter {
private:
	void (*cb)(Packet*);
	void (*cb_setup)(SetupPacket*);
public:
	PacketFilter_Callback(void (*_cb)(Packet*),void (*_cb_setup)(SetupPacket*)) {cb=_cb;cb_setup=_cb_setup;}
	void filter_packet(Packet* packet) {cb(packet);}
	void filter_setup_packet(SetupPacket* packet) {cb_setup(packet);}
	virtual char* toString() {return (char*)"Filter";}

};

#endif /* USBPROXY_PACKETFILTER_H */
