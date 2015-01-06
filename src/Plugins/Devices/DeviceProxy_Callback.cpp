/*
 * This file is part of USBProxy.
 */

#include "DeviceProxy_Callback.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "Packet.h"
#include "HexString.h"
#include "TRACE.h"
#include "USBString.h"

// Find the right place to pull this in from
#define cpu_to_le16(x) (x)

#define BUF_LEN 100

int DeviceProxy_Callback::debugLevel = 2;

DeviceProxy_Callback::DeviceProxy_Callback(ConfigParser *cfg) {
	connect_cb = (f_connect) cfg->get_pointer("DeviceProxy_Callback::connect");
	disconnect_cb = (f_disconnect) cfg->get_pointer("DeviceProxy_Callback::disconnect");
	reset_cb = (f_reset) cfg->get_pointer("DeviceProxy_Callback::reset");
	control_request_cb = (f_control_request) cfg->get_pointer("DeviceProxy_Callback::control_request");
	send_data_cb = (f_send_data) cfg->get_pointer("DeviceProxy_Callback::send_data");
	receive_data_cb = (f_receive_data) cfg->get_pointer("DeviceProxy_Callback::receive_data");
	toString_cb = (f_toString) cfg->get_pointer("DeviceProxy_Callback::toString");
	p_is_connected = false;
}

DeviceProxy_Callback::~DeviceProxy_Callback() {
	disconnect();
}

int DeviceProxy_Callback::connect(int timeout) {
	if(connect_cb)
		return connect_cb(timeout);
	return -1;
}

void DeviceProxy_Callback::disconnect() {
	if(disconnect_cb)
		disconnect_cb();
}

void DeviceProxy_Callback::reset() {
	if(reset_cb)
		reset_cb();
}

bool DeviceProxy_Callback::is_connected() {
	return p_is_connected;
}

bool DeviceProxy_Callback::is_highspeed() {
	return false;
}

int DeviceProxy_Callback::control_request(const usb_ctrlrequest* setup_packet, int* nbytes, __u8* dataptr, int timeout) {
	int rv, i;
	fprintf(stdout, "control_request_cb: %d bytes\n", *nbytes);
	if(control_request_cb)
		rv = control_request_cb(setup_packet, nbytes, dataptr, timeout);
	fprintf(stdout, "control_request_cb: %d, %d bytes\n", rv, *nbytes);
	for(i=0; i<*nbytes; i++)
		fprintf(stdout, "%02x ", dataptr[i]);
	fprintf(stdout, "\n\n");
	return rv;
}

void DeviceProxy_Callback::send_data(__u8 endpoint,__u8 attributes, __u16 maxPacketSize, __u8* dataptr, int length) {
	if(send_data_cb)
		send_data_cb(endpoint, attributes, maxPacketSize, dataptr, length);
}

void DeviceProxy_Callback::receive_data(__u8 endpoint,__u8 attributes, __u16 maxPacketSize, __u8** dataptr, int* length, int timeout) {
	if(receive_data_cb)
		receive_data_cb(endpoint, attributes, maxPacketSize, dataptr, length, timeout);

}

void DeviceProxy_Callback::setConfig(Configuration* fs_cfg, Configuration* hs_cfg, bool hs) {
	;
}

void DeviceProxy_Callback::claim_interface(__u8 interface) {
	;
}

void DeviceProxy_Callback::release_interface(__u8 interface) {
	;
}

__u8 DeviceProxy_Callback::get_address() {
	return 1;
}

char* DeviceProxy_Callback::toString() {
	if(toString_cb)
		return toString_cb();
	else
		return (char *) "Callback device";
}

static DeviceProxy_Callback *proxy;

extern "C" {
	DeviceProxy * get_deviceproxy_plugin(ConfigParser *cfg) {
		proxy = new DeviceProxy_Callback(cfg);
		return (DeviceProxy *) proxy;
	}
	
	void destroy_plugin() {
		delete proxy;
	}
}
