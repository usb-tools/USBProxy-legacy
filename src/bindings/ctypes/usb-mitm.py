#!/usr/bin/env python

import usbproxy
import deviceproxy

def log_filter(p_pkt):
	pkt = p_pkt[0]
	print "Packet:\n\tlength:%d\n\tEP:%02X" % (pkt.wLength, pkt.bEndpoint)
	x = ["%02x" % pkt.data[i] for i in range(pkt.wLength)]
	print x

def rot13_filter(p_pkt):
	pkt = p_pkt[0]
	for i in range(2, 8):
		if pkt.data[i] <= 0x1d and pkt.data[i] >= 0x04:
			if pkt.data[i] <= 0x10:
				pkt.data[i] = pkt.data[i]+13
			else:
				pkt.data[i] = pkt.data[i]-13

if __name__ == '__main__':
	usbproxy.init()
	usbproxy.register_packet_filter(rot13_filter)
	x = usbproxy.register_deviceproxy(
		connect_func=deviceproxy.connect_f,
		control_request_func=deviceproxy.control_req
		)
	
	usbproxy.lib.print_config()
	usbproxy.lib.load_plugins()
	usbproxy.run()
