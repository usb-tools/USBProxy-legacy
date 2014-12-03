/*
 * This file is part of USBProxy.
 */

#ifndef PACKETFILTER_MASSSTORAGE_H
#define PACKETFILTER_MASSSTORAGE_H

#include "PacketFilter.h"
#include "Injector.h"
#include <map>
#include <string>

//writes all traffic to a stream
class PacketFilter_MassStorage : public PacketFilter, public Injector {
private:
	int state;
	char tag[4];
	int pipe_fd[2]; /* [read, write] */

	/* config flags */
	bool block_writes;
	bool cache_blocks;
	bool inband_signalling;
	bool inband_block_writes;
	std::string unblock_password;
	std::string block_password;

	/* Block caching */
	__u32 block_count;
	__u32 block_offset;
	__u32 base_address;
	std::map<__u32, __u8*> block_cache;
	void cache_read(__u32 address, __u8 *data);
	void cache_write(__u32 address, __u8 *data);
	void print_block_diff(__u8 *olddata, __u8 *newdata);
	char printable(__u8 in);

	/* In-Band Signalling */
	void check_for_password(__u8 *data);

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
