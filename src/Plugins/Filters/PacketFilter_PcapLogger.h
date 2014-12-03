/*
 * This file is part of USBProxy.
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
	__u64 pkt_count;

public:
	PacketFilter_PcapLogger(ConfigParser *cfg);
	~PacketFilter_PcapLogger();
	void filter_setup_packet(SetupPacket* packet,bool direction_out);
	void filter_packet(Packet* packet);
	virtual char* toString() {return (char*)"Pcap Logging Filter";}
};

#endif /* USBPROXY_PACKETFILTER_KEYLOGGER_H */
