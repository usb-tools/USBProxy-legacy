/*
 * This file is part of USBProxy.
 */

#include "PacketFilter_Xbox360.h"

PacketFilter_Xbox360::PacketFilter_Xbox360(ConfigParser *cfg) {
	file  = (FILE *) cfg->get_pointer("PacketFilter_Xbox360::file");
}

void PacketFilter_Xbox360::filter_packet(Packet* packet) {
	if (packet->wLength == 20) {
		
		for (int i = 0; i < 12; ++i)
			for(int j = 0; j < 8; ++j)
				fprintf(file, "%d", (packet->data[2+i] & (1 << j)) != 0);

		fprintf(file, "\n");
	}
}
void PacketFilter_Xbox360::filter_setup_packet(SetupPacket* packet,bool direction) {
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

static PacketFilter_Xbox360 *proxy;

extern "C" {
	int plugin_type = PLUGIN_FILTER;
	
	PacketFilter * get_plugin(ConfigParser *cfg) {
		proxy = new PacketFilter_Xbox360(cfg);
		return (PacketFilter *) proxy;
	}
	
	void destroy_plugin() {
		delete proxy;
	}
}
