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
 * PacketFilter_UDPHID.cpp
 *
 * Created on: Feb 22, 2014
 */
#include "PacketFilter_UDPHID.h"

PacketFilter_UDPHID::PacketFilter_UDPHID(ConfigParser *cfg) {
	this->interface.deviceClass=0xff;
	this->interface.subClass=0x5d;
	this->endpoint.address=0x80;
	this->endpoint.addressMask=0x80;
	this->packetHeaderMaskLength=2;
	this->packetHeader[0]=0;
	this->packetHeader[1]=20;
	this->packetHeaderMask[0]=0xff;
	this->packetHeaderMask[1]=0xff;
	this->packetHeaderSetupIn=false;
	this->packetHeaderSetupOut=false;
	Injector_UDPHID* injector = (Injector_UDPHID *) cfg->get_pointer("PacketFilter_UDPHID::injector");
	reportBuffer=injector->getReportBuffer();
}

void PacketFilter_UDPHID::filter_packet(Packet* packet) {
	int i;
	if (packet->wLength!=20) return;
	packet->data[2]=packet->data[2]|reportBuffer[2];
	packet->data[3]=packet->data[3]|reportBuffer[3];
	for (i=4;i<=13;i++) {
		if (reportBuffer[i]) packet->data[i]=reportBuffer[i];
	}
}
void PacketFilter_UDPHID::filter_setup_packet(SetupPacket* packet,bool direction) {
}

static PacketFilter_UDPHID *proxy;

extern "C" {
	int plugin_type = PLUGIN_FILTER;
	
	PacketFilter * get_plugin(ConfigParser *cfg) {
		proxy = new PacketFilter_UDPHID(cfg);
		return (PacketFilter *) proxy;
	}
	
	void destroy_plugin() {
		delete proxy;
	}
}
