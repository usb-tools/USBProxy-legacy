/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_PACKETFILTER_KEYLOGGER_H_
#define USBPROXY_PACKETFILTER_KEYLOGGER_H_

#include <stdio.h>
#include "PacketFilter.h"

class PacketFilter_KeyLogger : public PacketFilter {
private:
	FILE* file;
	char lastReport[8];
	void keyPressed(__u8 keyCode,__u8 mods);
	const char* keyMap[0x66];
	const char* shiftKeyMap[0x66];
public:
	PacketFilter_KeyLogger(ConfigParser *cfg);
	void filter_packet(Packet* packet);
	virtual char* toString() {return (char*)"Key Logger Filter";}
};

#endif /* USBPROXY_PACKETFILTER_KEYLOGGER_H_ */
