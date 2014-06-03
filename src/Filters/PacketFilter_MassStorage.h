/*
 * Copyright 2014 Dominic Spill
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
 * PacketFilter_MassStorage.h
 */
#ifndef PACKETFILTER_MASSSTORAGE_H
#define PACKETFILTER_MASSSTORAGE_H

#include "PacketFilter.h"
#include "Injector.h"

//writes all traffic to a stream
class PacketFilter_MassStorage : public PacketFilter {
private:
	int state;
	char tag[4];
	int pipe_fd[2]; /* [read, write] */
	Packet *p;
	bool packet_waiting;

public:
	PacketFilter_MassStorage();
	
	/* Filter functions */
	void filter_packet(Packet* packet);
	void queue_packet();
};
#endif /* PACKETFILTER_MASSSTORAGE_H */
