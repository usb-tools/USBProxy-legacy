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
#include <stdio.h>

#include "Plugins.h"
#include "Device.h"
#include "Configuration.h"
#include "Interface.h"
#include "Endpoint.h"
#include "Packet.h"
#include "HexString.h"
#include "Criteria.h"
#include "ConfigParser.h"

class PacketFilter {
protected:
	__u8 packetHeader[8];
	__u8 packetHeaderMask[8];
	__u8 packetHeaderMaskLength;
	bool packetHeaderSetupOut;
	bool packetHeaderSetupIn;

public:
	static const __u8 plugin_type=PLUGIN_FILTER;
	struct criteria_endpoint endpoint;
	struct criteria_interface interface;
	struct criteria_configuration configuration;
	struct criteria_device device;

	PacketFilter() {
		int i;
		for (i=0;i<8;i++) {packetHeader[i]=0;packetHeaderMask[i]=0;}
		packetHeaderMaskLength=0;
		packetHeaderSetupIn=true;
		packetHeaderSetupOut=true;
	}
	virtual ~PacketFilter() {};

	virtual void filter_packet(Packet* packet) {}
	virtual void filter_setup_packet(SetupPacket* packet,bool direction_out) {}

	bool test_packet(Packet* packet);
	bool test_setup_packet(SetupPacket* packet,bool direction_out);
	void set_packet_filter(__u8 header[4],__u8 mask[4]);
	virtual char* toString() {return (char*)"Filter";}
};

extern "C" {
	PacketFilter *get_packetfilter_plugin();
}
#endif /* USBPROXY_PACKETFILTER_H */
