
import time
from ctypes import *
lib = cdll.LoadLibrary("libUSBProxyAPI.so")

def set_config():
	# Put in some dummy config so that it works for now
	lib.set_config("HostProxy", "HostProxy_GadgetFS");
	#lib.set_config("DeviceProxy", "DeviceProxy_LibUSB");
	#lib.set_config("HostProxy", "HostProxy_TCP");
	#lib.set_config("HostProxy_TCP::TCPAddress", "127.0.0.1");

def init():
	lib.usbproxy_init()
	set_config()

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

USBM_IDLE = 0
USBM_SETUP = 1
USBM_RELAYING = 2
USBM_STOPPING = 3
USBM_SETUP_ABORT = 4

def run():
	lib.start()
	while(lib.get_status() == USBM_RELAYING):
		time.sleep(10)
	lib.shutdown()

class USB_CtrlRequest(LittleEndianStructure):
	_fields_ = [("bRequestType", c_uint8),
	            ("bRequest", c_uint8),
	            ("wValue", c_uint16),
	            ("wIndex", c_uint16),
	            ("wLength", c_uint16)]

DEVICE_VOID_VOID_FUNC = CFUNCTYPE(None)
DEVICE_CONNECT_FUNC = CFUNCTYPE(c_int, c_int)
DEVICE_CONTROL_FUNC = CFUNCTYPE(c_int, POINTER(USB_CtrlRequest), POINTER(c_int), POINTER(c_uint8), c_int)
DEVICE_SEND_FUNC = CFUNCTYPE(None, c_uint8, c_uint8, c_uint16, POINTER(c_uint8), c_int)
DEVICE_RECEIVE_FUNC = CFUNCTYPE(None, c_uint8, c_uint8, c_uint16, POINTER(c_uint8), POINTER(c_int), c_int)

def register_deviceproxy(connect=None, disconnect=None,
						 reset=None, control_request=None,
						 send_data=None, receive_data=None,
						 toString=None):
	if connect:
		connect_cb = DEVICE_CONNECT_FUNC(connect)
	else:
		connect_cb = None
	if disconnect:
		disconnect_cb = DEVICE_VOID_VOID_FUNC(disconnect)
	else:
		disconnect_cb = None
	if reset:
		reset_cb = DEVICE_VOID_VOID_FUNC(reset)
	else:
		reset_cb = None
	if control_request:
		control_request_cb = DEVICE_CONTROL_FUNC(control_request)
	else:
		control_request_cb = None
	if send_data:
		send_data_cb = DEVICE_SEND_FUNC(send_data)
	else:
		send_data_cb = None
	if receive_data:
		receive_data_cb = DEVICE_RECEIVE_FUNC(receive_data)
	else:
		receive_data_cb = None
	if toString:
		toString_cb = DEVICE_VOID_VOID_FUNC(toString)
	else:
		toString_cb = None
	
	lib.register_deviceproxy(
		connect_cb,
		disconnect_cb,
		reset_cb,
		control_request_cb,
		send_data_cb,
		receive_data_cb,
		toString_cb)

	return (connect_cb, disconnect_cb, reset_cb,
			control_request_cb, send_data_cb, receive_data_cb,
			toString_cb)
