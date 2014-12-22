/*
 * This file is part of USBProxy.
 */

#include "Manager.h"
#include "ConfigParser.h"
#include "PacketFilter_Callback.h"
#include "DeviceProxy_Callback.h"

#define API extern "C"

API void usbproxy_init();

API void set_config(char* key, char* value);

API void register_deviceproxy(
		struct usb_device_descriptor callback_device_descriptor,
		struct usb_config_descriptor callback_config_descriptor,
		struct usb_interface_descriptor callback_interface_descriptor,
		struct usb_endpoint_descriptor callback_eps,
		f_connect connect_cb,
		f_disconnect disconnect_cb,
		f_reset reset_cb,
		f_control_request control_request_cb,
		f_send_data send_data_cb,
		f_receive_data receive_data_cb,
		f_toString toString_cb
		);

API void register_packet_filter(f_cb cb);

API void print_config();
API void load_plugins();

API void start();
API int get_status();
API void shutdown();
