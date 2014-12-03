/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_PACKETFILTER_POWER_H
#define USBPROXY_PACKETFILTER_POWER_H

#include <stdio.h>
#include "PacketFilter.h"

class PacketFilter_Power : public PacketFilter {

public:
	PacketFilter_Power(ConfigParser *cfg);
	void filter_setup_packet(SetupPacket* packet, bool direction_in);
	virtual char* toString() {return (char*)"Force Self Powered Filter";}
};

#endif /* USBPROXY_PACKETFILTER_POWER_H */
