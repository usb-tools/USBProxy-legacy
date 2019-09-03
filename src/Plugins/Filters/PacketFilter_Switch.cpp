/*
 * This file is part of USBProxy.
 */

#include "PacketFilter_Switch.h"

PacketFilter_Switch::PacketFilter_Switch(ConfigParser *cfg) {
	file  = (FILE *) cfg->get_pointer("PacketFilter_Switch::file");
}

void PacketFilter_Switch::filter_packet(Packet* packet) {
	if (packet->wLength == 64) {
		for (int i = 0; i < 8; ++i)
			fprintf(file, "%d", (packet->data[3] & (1 << i)) != 0);
		for (int i = 0; i < 8; ++i)
			fprintf(file, "%d", (packet->data[4] & (1 << i)) != 0);
		for (int i = 0; i < 8; ++i)
			fprintf(file, "%d", (packet->data[5] & (1 << i)) != 0);

		__u8* analog = &packet->data[6];
		uint8_t lx = (((analog[1] & 0x0F) << 4) | ((analog[0] & 0xF0) >> 4)) + 127;
		uint8_t ly = analog[2] + 127;
		uint8_t rx = (((analog[4] & 0x0F) << 4) | ((analog[3] & 0xF0) >> 4)) + 127;
		uint8_t ry = analog[5] + 127;

		for (int i = 0; i < 8; ++i)
			fprintf(file, "%d", (lx & (1 << i)) != 0);
		for (int i = 0; i < 8; ++i)
			fprintf(file, "%d", (ly & (1 << i)) != 0);
		for (int i = 0; i < 8; ++i)
			fprintf(file, "%d", (rx & (1 << i)) != 0);
		for (int i = 0; i < 8; ++i)
			fprintf(file, "%d", (ry & (1 << i)) != 0);

		fprintf(file, "\n");
	}
	if (packet->wLength == 8) {
		for (int i = 0; i < 8; ++i)
			fprintf(file, "%d", (packet->data[0] & (1 << i)) != 0);
		for (int i = 0; i < 8; ++i)
			fprintf(file, "%d", (packet->data[1] & (1 << i)) != 0);
		for (int i = 0; i < 8; ++i)
			fprintf(file, "%d", (packet->data[2] & (1 << i)) != 0);
		for (int i = 0; i < 8; ++i)
			fprintf(file, "%d", (packet->data[3] & (1 << i)) != 0);
		for (int i = 0; i < 8; ++i)
			fprintf(file, "%d", (packet->data[4] & (1 << i)) != 0);
		for (int i = 0; i < 8; ++i)
			fprintf(file, "%d", (packet->data[5] & (1 << i)) != 0);
		for (int i = 0; i < 8; ++i)
			fprintf(file, "%d", (packet->data[6] & (1 << i)) != 0);

		fprintf(file, "0\n");
	}
}
void PacketFilter_Switch::filter_setup_packet(SetupPacket* packet,bool direction) {
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

static PacketFilter_Switch *proxy;

extern "C" {
	int plugin_type = PLUGIN_FILTER;
	
	PacketFilter * get_plugin(ConfigParser *cfg) {
		proxy = new PacketFilter_Switch(cfg);
		return (PacketFilter *) proxy;
	}
	
	void destroy_plugin() {
		delete proxy;
	}
}
