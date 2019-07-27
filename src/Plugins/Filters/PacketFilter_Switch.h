/*
 * This file is part of USBProxy.
 */

#ifndef PACKETFILTER_SWITCH_H_
#define PACKETFILTER_SWITCH_H_

#include "PacketFilter.h"

//writes all traffic to a stream
class PacketFilter_Switch : public PacketFilter {
private:
	FILE* file;
public:
	PacketFilter_Switch(ConfigParser *cfg);
	void filter_packet(Packet* packet);
	void filter_setup_packet(SetupPacket* packet,bool direction);
	virtual char* toString() {return (char*)"Nintendo Switch Filter";}
};
#endif /* PACKETFILTER_SWITCH_H_ */
