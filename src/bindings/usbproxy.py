
import time
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
	filter_pkt_func = FILTER_PKT_FUNC(func)
	lib.register_packet_filter(filter_pkt_func)

def run():
	lib.start()
	while(lib.get_status() == lib.USBM_RELAYING):
		time.sleep(10)
	lib.shutdown()
