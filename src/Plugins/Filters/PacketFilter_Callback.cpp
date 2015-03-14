/*
 * This file is part of USBProxy.
 */

#include "PacketFilter_Callback.h"

PacketFilter_Callback::PacketFilter_Callback(ConfigParser *cfg) {
	cb = (f_cb) cfg->get_pointer("PacketFilter_Callback::filter_packet");
	cb_setup = (f_cb_setup) cfg->get_pointer("PacketFilter_Callback::filter_setup_packet");
}

PacketFilter_Callback::PacketFilter_Callback(f_cb _cb, f_cb_setup _cb_setup) {
	cb = _cb;
	cb_setup = _cb_setup;
}

void PacketFilter_Callback::filter_packet(Packet* packet) {
	if(cb)
		cb(packet);
}

void PacketFilter_Callback::filter_setup_packet(SetupPacket* packet,bool direction_out) {
	if (cb_setup)
		cb_setup(packet,direction_out);
}

static PacketFilter_Callback *proxy;

extern "C" {
	int plugin_type = PLUGIN_FILTER;
	
	PacketFilter * get_plugin(ConfigParser *cfg) {
		proxy = new PacketFilter_Callback(cfg);
		return (PacketFilter *) proxy;
	}
	
	void destroy_plugin() {
		delete proxy;
	}
}
