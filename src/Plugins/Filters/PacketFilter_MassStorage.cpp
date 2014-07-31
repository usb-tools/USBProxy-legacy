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
 * PacketFilter_MassStorage.cpp
 */

#include "PacketFilter_MassStorage.h"
#include <linux/types.h>
#include <unistd.h>
#include <memory.h>

#define IDLE 0
#define COMMAND 1
#define READ 2
#define WRITE 3
#define STATUS 4
#define UNKNOWN 5

PacketFilter_MassStorage::PacketFilter_MassStorage(ConfigParser *cfg) {
	int rs;
	state = IDLE;
	block_count = 0;

	rs = pipe(pipe_fd);
	if (rs < 0) 
	{
		perror("pipe");
		exit(EXIT_FAILURE);
	}
}

PacketFilter_MassStorage::~PacketFilter_MassStorage() {
	fprintf(stderr, "Destroying mass storage filter - cache size: %d\n", (int) block_cache.size());
}
void PacketFilter_MassStorage::queue_packet() {
	// Write tag to pipe
	write(pipe_fd[1], tag, 4);
}

#define BLOCK_SIZE 512

void PacketFilter_MassStorage::cache_read(__u32 address, __u8 *data) {
	std::map<__u32, __u8*>::iterator cmitr = block_cache.find(address);
    if (cmitr == block_cache.end()) {
        /* New block, add to cache */
		__u8 *block = (__u8*) malloc(BLOCK_SIZE);
		if(block == NULL) {
			fprintf(stderr, "Block cache full! (malloc failed)\n");
			return;
		}
		memcpy(block, data, BLOCK_SIZE);
		block_cache[address] = block;
	} else {
		/* A block in our cache, may have been written, use cached version */
		memcpy(data, cmitr->second, BLOCK_SIZE);
	}
}


#define COL_RED  31
#define COL_BLUE 34
static const char *fmt_colour = "\033[0;%dm%02x\033[0m";
static const char *fmt_colour_chr = "\033[0;%dm%c\033[0m";

char PacketFilter_MassStorage::printable(__u8 in) {
	if(isprint(in))
		return (char) in;
	else
		return '.';
}
void PacketFilter_MassStorage::print_block_diff(__u8 *olddata, __u8 *newdata) {
	int i, j;
	for(i=0;i<BLOCK_SIZE;i+=16) {
		for(j=0;j<16;j++) {
			if(olddata[i+j]!=newdata[i+j])
				fprintf(stderr, fmt_colour, COL_RED, olddata[i+j]);
			else
				fprintf(stderr, "%02x", olddata[i+j]);
			if(j%2==1)
				fprintf(stderr, " ");
		}
		fprintf(stderr, "| ");
		for(j=0;j<16;j++) {
			if(olddata[i+j]!=newdata[i+j])
				fprintf(stderr, fmt_colour, COL_BLUE, newdata[i+j]);
			else
				fprintf(stderr, "%02x", newdata[i+j]);
			if(j%2==1)
				fprintf(stderr, " ");
		}
		for(j=0;j<16;j++) {
			if(olddata[i+j]!=newdata[i+j])
				fprintf(stderr, fmt_colour_chr, COL_RED, printable(olddata[i+j]));
			else
				fprintf(stderr, "%c", printable(olddata[i+j]));
			if(j%2==1)
				fprintf(stderr, " ");
		}
		fprintf(stderr, "| ");
		for(j=0;j<16;j++) {
			if(olddata[i+j]!=newdata[i+j])
				fprintf(stderr, fmt_colour_chr, COL_BLUE, printable(newdata[i+j]));
			else
				fprintf(stderr, "%c", printable(newdata[i+j]));
			if(j%2==1)
				fprintf(stderr, " ");
		}
		fprintf(stderr, "\n");
	}
}

void PacketFilter_MassStorage::cache_write(__u32 address, __u8 *data) {
	std::map<__u32, __u8*>::iterator cmitr = block_cache.find(address);
    if (cmitr == block_cache.end()) {
        /* New block, add to cache */
		__u8 *block = (__u8*) malloc(BLOCK_SIZE);
		if(block == NULL) {
			fprintf(stderr, "Block cache full! (malloc failed)\n");
			return;
		}
		fprintf(stderr, "Writing previously unread block to cache\n");
		memcpy(block, data, BLOCK_SIZE);
		block_cache[address] = block;
	} else {
		/* A block in our cache, print diff */
		fprintf(stderr, "New block written to cache:\n");
		print_block_diff(cmitr->second, data);
		memcpy(cmitr->second, data, BLOCK_SIZE);
	}
}

