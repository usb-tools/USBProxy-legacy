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
 * PacketFilter_Callback.cpp
 *
 * Created on: Dec 28, 2013
 */
#include "PacketFilter_Callback.h"

PacketFilter_Callback::PacketFilter_Callback(ConfigParser *cfg) {
	cb = (f_cb) cfg->get_pointer("PacketFilter_Callback::filter_packet");
	cb_setup = (f_cb_setup) cfg->get_pointer("PacketFilter_Callback::filter_setup_packet");
}
void PacketFilter_Callback::filter_packet(Packet* packet) {
	cb(packet);
}

void PacketFilter_Callback::filter_setup_packet(SetupPacket* packet,bool direction_out) {
	cb_setup(packet,direction_out);
}

static PacketFilter_Callback *proxy;

extern "C" {
	int plugin_type = PLUGIN_FILTER;
	
	PacketFilter * get_plugin(ConfigParser *cfg) {
		proxy = new PacketFilter_Callback(cfg);
		return (PacketFilter *) proxy;
	}
	
	void destroy_plugin() {
		delete proxy;
	}
}
