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
#include <stdlib.h>
#include <memory.h>
#include <sys/time.h>
#include <pcap/usb.h>

#include "TRACE.h"
#include "PacketFilter_PcapLogger.h"

/* maximum bytes per packet to capture - 64 byte packet + 48 byte header */
#define SNAP_LEN 112


PacketFilter_PcapLogger::PacketFilter_PcapLogger(const char* filename) {
	pkt_count = 0;
	file = fopen(filename, "w");
	pcap_file = pcap_open_dead(DLT_USB_LINUX, SNAP_LEN);
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

void PacketFilter_PcapLogger::filter_setup_packet(SetupPacket* packet,bool direction_out) {
	struct pcap_pkthdr ph;
	struct timeval ts;
	__u8 *buf, *ptr;
	pcap_usb_header *usb_pkthdr;
	__u32 data_len, buf_len;
	
	if(packet->data != NULL)
		data_len = packet->ctrl_req.wLength;
	else
		data_len = 0;
	
	buf_len = sizeof(pcap_usb_header) + data_len;
	buf = (__u8 *) malloc(buf_len);
	if(buf == NULL) {
		fprintf(stderr, "PcapLogger: Unable to allocate packet buffer\n");
		return;
	}
	ptr = buf;
	
	gettimeofday(&ts, NULL);
	ph.ts = ts;
	ph.caplen = ph.len = buf_len;

	usb_pkthdr = (pcap_usb_header *) buf;

	
	/* Hardcoding these for now, could retrieve them in future if we're handling
	 * multiple devices, but for now we're only ever going to have a single device
	 */
	usb_pkthdr->device_address = 1;
	usb_pkthdr->bus_id = 1;
	usb_pkthdr->transfer_type = 2; /* This is almost certainly wrong on many packets */
	
	/*if !=0 the urb setup header is not present*/
	usb_pkthdr->setup_flag = 0;
	/*if !=0 no urb data is present*/
	if (data_len) {
		usb_pkthdr->data_flag = 0;
		/* FIXME Need to set transfer mode correctly (see pcap/usb.h) */
		usb_pkthdr->event_type = URB_COMPLETE;
	} else {
		usb_pkthdr->data_flag = 1;
		/* FIXME Need to set transfer mode correctly (see pcap/usb.h) */
		usb_pkthdr->event_type = URB_SUBMIT;
	}
	
	usb_pkthdr->endpoint_number = 0;
	usb_pkthdr->ts_sec = ts.tv_sec;
	usb_pkthdr->ts_usec = ts.tv_usec;
	usb_pkthdr->status = 0; /* I believe 0 means success */
	usb_pkthdr->urb_len = 8;
	usb_pkthdr->data_len = data_len;
	
	/* Copy SetupPacket's ctrl_req values */
	usb_pkthdr->setup.bmRequestType = packet->ctrl_req.bRequestType;
	usb_pkthdr->setup.bRequest = packet->ctrl_req.bRequest;
	usb_pkthdr->setup.wValue = packet->ctrl_req.wValue;
	usb_pkthdr->setup.wIndex = packet->ctrl_req.wIndex;
	usb_pkthdr->setup.wLength = packet->ctrl_req.wLength;
	
	ptr += sizeof(pcap_usb_header);
	memcpy(ptr, packet->data, data_len);
	
	pthread_mutex_lock(&pcap_writer_mutex);
	usb_pkthdr->id = ++pkt_count;
	pcap_dump((unsigned char *)pcap_writer, &ph, buf);
	pcap_dump_flush(pcap_writer);
	pthread_mutex_unlock(&pcap_writer_mutex);
}

void PacketFilter_PcapLogger::filter_packet(Packet* packet) {
	struct pcap_pkthdr ph;
	struct timeval ts;
	__u8 *buf, *ptr;
	pcap_usb_header *usb_pkthdr;
	__u32 data_len, buf_len;
	
	if(packet->data != NULL)
		data_len = packet->wLength;
	else
		data_len = 0;
	
	buf_len = sizeof(pcap_usb_header) + data_len;
	buf = (__u8 *) malloc(buf_len);
	if(buf == NULL) {
		fprintf(stderr, "PcapLogger: Unable to allocate packet buffer\n");
		return;
	}
	ptr = buf;
	
	gettimeofday(&ts, NULL);
	ph.ts = ts;
	ph.caplen = ph.len = buf_len;

	usb_pkthdr = (pcap_usb_header *) buf;
	
	/* Hardcoding these for now, could retrieve them in future if we're handling
	 * multiple devices, but for now we're only ever going to have a single device
	 */
	usb_pkthdr->device_address = 1;
	usb_pkthdr->bus_id = 1;
	usb_pkthdr->transfer_type = 3; /* This is almost certainly wrong on many packets */
	
	/*if !=0 the urb setup header is not present*/
	usb_pkthdr->setup_flag = 1;
	/*if !=0 no urb data is present*/
	if (data_len) {
		usb_pkthdr->data_flag = 0;
		/* FIXME Need to set transfer mode correctly (see pcap/usb.h) */
		usb_pkthdr->event_type = URB_COMPLETE;
	} else {
		usb_pkthdr->data_flag = 1;
		/* FIXME Need to set transfer mode correctly (see pcap/usb.h) */
		usb_pkthdr->event_type = URB_SUBMIT;
	}
	
	usb_pkthdr->endpoint_number = packet->bEndpoint;
	usb_pkthdr->ts_sec = ts.tv_sec;
	usb_pkthdr->ts_usec = ts.tv_usec;
	usb_pkthdr->status = 0; /* I believe 0 means success */
	usb_pkthdr->urb_len = 0;
	usb_pkthdr->data_len = data_len;
	
	ptr += sizeof(pcap_usb_header);
	memcpy(ptr, packet->data, data_len);
	
	pthread_mutex_lock(&pcap_writer_mutex);
	usb_pkthdr->id = ++pkt_count;
	pcap_dump((unsigned char *)pcap_writer, &ph, buf);
	pcap_dump_flush(pcap_writer);
	pthread_mutex_unlock(&pcap_writer_mutex);
}
