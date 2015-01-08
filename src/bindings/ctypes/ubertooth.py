#!/usr/bin/env python

import usbproxy
import deviceproxy

callback_strings = [None, 'ubertooth', 'bt_rxtx', '0001']

device_desc = [
	18, 1, 0x00, 0x02, 0xff, 0x00, 0x00,
	64, 0x50, 0x1d, 0x02, 0x60, 0x00,
	0x01, 0x00, 0x00, 0x00, 1
	#0x01, 0x01, 0x02, 0x03, 1
]

config_desc = [
	9, 2, 32, 0, 1, 1, 0, 0x80, 0x32,
	9, 4, 0, 0, 2, 0xff, 0, 0, 0,
	7, 5, 0x82, 2, 0x40, 0x00, 0,
	7, 5, 0x05, 2, 0x40, 0x00, 0
]

class Ubertooth(deviceproxy.DeviceProxy):
	pass

if __name__ == '__main__':
	usbproxy.init()
	device = Ubertooth(device_desc, config_desc)
	deviceproxy.init(device)
	x = usbproxy.register_deviceproxy(
		connect_func=deviceproxy.connect,
		control_request_func=deviceproxy.control_req
		)
	
	usbproxy.lib.print_config()
	usbproxy.lib.load_plugins()
	usbproxy.run()
