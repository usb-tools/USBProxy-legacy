/*
 * This file is part of USBProxy.
 */

#include "Manager.h"
#include "ConfigParser.h"
#include "PacketFilter_Callback.h"

#define API extern "C"
    
Manager *manager;
ConfigParser *cfg;

API void usbproxy_init() {
	cfg = new ConfigParser();
    manager = new Manager();
}

API void set_config() {
	cfg->set("DeviceProxy", "DeviceProxy_LibUSB");
	cfg->set("HostProxy", "HostProxy_GadgetFS");
}

API void register_packet_filter(f_cb cb) {
	cfg->add_to_vector("Plugins", "PacketFilter_Callback");
	cfg->add_pointer("PacketFilter_Callback::filter_packet", (void *)cb);
	cfg->add_pointer("PacketFilter_Callback::filter_setup_packet", NULL);
}

API void print_config() {
	cfg->print_config();
}

API void load_plugins() {
	manager->load_plugins(cfg);
}

API void start() {
	manager->start_control_relaying();
}

API int get_status() {
	return manager->get_status();
}

API void shutdown() {
    manager->stop_relaying();
	manager->cleanup();
	delete(manager);
}
