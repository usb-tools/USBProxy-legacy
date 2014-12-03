/*
 * This file is part of USBProxy.
 */

#ifndef PACKETFILTER_STREAMLOG_H_
#define PACKETFILTER_STREAMLOG_H_

#include "PacketFilter.h"

//writes all traffic to a stream
class PacketFilter_StreamLog : public PacketFilter {
private:
	FILE* file;
public:
	PacketFilter_StreamLog(ConfigParser *cfg);
	void filter_packet(Packet* packet);
	void filter_setup_packet(SetupPacket* packet,bool direction);
	virtual char* toString() {return (char*)"Stream Log Filter";}
};
#endif /* PACKETFILTER_STREAMLOG_H_ */
