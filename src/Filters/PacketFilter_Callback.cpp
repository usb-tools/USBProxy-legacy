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

	PacketFilter_Callback::PacketFilter_Callback(void (*_cb)(Packet*),void (*_cb_setup)(SetupPacket*,bool)) {
		cb=_cb;cb_setup=_cb_setup;
	}
	void PacketFilter_Callback::filter_packet(Packet* packet) {
		cb(packet);
	}
	void PacketFilter_Callback::filter_setup_packet(SetupPacket* packet,bool direction_in) {
		cb_setup(packet,direction_in);
	}
