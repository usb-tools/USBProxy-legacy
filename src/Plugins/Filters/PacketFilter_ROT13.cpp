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
 * PacketFilter_ROT13.cpp
 *
 * Created on: Dec 10, 2013
 */

#include "PacketFilter_ROT13.h"

void PacketFilter_ROT13::filter_packet(Packet* packet) {
	int i;
	for (i=2;i<8;i++) {
		if (packet->data[i]<=0x1d && packet->data[i]>=0x04) {
			if(packet->data[i]<=0x10)
				packet->data[i]=packet->data[i]+13;
			else
				packet->data[i]=packet->data[i]-13;
		}
	}
}

static PacketFilter_ROT13 *proxy;

extern "C" {
	int plugin_type = PLUGIN_FILTER;
	
	PacketFilter * get_plugin(ConfigParser *cfg) {
		proxy = new PacketFilter_ROT13(cfg);
		return (PacketFilter *) proxy;
	}
	
	void destroy_plugin() {
		delete proxy;
	}
}
