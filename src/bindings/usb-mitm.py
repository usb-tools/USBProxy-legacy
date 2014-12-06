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
	usbproxy.lib.print_config()
	usbproxy.lib.load_plugins()
	usbproxy.run()
