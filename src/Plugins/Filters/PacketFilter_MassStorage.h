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
#include <map>

//writes all traffic to a stream
class PacketFilter_MassStorage : public PacketFilter, public Injector {
private:
	int state;
	char tag[4];
	int pipe_fd[2]; /* [read, write] */

	/* config flags */
	bool block_writes;
	bool cache_blocks;
	
	/* Block caching */
	__u32 block_count;
	__u32 block_offset;
	__u32 base_address;
	std::map<__u32, __u8*> block_cache;
	void cache_read(__u32 address, __u8 *data);
	void cache_write(__u32 address, __u8 *data);
	void print_block_diff(__u8 *olddata, __u8 *newdata);
	char printable(__u8 in);

public:
	PacketFilter_MassStorage(ConfigParser *cfg);
	~PacketFilter_MassStorage();
	
	/* Filter functions */
	void filter_packet(Packet* packet);
	void queue_packet();
	
	/* Injector functions */
	void start_injector();
	int *get_pollable_fds();
	void stop_injector();
	void get_packets(Packet** packet, SetupPacket** setup, int timeout);
	void full_pipe(Packet* p);
};

#endif /* PACKETFILTER_MASSSTORAGE_H */
