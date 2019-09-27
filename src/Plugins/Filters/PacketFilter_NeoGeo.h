/*
 * This file is part of USBProxy.
 */

#ifndef PACKETFILTER_NEOGEO_H_
#define PACKETFILTER_NEOGEO_H_

#include "PacketFilter.h"

//writes all traffic to a stream
class PacketFilter_NeoGeo : public PacketFilter {
private:
	FILE* file;
public:
	PacketFilter_NeoGeo(ConfigParser *cfg);
	void filter_packet(Packet* packet);
	void filter_setup_packet(SetupPacket* packet,bool direction);
	virtual char* toString() {return (char*)"Neo Geo Mini Filter";}
};
#endif /* PACKETFILTER_NEOGEO_H_ */
