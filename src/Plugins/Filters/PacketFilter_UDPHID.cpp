/*
 * This file is part of USBProxy.
 */

#include "PacketFilter_UDPHID.h"

PacketFilter_UDPHID::PacketFilter_UDPHID(ConfigParser *cfg) {
	this->interface.deviceClass=0xff;
	this->interface.subClass=0x5d;
	this->endpoint.address=0x80;
	this->endpoint.addressMask=0x80;
	this->packetHeaderMaskLength=2;
	this->packetHeader[0]=0;
	this->packetHeader[1]=20;
	this->packetHeaderMask[0]=0xff;
	this->packetHeaderMask[1]=0xff;
	this->packetHeaderSetupIn=false;
	this->packetHeaderSetupOut=false;
	Injector_UDPHID* injector = (Injector_UDPHID *) cfg->get_pointer("PacketFilter_UDPHID::injector");
	reportBuffer=injector->getReportBuffer();
}

void PacketFilter_UDPHID::filter_packet(Packet* packet) {
	int i;
	if (packet->wLength!=20) return;
	packet->data[2]=packet->data[2]|reportBuffer[2];
	packet->data[3]=packet->data[3]|reportBuffer[3];
	for (i=4;i<=13;i++) {
		if (reportBuffer[i]) packet->data[i]=reportBuffer[i];
	}
}
void PacketFilter_UDPHID::filter_setup_packet(SetupPacket* packet,bool direction) {
}

static PacketFilter_UDPHID *proxy;

extern "C" {
	int plugin_type = PLUGIN_FILTER;
	
	PacketFilter * get_plugin(ConfigParser *cfg) {
		proxy = new PacketFilter_UDPHID(cfg);
		return (PacketFilter *) proxy;
	}
	
	void destroy_plugin() {
		delete proxy;
	}
}
