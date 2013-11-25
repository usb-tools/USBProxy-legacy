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
 * USBPacketFilter_ROT13.h
 *
 * Created on: Nov 24, 2013
 */
#ifndef USBPACKETFILTER_ROT13_H_
#define USBPACKETFILTER_ROT13_H_

#include "USBPacketFilter.h"

class USBPacketFilter_ROT13: public USBPacketFilter {
public:
	USBPacketFilter_ROT13() {}
	virtual ~USBPacketFilter_ROT13() {}
	void filter_packet(USBPacket* packet) {
		int i;
		for (i=2;i<8;i++) {
			if (packet->data[i]<=0x1d && packet->data[i]>=0x04) {
				packet->data[i]=0x21-packet->data[i];
			}
		}
	}
	virtual char* toString() {return (char*)"ROT13 Filter";}
};

#endif /* USBPACKETFILTER_ROT13_H_ */
