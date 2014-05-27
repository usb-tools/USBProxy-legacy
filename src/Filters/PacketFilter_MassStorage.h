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
#include <linux/types.h>
#include <mqueue.h>
#include <poll.h>

//writes all traffic to a stream
class PacketFilter_MassStorage : public PacketFilter {
private:
	int state;
	int flag;
	char tag[4];
	__u8 haltSignal;
	mqd_t sendQueue;
	bool halt;
	struct pollfd haltpoll;
	int haltfd;
	struct pollfd poll_out;

public:
	PacketFilter_MassStorage();
	void filter_packet(Packet* packet);
	void start_queue();
	void send_packet();
	virtual char* toString() {return (char*)"Mass Storage Filter";}
};
#endif /* PACKETFILTER_MASSSTORAGE_H */
