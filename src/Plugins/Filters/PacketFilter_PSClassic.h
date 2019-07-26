/*
 * This file is part of USBProxy.
 */

#ifndef PACKETFILTER_PSCLASSIC_H_
#define PACKETFILTER_PSCLASSIC_H_

#include "PacketFilter.h"

//writes all traffic to a stream
class PacketFilter_PSClassic : public PacketFilter {
private:
	FILE* file;
public:
	PacketFilter_PSClassic(ConfigParser *cfg);
	void filter_packet(Packet* packet);
	void filter_setup_packet(SetupPacket* packet,bool direction);
	virtual char* toString() {return (char*)"PlayStation Classic Filter";}
};
#endif /* PACKETFILTER_PSCLASSIC_H_ */
