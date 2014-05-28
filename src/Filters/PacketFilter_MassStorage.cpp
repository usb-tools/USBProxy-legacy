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

#define IDLE 0
#define COMMAND 1
#define READ 2
#define WRITE 3
#define STATUS 4
#define UNKNOWN 5

PacketFilter_MassStorage::PacketFilter_MassStorage() {
	state = IDLE;
}

void PacketFilter_MassStorage::queue_packet() {
	__u8 buf[] = {0x55, 0x53, 0x42, 0x53,
						  0xFF, 0xFF, 0xFF, 0xFF,
						  0x00, 0x00, 0x00, 0x00, 0x00};
	buf[4] = tag[0];
	buf[5] = tag[1];
	buf[6] = tag[2];
	buf[7] = tag[3];
	
	Packet *p = new Packet(0x82, buf, 13);
}

void PacketFilter_MassStorage::filter_packet(Packet* packet) {
	int length, i, type = UNKNOWN;
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
			switch(packet->data[0xf]) {
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
					//packet->transmit = false;
					packet->data[0x16] = 0;
					packet->data[0x17] = 0;
					tag[0] = packet->data[4];
					tag[1] = packet->data[5];
					tag[2] = packet->data[6];
					tag[3] = packet->data[7];
					fprintf(stderr, "CBW: Write, tag: %02X %02X %02X %02X\n", tag[0], tag[1], tag[2], tag[3]);
					length = (packet->data[0x16]<<8) | packet->data[0x17];
					fprintf(stderr, "CBW: Write LBA: 0x%02X%02X%02X%02X, %d blocks\n",
							packet->data[0x11],
							packet->data[0x12],
							packet->data[0x13],
							packet->data[0x14],
							length);
					break;
			}
			break;
		
		case WRITE:
			fprintf(stderr, "WRITE:%d (EP%02x)\n", packet->wLength, packet->bEndpoint);
			packet->wLength = 0;
			//free(packet->data);
			//packet->transmit = false;
			// Need to inject a status response here
			//queue_packet();
			break;
		
		case READ:
			fprintf(stderr, "READ:%d\n", packet->wLength);
			break;
		
		case STATUS:
			// A CSW (Command Status Wrapper)
			switch(packet->data[12]) {
				case 0:
					fprintf(stderr, "CSW: Success\n");
					break;
				default:
					fprintf(stderr, "CBW: Error(%d)\n", packet->data[12]);
					//if (flag) {
					//	fprintf(stderr, "Ignoring error\n");
					//	for(i=6; i<packet->wLength; i++)
					//		packet->data[i] = 0;
					//}
					break;
			}
				state = IDLE;
			break;
	}
}
