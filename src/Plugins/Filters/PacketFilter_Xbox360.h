/*
 * This file is part of USBProxy.
 */

#ifndef PACKETFILTER_XBOX360_H_
#define PACKETFILTER_XBOX360_H_

#include "PacketFilter.h"

//writes all traffic to a stream
class PacketFilter_Xbox360 : public PacketFilter {
private:
	FILE* file;
public:
	PacketFilter_Xbox360(ConfigParser *cfg);
	void filter_packet(Packet* packet);
	void filter_setup_packet(SetupPacket* packet,bool direction);
	virtual char* toString() {return (char*)"Xbox 360 Filter";}
};
#endif /* PACKETFILTER_XBOX360_H_ */
