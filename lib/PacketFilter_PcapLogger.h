/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
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
 * PacketFilter_PcapLogger.h
 *
 * Created on: Dec 5, 2013
 */
#ifndef USBPROXY_PACKETFILTER_KEYLOGGER_H
#define USBPROXY_PACKETFILTER_KEYLOGGER_H

#include "PacketFilter.h"
#include <pcap.h>
#include <stdio.h>
#include "pthread.h"

class PacketFilter_PcapLogger : public PacketFilter {
private:
	FILE *file;
	pcap_t *pcap_file;
	pcap_dumper_t *pcap_writer;
	pthread_mutex_t pcap_writer_mutex;

public:
	PacketFilter_PcapLogger(const char* filename);
	~PacketFilter_PcapLogger();
	void filter_setup_packet(SetupPacket* packet);
	void filter_packet(Packet* packet);
	virtual char* toString() {return (char*)"Pcap Logging Filter";}
};

#endif /* USBPROXY_PACKETFILTER_KEYLOGGER_H */
