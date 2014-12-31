
import time
from ctypes import *
lib = cdll.LoadLibrary("libUSBProxyAPI.so")

def set_config():
	# Put in some dummy config so that it works for now
	#lib.set_config("HostProxy", "HostProxy_GadgetFS");
	lib.set_config("DeviceProxy", "DeviceProxy_LibUSB");
	lib.set_config("HostProxy", "HostProxy_TCP");
	lib.set_config("HostProxy_TCP::TCPAddress", "127.0.0.1");

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

class USB_Device_Descriptor(LittleEndianStructure):
	_fields_ = [("bLength", c_uint8),
	            ("bDescriptorType", c_uint8),
	            ("bcdUSB", c_uint16),
	            ("bDeviceClass", c_uint8),
	            ("bDeviceSubClass", c_uint8),
	            ("bDeviceProtocol", c_uint8),
	            ("bMaxPacketSize0", c_uint8),
	            ("idVendor", c_uint16),
	            ("idProduct", c_uint16),
	            ("bcdDevice", c_uint16),
	            ("iManufacturer", c_uint8),
	            ("iProduct", c_uint8),
	            ("iSerialNumber", c_uint8),
	            ("bNumConfigurations", c_uint8)]

class USB_Config_Descriptor(LittleEndianStructure):
	_fields_ = [("bLength", c_uint8),
	            ("bDescriptorType", c_uint8),
	            ("wTotalLength", c_uint16),
	            ("bNumInterfaces", c_uint8),
	            ("bConfigurationValue", c_uint8),
	            ("iConfiguration", c_uint8),
	            ("bmAttributes", c_uint8),
	            ("bMaxPower", c_uint8)]

class USB_Interface_Descriptor(LittleEndianStructure):
	_fields_ = [("bLength", c_uint8),
	            ("bDescriptorType", c_uint8),
	            ("bInterfaceNumber", c_uint8),
	            ("bAlternateSetting", c_uint8),
	            ("bNumEndpoints", c_uint8),
	            ("bInterfaceClass", c_uint8),
	            ("bInterfaceSubClass", c_uint8),
	            ("bInterfaceProtocol", c_uint8),
	            ("iInterface", c_uint8)]

class USB_Endpoint_Descriptor(LittleEndianStructure):
	_fields_ = [("bLength", c_uint8),
	            ("bDescriptorType", c_uint8),
	            ("bEndpointAddress", c_uint8),
	            ("bmAttributes", c_uint8),
	            ("wMaxPacketSize", c_uint16),
	            ("bInterval", c_uint8)]

class USB_String_Descriptor(LittleEndianStructure):
	_fields_ = [("bLength", c_uint8),
	            ("bDescriptorType", c_uint8),
	            ("wData", c_uint16)]

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
DEVICE_RECEIVE_FUNC = CFUNCTYPE(None, c_uint8, c_uint8, c_uint16, POINTER(POINTER(c_uint8)), POINTER(c_int), c_int)

def register_deviceproxy(
	device_descriptor, config_descriptor, interface_descriptor, callback_eps,
	string_descriptor, connect_func=None, disconnect_func=None, reset_func=None,
	control_request_func=None, send_data_func=None, receive_data_func=None,
	toString_func=None):
	
	if control_request_func:
		control_request_func_cb = DEVICE_CONTROL_FUNC(control_request_func)
	else:
		control_request_func_cb = None
	if send_data_func:
		send_data_func_cb = DEVICE_SEND_FUNC(send_data_func)
	else:
		send_data_func_cb = None
	if receive_data_func:
		receive_data_func_cb = DEVICE_RECEIVE_FUNC(receive_data_func)
	else:
		receive_data_func_cb = None
	if connect_func:
		connect_func_cb = DEVICE_CONNECT_FUNC(connect_func)
	else:
		connect_func_cb = None
	if disconnect_func:
		disconnect_func_cb = DEVICE_VOID_VOID_FUNC(disconnect_func)
	else:
		disconnect_func_cb = None
	if reset_func:
		reset_func_cb = DEVICE_VOID_VOID_FUNC(reset_func)
	else:
		reset_func_cb = None
	if toString_func:
		toString_func_cb = DEVICE_VOID_VOID_FUNC(toString_func)
	else:
		toString_func_cb = None
	
	lib.register_deviceproxy(
		device_descriptor,
		config_descriptor,
		interface_descriptor,
		callback_eps,
		control_request_func_cb,
		send_data_func_cb,
		receive_data_func_cb,
		connect_func_cb,
		disconnect_func_cb,
		reset_func_cb,
		toString_func_cb)
