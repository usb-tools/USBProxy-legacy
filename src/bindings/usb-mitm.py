#!/usr/bin/env python

import usbproxy

def filter_func(p_pkt):
	pkt = p_pkt[0]
	print "Packet:\n\tlength:%d\n\tEP:%02X" % (pkt.wLength, pkt.bEndpoint)
	x = ["%02x" % pkt.data[i] for i in range(pkt.wLength)]
	print x

if __name__ == '__main__':
	usbproxy.init()
	usbproxy.register_packet_filter(filter_func)
	device_desc = usbproxy.USB_Device_Descriptor(
		18, 1, 0x0200, 0xff, 0, 0, 64, 0x1d50, 0x6002, 0x0100, 1, 2, 3, 1
	)
	config_desc = usbproxy.USB_Config_Descriptor(
		9, 2, 0x0020, 1, 1, 0, 0x80
	)
	interface_desc = usbproxy.USB_Interface_Descriptor(
		9, 4, 0, 0, 2, 0xff, 0, 0, 0
	)
	ep_desc = usbproxy.USB_Endpoint_Descriptor(
		7, 5, 0x82, 2, 0x0040, 0
	)

	usbproxy.register_deviceproxy(device_desc, config_desc, interface_desc, ep_desc)
	usbproxy.lib.print_config()
	usbproxy.lib.load_plugins()
	usbproxy.run()
