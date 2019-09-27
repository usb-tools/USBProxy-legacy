/*
 * This file is part of USBProxy.
 */

#include "PacketFilter_NeoGeo.h"

PacketFilter_NeoGeo::PacketFilter_NeoGeo(ConfigParser *cfg) {
	file  = (FILE *) cfg->get_pointer("PacketFilter_NeoGeo::file");
}

void PacketFilter_NeoGeo::filter_packet(Packet* packet) {
	if (packet->wLength == 21) {
		for (int i = 0; i < 8; ++i)
			fprintf(file, "%d", (packet->data[0] & (1 << i)) != 0);
		for (int i = 0; i < 8; ++i)
			fprintf(file, "%d", (packet->data[1] & (1 << i)) != 0);
		for (int i = 0; i < 8; ++i)
			fprintf(file, "%d", (packet->data[2] & (1 << i)) != 0);
		fprintf(file, "\n");
	}
}
void PacketFilter_NeoGeo::filter_setup_packet(SetupPacket* packet,bool direction) {
/*	if (packet->ctrl_req.wLength && packet->data) {
		char* hex_setup=hex_string(&(packet->ctrl_req),sizeof(packet->ctrl_req));
		char* hex_data=hex_string((void*)(packet->data),packet->ctrl_req.wLength);
		fprintf(file,"[%s]: %s\n",hex_setup,hex_data);
		free(hex_data);
		free(hex_setup);
	} else {
		char* hex_setup=hex_string(&(packet->ctrl_req),sizeof(packet->ctrl_req));
		fprintf(file,"[%s]\n",hex_setup);
		free(hex_setup);
	}*/
}

static PacketFilter_NeoGeo *proxy;

extern "C" {
	int plugin_type = PLUGIN_FILTER;
	
	PacketFilter * get_plugin(ConfigParser *cfg) {
		proxy = new PacketFilter_NeoGeo(cfg);
		return (PacketFilter *) proxy;
	}
	
	void destroy_plugin() {
		delete proxy;
	}
}
