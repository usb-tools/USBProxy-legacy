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
 * PacketFilter_Python.h
 *
 * Created on: Jan 10, 2014
 */
#ifndef USBPROXY_PACKETFILTER_PYTHON_H
#define USBPROXY_PACKETFILTER_PYTHON_H

#include "PacketFilter.h"

extern "C" {
#include "Python.h"
}

//uses function pointers to filter packets
class PacketFilter_Python : public PacketFilter {
private:
	PyObject *p_module, *p_filter_func, *p_setup_func, *p_usbproxy;
	char* filter_name;

public:
	static int debugLevel;
	PacketFilter_Python(char* modulename);
	~PacketFilter_Python();
	void filter_packet(Packet* packet);
	void filter_setup_packet(SetupPacket* packet, bool direction_out);
	char* toString() {return (char*)"Python Filter";}
};

#endif /* USBPROXY_PACKETFILTER_CALLBACK_H */
