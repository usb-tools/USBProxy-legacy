
from ctypes import c_ubyte

USB_DIR_IN  = 0x80
USB_DIR_OUT = 0x00

USB_REQ_GET_STATUS = 0x00
USB_REQ_CLEAR_FEATURE = 0x01
USB_REQ_SET_FEATURE = 0x03
USB_REQ_SET_ADDRESS = 0x05
USB_REQ_GET_DESCRIPTOR = 0x06
USB_REQ_SET_DESCRIPTOR = 0x07
USB_REQ_GET_CONFIGURATION = 0x08
USB_REQ_SET_CONFIGURATION = 0x09
USB_REQ_GET_INTERFACE = 0x0A
USB_REQ_SET_INTERFACE = 0x0B

USB_DT_DEVICE = 0x01
USB_DT_CONFIG = 0x02
USB_DT_STRING = 0x03
USB_DT_INTERFACE = 0x04
USB_DT_ENDPOINT = 0x05
USB_DT_DEVICE_QUALIFIER = 0x06
USB_DT_OTHER_SPEED_CONFIG = 0x07

USB_TYPE_VENDOR = 0x40

class DeviceProxy(object):
	def __init__(self, device_desc, config_desc, callback_strings=None):
		self.device_desc = device_desc
		self.config_desc = config_desc
		self.callback_strings = callback_strings

	def control_req(self, p_ctrl_req, p_nbytes, dataptr, timeout):
		setup_packet = p_ctrl_req[0]
		#dataptr = dataptr[0]
		if setup_packet.bRequestType & USB_DIR_IN and setup_packet.bRequest == USB_REQ_GET_DESCRIPTOR:
			value = setup_packet.wValue >> 8
			print("value: %02x" % value)
			if value == USB_DT_DEVICE:
				print("USB_DT_DEVICE")
				p_nbytes[0] = len(self.device_desc)
				for i in range(p_nbytes[0]):
					dataptr[i] = c_ubyte(self.device_desc[i])
				
			elif value == USB_DT_CONFIG:
				print("USB_DT_CONFIG")
				p_nbytes[0] = self.config_desc[2] | self.config_desc[3] << 8
				print(p_nbytes[0])
				if p_nbytes[0] > setup_packet.wLength:
					p_nbytes[0] = setup_packet.wLength
				for i in range(p_nbytes[0]):
					dataptr[i] = c_ubyte(self.config_desc[i])
				
			elif value == USB_DT_STRING:
				print("USB_DT_STRING")
				idx = setup_packet.wValue & 0xff
				print("idx: ", idx)
				if idx == 0:
					p_nbytes[0] = 4
					dataptr[0] = c_ubyte(0x04)
					dataptr[1] = c_ubyte(0x03)
					dataptr[2] = c_ubyte(0x09)
					dataptr[3] = c_ubyte(0x04)
					return 0
				elif idx>0 and setup_packet.wIndex!=0x0409:
					print("wIndex: %x" % setup_packet.wIndex)
					print(type(setup_packet.wIndex))
					return -1
				elif idx>=len(callback_strings):
					return -1
				self.string_desc=callback_strings[idx]
				print(self.string_desc)
				if len(self.string_desc)>setup_packet.wLength:
					p_nbytes[0] = setup_packet.wLength
				else:
					p_nbytes[0] = len(self.string_desc)
				for i in range(p_nbytes[0]):
					dataptr[i] = c_ubyte(ord(self.string_desc[i]))
	
			elif value == USB_DT_DEVICE_QUALIFIER:
				return -1
			
			elif value == USB_DT_OTHER_SPEED_CONFIG:
				return -1
			
			else:
				self.get_extended_descriptor(p_ctrl_req, p_nbytes, dataptr, timeout)
		
		elif  setup_packet.bRequestType & USB_DIR_IN and setup_packet.bRequest == USB_REQ_GET_CONFIGURATION:
			print("USB_REQ_GET_CONFIGURATION")
			p_nbytes[0] = 1
			dataptr[0] = c_ubyte(1)
		
		elif setup_packet.bRequest == USB_REQ_SET_CONFIGURATION:
			print("Setting config %d (As if that does anything)\n" % setup_packet.wValue)
		
		elif setup_packet.bRequest == USB_REQ_GET_INTERFACE:
			self.get_interface(p_ctrl_req, p_nbytes, dataptr, timeout)
		
		elif setup_packet.bRequestType == USB_TYPE_VENDOR:
			self.vendor_request(p_ctrl_req, p_nbytes, dataptr, timeout)
		else:
			print("Unhandled control request: 0x%02x, 0x%02x, %d, %d\n" % \
					(setup_packet.bRequestType, setup_packet.bRequest,
					setup_packet.wValue, setup_packet.wIndex))
		
		return 0
	
	def connect(self, timeout):
		return 0
	
	def disconnect(self, timeout):
		pass
	
	def send_data(self, endpoint, attributes, maxPacketSize, dataptr, length):
		print("send")
	
	def receive_data(self, endpoint, attributes, maxPacketSize, dataptr, p_length, timeout):
		print("receive")
	
	def get_interface(self, p_ctrl_req, p_nbytes, dataptr, timeout):
		setup_packet = p_ctrl_req[0]
		print("Unhandled get_interface request: 0x%02x, 0x%02x, %d, %d\n" % \
				(setup_packet.bRequestType, setup_packet.bRequest,
				setup_packet.wValue, setup_packet.wIndex))
	
	def vendor_request(self, p_ctrl_req, p_nbytes, dataptr, timeout):
		setup_packet = p_ctrl_req[0]
		print("Unhandled vendor request: 0x%02x, 0x%02x, %d, %d\n" % \
				(setup_packet.bRequestType, setup_packet.bRequest,
				setup_packet.wValue, setup_packet.wIndex))

	def get_extended_descriptor(p_ctrl_req, p_nbytes, dataptr, timeout):
		setup_packet = p_ctrl_req[0]
		print("Unhandled descriptor request: 0x%02x, 0x%02x, %d, %d\n" % \
				(setup_packet.bRequestType, setup_packet.bRequest,
				setup_packet.wValue, setup_packet.wIndex))

dev = None

def connect(*args):
	if dev:
		return dev.connect(*args)

def disconnect(*args):
	if dev:
		dev.disconnect(*args)

def control_req(*args):
	if dev:
		return dev.control_req(*args)

def send_data(*args):
	if dev:
		dev.send_data(*args)

def receive_data(*args):
	if dev:
		dev.receive_data(*args)

def init(device):
	global dev
	dev = device
