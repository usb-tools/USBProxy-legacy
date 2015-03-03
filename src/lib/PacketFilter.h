/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_PACKETFILTER_H
#define USBPROXY_PACKETFILTER_H

#include <stdlib.h>
#include <stdio.h>

#include "Plugins.h"
#include "Device.h"
#include "Configuration.h"
#include "Interface.h"
#include "Endpoint.h"
#include "Packet.h"
#include "HexString.h"
#include "Criteria.h"
#include "ConfigParser.h"

class PacketFilter {
protected:
	__u8 packetHeader[8];
	__u8 packetHeaderMask[8];
	__u8 packetHeaderMaskLength;
	bool packetHeaderSetupOut;
	bool packetHeaderSetupIn;

public:
	static const __u8 plugin_type=PLUGIN_FILTER;
	struct criteria_endpoint endpoint;
	struct criteria_interface interface;
	struct criteria_configuration configuration;
	struct criteria_device device;

	PacketFilter() {
		int i;
		for (i=0;i<8;i++) {packetHeader[i]=0;packetHeaderMask[i]=0;}
		packetHeaderMaskLength=0;
		packetHeaderSetupIn=true;
		packetHeaderSetupOut=true;
	}
	virtual ~PacketFilter() {};

	virtual void filter_packet(Packet* packet) {}
	virtual void filter_setup_packet(SetupPacket* packet,bool direction_out) {}

	bool test_packet(const Packet* packet);
	bool test_setup_packet(const SetupPacket* packet,bool direction_out);
	void set_packet_filter(__u8 header[4],__u8 mask[4]);
	virtual char* toString() {return (char*)"Filter";}
};

extern "C" {
	PacketFilter *get_packetfilter_plugin();
}
#endif /* USBPROXY_PACKETFILTER_H */
