/*
 * This file is part of USBProxy.
 */

#include "HexString.h"
#include "PacketFilter_StreamLog.h"

PacketFilter_StreamLog::PacketFilter_StreamLog(ConfigParser *cfg) {
	file  = (FILE *) cfg->get_pointer("PacketFilter_StreamLog::file");
}

void PacketFilter_StreamLog::filter_packet(Packet* packet) {
	if (packet->wLength<=64) {
		char* hex=hex_string((void*)packet->data,packet->wLength);
		fprintf(file,"%02x[%d]: %s\n",packet->bEndpoint,packet->wLength,hex);
		free(hex);
	}
}
void PacketFilter_StreamLog::filter_setup_packet(SetupPacket* packet,bool direction) {
	if (packet->ctrl_req.wLength && packet->data) {
		char* hex_setup=hex_string(&(packet->ctrl_req),sizeof(packet->ctrl_req));
		char* hex_data=hex_string((void*)(packet->data),packet->ctrl_req.wLength);
		fprintf(file,"[%s]: %s\n",hex_setup,hex_data);
		free(hex_data);
		free(hex_setup);
	} else {
		char* hex_setup=hex_string(&(packet->ctrl_req),sizeof(packet->ctrl_req));
		fprintf(file,"[%s]\n",hex_setup);
		free(hex_setup);
	}
}

static PacketFilter_StreamLog *proxy;

extern "C" {
	int plugin_type = PLUGIN_FILTER;
	
	PacketFilter * get_plugin(ConfigParser *cfg) {
		proxy = new PacketFilter_StreamLog(cfg);
		return (PacketFilter *) proxy;
	}
	
	void destroy_plugin() {
		delete proxy;
	}
}
