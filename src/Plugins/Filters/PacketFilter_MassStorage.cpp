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
	blocks = 0;
	status_buf[0] = 0x55;
	status_buf[1] = 0x53;
	status_buf[2] = 0x42;
	status_buf[3] = 0x53;
	/* Replace the next four with tag */
	status_buf[4] = 0xFF;
	status_buf[5] = 0xFF;
	status_buf[6] = 0xFF;
	status_buf[7] = 0xFF;
	status_buf[8] = 0x00;
	status_buf[9] = 0x00;
	status_buf[10] = 0x00;
	status_buf[11] = 0x00;
	status_buf[812] = 0x00;

	rs = pipe(pipe_fd);
	if (rs < 0) 
	{
		perror("pipe");
		exit(EXIT_FAILURE);
	}
}

void PacketFilter_MassStorage::queue_packet() {
	// Write tag to pipe
	write(pipe_fd[1], tag, 4);
}

void PacketFilter_MassStorage::filter_packet(Packet* packet) {
	int length, type = UNKNOWN;
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
					length = (packet->data[0x16]<<8) | packet->data[0x17];
					fprintf(stderr, "CBW: Read LBA: 0x%02X%02X%02X%02X, %d blocks\n",
							packet->data[0x11],
							packet->data[0x12],
							packet->data[0x13],
							packet->data[0x14],
							length);
					break;
				case 0x2a:
					state = WRITE;
					packet->transmit = false;
					//packet->data[0x16] = 0;
					//packet->data[0x17] = 0;
					tag[0] = packet->data[0x04];
					tag[1] = packet->data[0x05];
					tag[2] = packet->data[0x06];
					tag[3] = packet->data[0x07];
					fprintf(stderr, "CBW: Write, tag: %02x %02x %02x %02x\n", tag[0], tag[1], tag[2], tag[3]);
					blocks = (packet->data[0x16]<<8) | packet->data[0x17];
					fprintf(stderr, "CBW: Write LBA: 0x%02X%02X%02X%02X, %d blocks\n",
							packet->data[0x11],
							packet->data[0x12],
							packet->data[0x13],
							packet->data[0x14],
							blocks);
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
			fprintf(stderr, "WRITE:%d (EP%02x)\n", packet->wLength, packet->bEndpoint);
			packet->transmit = false;
			if(--blocks == 0)
				queue_packet();
			break;
		
		case READ:
			fprintf(stderr, "READ:%d\n", packet->wLength);
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
					fprintf(stderr, "CBW: Error(%d)\n", packet->data[0x0c]);
					break;
			}
				state = IDLE;
			break;
	}
}

void PacketFilter_MassStorage::start_injector() {
	fprintf(stderr,"Opening Queue Injector.\n");
	// TODO any queue setup required
}

int* PacketFilter_MassStorage::get_pollable_fds() {
	// TODO, create pollable fd that we prod whenever we have packets ready
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
	
	status_buf[4] = tag_buf[0];
	status_buf[5] = tag_buf[1];
	status_buf[6] = tag_buf[2];
	status_buf[7] = tag_buf[3];
	
	fprintf(stderr, "Injecting false OK status, tag: %02x %02x %02x %02x\n",
			status_buf[4], status_buf[5], status_buf[6], status_buf[7]);
	
	__u8* usbbuf=(__u8*)malloc(status_len);
	memcpy(usbbuf, status_buf, status_len);
	
	*packet = new Packet(0x82, usbbuf, status_len);

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
