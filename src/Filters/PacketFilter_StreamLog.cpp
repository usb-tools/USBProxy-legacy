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
 * PacketFilter_StreamLog.cpp
 *
 * Created on: Dec 28, 2013
 */
#include "PacketFilter_StreamLog.h"

	void PacketFilter_StreamLog::filter_packet(Packet* packet) {
		if (packet->wLength<=64) {
			char* hex=hex_string((void*)packet->data,packet->wLength);
			fprintf(file,"%02x[%d]: %s\n",packet->bEndpoint,packet->wLength,hex);
			free(hex);
		}
	}
	void PacketFilter_StreamLog::filter_setup_packet(SetupPacket* packet,bool direction) {
		if (packet->ctrl_req.wLength && packet->data) {
			char* hex_setup=hex_string(&(packet->ctrl_req),sizeof(packet->ctrl_req));
			char* hex_data=hex_string((void*)(packet->data),packet->ctrl_req.wLength);
			fprintf(file,"[%s]: %s\n",hex_setup,hex_data);
			free(hex_data);
			free(hex_setup);
		} else {
			char* hex_setup=hex_string(&(packet->ctrl_req),sizeof(packet->ctrl_req));
			fprintf(file,"[%s]\n",hex_setup);
			free(hex_setup);
		}
	}
