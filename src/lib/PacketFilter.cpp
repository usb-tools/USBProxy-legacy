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
 * PacketFilter.cpp
 *
 * Created on: Nov 11, 2013
 */

#include <linux/usb/ch9.h>
#include "PacketFilter.h"
#include "TRACE.h"

void PacketFilter::set_packet_filter(__u8 header[8],__u8 mask[8]) {
	int i;
	packetHeaderMaskLength=1;
	for(i=0;i<8;i++) {
		if (mask[i]) {packetHeaderMaskLength=i;}
	}
}

bool PacketFilter::test_packet(Packet* packet) {
	__u8* data;
	if (packet->wLength<packetHeaderMaskLength) {return false;}
	data=packet->data;
	int i;
	for(i=0;i<packetHeaderMaskLength;i++) {
		if (packetHeaderMask[i] && ((packetHeaderMask[i]&data[i])!=(packetHeaderMask[i]&packetHeader[i]))) {return false;}
	}
	return true;
}

bool PacketFilter::test_setup_packet(SetupPacket* packet,bool direction_out) {
	if (!((packetHeaderSetupOut || (!direction_out)) || (packetHeaderSetupIn || direction_out))) return false;
	__u8* data;
	data=(__u8*)&(packet->ctrl_req);
	int i;
	for(i=0;i<packetHeaderMaskLength;i++) {
		if (packetHeaderMask[i] && ((packetHeaderMask[i]&data[i])!=(packetHeaderMask[i]&packetHeader[i]))) {return false;}
	}
	return true;
}



