#!/usr/bin/env python

from ctypes import *

lib = cdll.LoadLibrary("libUSBProxyAPI.so")

def init():
	lib.usbproxy_init()
	# Put in some dummy config so that it works for now
	lib.set_config()

class PACKET(Structure):
	_fields_ = [("bEndpoint", c_uint8),
	            ("wLength", c_uint16),
	            ("filter", c_bool),
	            ("transmit", c_bool),
	            ("data", POINTER(c_uint8))]

FILTER_PKT_FUNC = CFUNCTYPE(None, POINTER(PACKET))

def register_packet_filter(func):
	filter_pkt_func = FILTER_PKT_FUNC(filter_func)
	lib.register_packet_filter(filter_pkt_func)

#class SETUP_PACKET(Structure):
#	_fields_ = [("bEndpoint", c_uint8),
#	            ("wLength", c_uint16),
#	            ("filter", c_bool),
#	            ("transmit", c_bool),
#	            ("data", POINTER(c_uint8))]

#FILTER_SETUP_PKT_FUNC = CFUNCTYPE(None, POINTER(SETUP_PACKET), POINTER(c_bool))

def filter_func(PACKET):
	print "In filter function"

if __name__ == '__main__':
	init()
	register_packet_filter(filter_func)
	lib.print_config()
	lib.load_plugins()
	lib.start()
	import time
	while(1):
		time.sleep(15)
	lib.usbproxy_shutdown()

