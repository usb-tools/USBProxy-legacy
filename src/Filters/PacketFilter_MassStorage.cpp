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
 * PacketFilter_StreamLog.cpp
 */
#include "PacketFilter_MassStorage.h"
#include "TRACE.h"
#include "get_tid.h"
#include "HaltSignal.h"

#define COMMAND 0
#define READ 1
#define WRITE 2
#define STATUS 3

	PacketFilter_MassStorage::PacketFilter_MassStorage() {
		state = COMMAND;
		flag = 0;
		halt = false;
	}
	PacketFilter_MassStorage::~PacketFilter_MassStorage() {
		if(sendQueue)
			mq_close(sendQueue);
	}

	void PacketFilter_MassStorage::start_queue() {
		if (haltsignal_setup(haltSignal,&haltpoll,&haltfd)!=0) return;
	
		char mqname[16];
		struct mq_attr mqa;
		mqa.mq_maxmsg=1;
		mqa.mq_msgsize=4;
		sprintf(mqname,"/USBProxy(UMS)-DGS");
		sendQueue=mq_open(mqname,O_RDWR | O_CREAT,S_IRWXU,&mqa);
		poll_out.fd=sendQueue;
		poll_out.events=POLLOUT;
	}

	void PacketFilter_MassStorage::send_packet() {
		__u8 buf[] = {0x55, 0x53, 0x42, 0x53,
							  0xFF, 0xFF, 0xFF, 0xFF,
							  0x00, 0x00, 0x00, 0x00, 0x00};
		buf[4] = tag[0];
		buf[5] = tag[1];
		buf[6] = tag[2];
		buf[7] = tag[3];
		
		Packet *p = new Packet(0x82, buf, 13);
		if (poll(&poll_out, 1, 500) && (poll_out.revents&POLLOUT)) {
			mq_send(sendQueue,(char*)&p,sizeof(Packet*),0);
			poll_out.revents=0;
		}
	}

	void PacketFilter_MassStorage::filter_packet(Packet* packet) {
		int length, i;
		switch(state) {
			case COMMAND:
				if ((packet->wLength == 31) &&
					(packet->data[0] == 0x55) &&
					(packet->data[1] == 0x53) &&
					(packet->data[2] == 0x42) &&
					(packet->data[3] == 0x43)) {
					// A CBW (Command Block Wrapper)
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
							packet->transmit = false;
							//packet->data[0xf] = 0x28;
							//flag = 1;
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
				}
				break;
			
			case WRITE:
				// First we'll recieve the write from the host
				// Then we'll need to trigger the read from the device (complicated and messy!)
				// Drop both packets and allow the status message to pass
				//packet->data is the block to be written
				packet->transmit = false;
				// Need to inject a status response here
				// "echo -e \\x82\\x00\\x00\\x0d\\x55\\x53\\x42\\x53\\x%02x\\x%02x\\x%02x\\x%02x\\x00\\x00\\x00\\x00\\x00 | nc -u 127.0.0.1 12345\0",
				// tag[0], tag[1], tag[2], tag[3]
				//state = STATUS;
				
				// Assume both write and read come through before status message
				fprintf(stderr, "WRITE:%d (EP%02x)\n", packet->wLength, packet->bEndpoint);
				// State is set to command, we'll fake the status message
				state = COMMAND;
				if(!sendQueue)
					start_queue();
				send_packet();
				//if(flag) {
				//	fprintf(stderr, "WRITE:%d (EP%02x)\n", packet->wLength, packet->bEndpoint);
				//	flag--;
				//} else {
				//	fprintf(stderr, "READ (WR):%d (EP%02x)\n", packet->wLength, packet->bEndpoint);
				//	state = STATUS;
				//}
				break;
			
			case READ:
				//packet->data is the block to be written
				fprintf(stderr, "READ:%d\n", packet->wLength);
				state = STATUS;
				break;
			
			case STATUS:
				//packet->data is the CSW from the device
				if ((packet->wLength == 13) &&
					(packet->data[0] == 0x55) &&
					(packet->data[1] == 0x53) &&
					(packet->data[2] == 0x42) &&
					(packet->data[3] == 0x53)) {
					// A CSW (Command Status Wrapper)
					switch(packet->data[12]) {
						case 0:
							fprintf(stderr, "CSW: Success\n");
							break;
						default:
							fprintf(stderr, "CBW: Error(%d)\n", packet->data[12]);
							if (flag) {
								fprintf(stderr, "Ignoring error\n");
								for(i=6; i<packet->wLength; i++)
									packet->data[i] = 0;
							}
							break;
					}
					state = COMMAND;
				} else {
					fprintf(stderr, "Not a CSW. len = %d\n", packet->wLength);
				}
				break;
		}
		if(flag)
			TRACE1(flag)
	}
