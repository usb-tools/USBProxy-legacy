/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_PACKETFILTER_ROT13_H_
#define USBPROXY_PACKETFILTER_ROT13_H_

#include "PacketFilter.h"

class PacketFilter_ROT13: public PacketFilter {
public:
	PacketFilter_ROT13(ConfigParser *cfg) {}
	virtual ~PacketFilter_ROT13() {}
	void filter_packet(Packet* packet);
	virtual char* toString() {return (char*)"ROT13 Filter";}
};

#endif /* USBPROXY_PACKETFILTER_ROT13_H_ */
