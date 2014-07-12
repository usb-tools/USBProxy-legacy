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
 * PacketFilter_UDPHID.h
 *
 * Created on: Feb 22, 2014
 */
#ifndef PACKETFILTER_UDPHID_H_
#define PACKETFILTER_UDPHID_H_

#include "PacketFilter.h"
#include "../Injectors/Injector_UDPHID.h"

//writes all traffic to a stream
class PacketFilter_UDPHID : public PacketFilter {
private:
	__u8* reportBuffer;
public:
	PacketFilter_UDPHID(Injector_UDPHID* injector);
	void filter_packet(Packet* packet);
	void filter_setup_packet(SetupPacket* packet,bool direction);
	virtual char* toString() {return (char*)"UDPHID Filter";}
};
#endif /* PACKETFILTER_UDPHID_H_ */
