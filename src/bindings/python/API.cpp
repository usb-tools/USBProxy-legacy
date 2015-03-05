/*
 * This file is part of USBProxy.
 */

#include "API.h"
#include <stdio.h>

Manager *manager;
ConfigParser *cfg;

API void usbproxy_init() {
	cfg = new ConfigParser();
    manager = new Manager(0);
}

API void set_config(char *key, char *value) {
	cfg->set(key,value);
}

API void enable_logging() {
	cfg->add_to_vector("Plugins", "PacketFilter_StreamLog");
	cfg->add_pointer("PacketFilter_StreamLog::file", stderr);
}

API void register_deviceproxy(
		f_connect connect_cb,
		f_disconnect disconnect_cb,
		f_reset reset_cb,
		f_control_request control_request_cb,
		f_send_data send_data_cb,
		f_receive_data receive_data_cb,
		f_toString toString_cb
		) {
	cfg->set("DeviceProxy", "DeviceProxy_Callback");
	cfg->add_pointer("DeviceProxy_Callback::connect", (void *)connect_cb);
	cfg->add_pointer("DeviceProxy_Callback::disconnect", (void *)disconnect_cb);
	cfg->add_pointer("DeviceProxy_Callback::reset", (void *)reset_cb);
	cfg->add_pointer("DeviceProxy_Callback::control_request", (void *)control_request_cb);
	cfg->add_pointer("DeviceProxy_Callback::send_data", (void *)send_data_cb);
	cfg->add_pointer("DeviceProxy_Callback::receive_data", (void *)receive_data_cb);
	cfg->add_pointer("DeviceProxy_Callback::toString", (void *)toString_cb);
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
