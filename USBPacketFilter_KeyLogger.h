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
 * USBPacketFilter_KeyLogger.h
 *
 * Created on: Nov 23, 2013
 */
#ifndef USBPACKETFILTER_KEYLOGGER_H_
#define USBPACKETFILTER_KEYLOGGER_H_

#include "USBPacketFilter.h"
#include <stdio.h>

class USBPacketFilter_KeyLogger : public USBPacketFilter {
private:
	FILE* file;
	char lastReport[8];
	void keyPressed(__u8 keyCode,__u8 mods);
	char* keyMap[0x66];
	char* shiftKeyMap[0x66];
public:
	USBPacketFilter_KeyLogger(FILE* _file);
	void filter_packet(USBPacket* packet);
	virtual char* toString() {return (char*)"Key Logger Filter";}
};

#endif /* USBPACKETFILTER_KEYLOGGER_H_ */
