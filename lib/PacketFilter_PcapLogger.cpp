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
 * PacketFilter_PcapLoggger.cpp
 *
 * Created on: Dec 5, 2013
 */
#include "PacketFilter_PcapLogger.h"
#include <sys/time.h>

/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 1518


PacketFilter_PcapLogger::PacketFilter_PcapLogger(const char* filename) {
	file = fopen(filename, "w");
	pcap_file = pcap_open_dead(DLT_USB, SNAP_LEN);
	if (pcap_file == NULL)
		fprintf(stderr, "Unable to open pcap file for output\n");
	pcap_writer = pcap_dump_fopen(pcap_file, file);
	pcap_dump_flush(pcap_writer);

	if (pcap_writer == NULL) {
		fprintf(stderr, "Unable to open pcap dumper for output\n");
		pcap_close(pcap_file);
	}
	pthread_mutex_init(&pcap_writer_mutex, NULL);
}

PacketFilter_PcapLogger::~PacketFilter_PcapLogger() {
	pcap_dump_flush(pcap_writer);
	pcap_close(pcap_file);
	fclose(file);
}

void PacketFilter_PcapLogger::filter_setup_packet(SetupPacket* packet) {
	struct pcap_pkthdr ph;
	struct timeval ts;
	__u8 *buf, *ptr;

	fprintf(stderr, "Packet length: %d\n", __le16_to_cpu(packet->ctrl_req.wLength));
	fprintf(stderr, "Allocating %d bytes \n", sizeof(struct usb_ctrlrequest) + packet->ctrl_req.wLength);
	buf = (__u8 *) malloc(sizeof(struct usb_ctrlrequest) + packet->ctrl_req.wLength);
	if(buf == NULL) {
		fprintf(stderr, "PcapLogger:Unable to allocate packet buffer\n");
		return;
	}
	ptr = buf;
	
	gettimeofday(&ts, NULL);
	ph.ts = ts;
	ph.caplen = ph.len = __cpu_to_le16(packet->ctrl_req.wLength);
	
	fprintf(stderr, "Copying %d bytes (usb_ctrlrequest) to %p\n", sizeof(struct usb_ctrlrequest), ptr);
	memcpy(ptr, (__u8 *) &(packet->ctrl_req), sizeof(struct usb_ctrlrequest));
	ptr += sizeof(struct usb_ctrlrequest);

	fprintf(stderr, "Copying %d bytes (data) to %p\n", packet->ctrl_req.wLength, ptr);
	memcpy(ptr, packet->data, packet->ctrl_req.wLength);
	TRACE
	
	pthread_mutex_lock(&pcap_writer_mutex);
	pcap_dump((unsigned char *)pcap_writer, &ph, buf);
	pthread_mutex_unlock(&pcap_writer_mutex);
}

void PacketFilter_PcapLogger::filter_packet(Packet* packet) {
	struct pcap_pkthdr ph;
	struct timeval ts;

	TRACE
	gettimeofday(&ts, NULL);
	ph.ts = ts;
	ph.caplen = ph.len = __cpu_to_le16(packet->wLength);
	
	TRACE
	pthread_mutex_lock(&pcap_writer_mutex);
	TRACE
	pcap_dump((unsigned char *)pcap_writer, &ph, packet->data);
	TRACE
	pthread_mutex_unlock(&pcap_writer_mutex);
	TRACE
}