void PacketFilter_MassStorage::filter_packet(Packet* packet) {
	int type = UNKNOWN;
	if ((packet->wLength == 31) &&
		(packet->data[0] == 0x55) &&
		(packet->data[1] == 0x53) &&
		(packet->data[2] == 0x42) &&
		(packet->data[3] == 0x43)) {
			type = COMMAND;
	}
	if ((packet->wLength == 13) &&
		(packet->data[0] == 0x55) &&
		(packet->data[1] == 0x53) &&
		(packet->data[2] == 0x42) &&
		(packet->data[3] == 0x53)) {
			type = STATUS;
	}

	if((type==UNKNOWN) &&
	   (packet->wLength > 64)) {
		// Probably data
		type = state;
	}

	switch(type) {
		case COMMAND:
			switch(packet->data[0x0f]) {
				case 0x28:
					state = READ;
					block_count = (packet->data[0x16]<<8) | packet->data[0x17];
					base_address = packet->data[0x11]<<24 |
								   packet->data[0x12]<<16 |
								   packet->data[0x13]<<8 |
								   packet->data[0x14];
					block_offset = 0;
					fprintf(stderr, "CBW: Read LBA: 0x%08X, %d blocks\n",
							base_address, block_count);
					break;
				case 0x2a:
					state = WRITE;
					packet->transmit = false;
					tag[0] = packet->data[0x04];
					tag[1] = packet->data[0x05];
					tag[2] = packet->data[0x06];
					tag[3] = packet->data[0x07];
					fprintf(stderr, "CBW: Write, tag: %02x %02x %02x %02x\n", tag[0], tag[1], tag[2], tag[3]);
					block_count = (packet->data[0x16]<<8) | packet->data[0x17];
					base_address = packet->data[0x11]<<24 |
								   packet->data[0x12]<<16 |
								   packet->data[0x13]<<8 |
								   packet->data[0x14];
					block_offset = 0;
					fprintf(stderr, "CBW: Write LBA: 0x%08X, %d blocks\n",
							base_address, block_count);
					break;
				default:
					if(packet->data[0x0f]) // Ignore status ping
						fprintf(stderr, "CBW: (%02x), tag: %02x %02x %02x %02x\n",
								packet->data[0x0f],
								packet->data[0x04],
								packet->data[0x05],
								packet->data[0x06],
								packet->data[0x07]);
					break;
			}
			break;
		
		case WRITE:
			fprintf(stderr, "WRITE: 0x%08X\n", block_offset + base_address);
			cache_write(block_offset + base_address, packet->data);
			packet->transmit = false;
			if(++block_offset == block_count)
				queue_packet();
			break;
		
		case READ:
			fprintf(stderr, "READ: 0x%08X\n", block_offset + base_address);
			cache_read(block_offset + base_address, packet->data);
			++block_offset;
			break;
		
		case STATUS:
			// A CSW (Command Status Wrapper)
			switch(packet->data[12]) {
				case 0:
					if(state==WRITE)
						fprintf(stderr, "CSW: Success, tag: %02x %02x %02x %02x\n",
										packet->data[0x04],
										packet->data[0x05],
										packet->data[0x06],
										packet->data[0x07]);
					break;
				default:
					fprintf(stderr, "CSW: Error(%d)\n", packet->data[0x0c]);
					break;
			}
				state = IDLE;
			break;
	}
}

void PacketFilter_MassStorage::start_injector() {
	// any injector setup stuff goes here
}

int* PacketFilter_MassStorage::get_pollable_fds() {
	// create pollable fd that we prod whenever we have packets ready
	int* tmp=(int*)calloc(2,sizeof(int));
	tmp[0]=pipe_fd[0];
	return tmp;
}

void PacketFilter_MassStorage::stop_injector() {
	close(pipe_fd[0]);
	close(pipe_fd[1]);
}

void PacketFilter_MassStorage::get_packets(Packet** packet, SetupPacket** setup, int timeout) {
	*packet=NULL;
	*setup=NULL;
	char tag_buf[4];
	int status_len = 13;
	read(pipe_fd[0], &tag_buf, 4);
	
	__u8* status_buf = (__u8*)malloc(status_len);
	
	/* Signature */
	status_buf[0] = 0x55;
	status_buf[1] = 0x53;
	status_buf[2] = 0x42;
	status_buf[3] = 0x53;
	/* Tag */
	status_buf[4] = tag_buf[0];
	status_buf[5] = tag_buf[1];
	status_buf[6] = tag_buf[2];
	status_buf[7] = tag_buf[3];
	/* 0x00 - OK */
	status_buf[8] = 0x00;
	status_buf[9] = 0x00;
	status_buf[10] = 0x00;
	status_buf[11] = 0x00;
	status_buf[12] = 0x00;
	
	fprintf(stderr, "Injecting false OK status, tag: %02x %02x %02x %02x\n",
			status_buf[4], status_buf[5], status_buf[6], status_buf[7]);
	*packet = new Packet(0x82, status_buf, status_len);
}

void PacketFilter_MassStorage::full_pipe(Packet* p) {fprintf(stderr,"Packet returned due to full pipe & buffer\n");}

static PacketFilter_MassStorage *proxy;

extern "C" {
	int plugin_type = PLUGIN_FILTER | PLUGIN_INJECTOR;
	
	PacketFilter * get_plugin(ConfigParser *cfg) {
		proxy = new PacketFilter_MassStorage(cfg);
		return (PacketFilter *) proxy;
	}
	
	void destroy_plugin() {
		delete proxy;
	}
}
