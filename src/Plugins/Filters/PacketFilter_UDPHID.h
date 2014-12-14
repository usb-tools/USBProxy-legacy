/*
 * This file is part of USBProxy.
 */

#ifndef PACKETFILTER_UDPHID_H_
#define PACKETFILTER_UDPHID_H_

#include "PacketFilter.h"
#include "../Injectors/Injector_UDPHID.h"

//writes all traffic to a stream
class PacketFilter_UDPHID : public PacketFilter {
private:
	__u8* reportBuffer;
public:
	PacketFilter_UDPHID(ConfigParser *cfg);
	void filter_packet(Packet* packet);
	void filter_setup_packet(SetupPacket* packet,bool direction);
	virtual char* toString() {return (char*)"UDPHID Filter";}
};
#endif /* PACKETFILTER_UDPHID_H_ */
