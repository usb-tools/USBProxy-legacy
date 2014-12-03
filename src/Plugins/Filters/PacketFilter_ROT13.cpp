/*
 * This file is part of USBProxy.
 */

#include "PacketFilter_ROT13.h"

void PacketFilter_ROT13::filter_packet(Packet* packet) {
	int i;
	for (i=2;i<8;i++) {
		if (packet->data[i]<=0x1d && packet->data[i]>=0x04) {
			if(packet->data[i]<=0x10)
				packet->data[i]=packet->data[i]+13;
			else
				packet->data[i]=packet->data[i]-13;
		}
	}
}

static PacketFilter_ROT13 *proxy;

extern "C" {
	int plugin_type = PLUGIN_FILTER;
	
	PacketFilter * get_plugin(ConfigParser *cfg) {
		proxy = new PacketFilter_ROT13(cfg);
		return (PacketFilter *) proxy;
	}
	
	void destroy_plugin() {
		delete proxy;
	}
}
