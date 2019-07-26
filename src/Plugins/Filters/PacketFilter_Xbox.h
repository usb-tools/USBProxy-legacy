/*
 * This file is part of USBProxy.
 */

#ifndef PACKETFILTER_XBOX_H_
#define PACKETFILTER_XBOX_H_

#include "PacketFilter.h"

//writes all traffic to a stream
class PacketFilter_Xbox : public PacketFilter {
private:
	FILE* file;
	char buffer[41];
public:
	PacketFilter_Xbox(ConfigParser *cfg);
	void filter_packet(Packet* packet);
	void filter_setup_packet(SetupPacket* packet,bool direction);
	virtual char* toString() {return (char*)"Xbox Filter";}
};
#endif /* PACKETFILTER_XBOX_H_ */
