/*
 * This file is part of USBProxy.
 */

#include "PacketFilter_Power.h"
#include <linux/usb/ch9.h>

PacketFilter_Power::PacketFilter_Power(ConfigParser *cfg) {
}

void PacketFilter_Power::filter_setup_packet(SetupPacket* packet, bool direction_in) {
	struct usb_config_descriptor *config;

	if (direction_in
		&& (packet->ctrl_req.bRequest == USB_REQ_GET_DESCRIPTOR)
		&& (packet->ctrl_req.wValue == USB_DT_CONFIG)) {
		config = (struct usb_config_descriptor *) packet->data;
		config->bmAttributes = USB_CONFIG_ATT_ONE;
		config->bMaxPower = 0x32; /* 100mA */
	}
}

static PacketFilter_Power *proxy;

extern "C" {
	int plugin_type = PLUGIN_FILTER;
	
	PacketFilter * get_plugin(ConfigParser *cfg) {
		proxy = new PacketFilter_Power(cfg);
		return (PacketFilter *) proxy;
	}
	
	void destroy_plugin() {
		delete proxy;
	}
}
