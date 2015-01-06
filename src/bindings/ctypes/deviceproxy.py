
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

callback_strings = [None, 'ubertooth', 'bt_rxtx', '0001']

def control_req(p_ctrl_req, p_nbytes, p_dataptr, timeout):
	setup_packet = p_ctrl_req[0]
	#dataptr = p_dataptr[0]
	if setup_packet.bRequestType & USB_DIR_IN and setup_packet.bRequest == USB_REQ_GET_DESCRIPTOR:
		value = setup_packet.wValue >> 8
		print "value:", value
		if value == USB_DT_DEVICE:
			print "USB_DT_DEVICE"
			response = [
				18, 1, 0x02, 0x00, 0xff, 0x00, 0x00, 0x00,
				64, 0x1d, 0x50, 0x60, 0x02, 0x01,
				#0x01, 0x02, 0x03, 1]
				0x00, 0x00, 0x00, 1]
			for i in range(len(response)):
				p_dataptr[i] = c_ubyte(response[i])
			p_nbytes[0] = len(response)
		elif value == USB_DT_CONFIG:
			print "USB_DT_CONFIG"
			pass
		
		elif value == USB_DT_STRING:
			print "USB_DT_STRING"
			idx = setup_packet.wValue & 0xff
			print "idx: ", idx
			if idx == 0:
				p_nbytes[0] = 4
				p_dataptr[0] = c_ubyte(0x00)
				p_dataptr[1] = c_ubyte(0x00)
				p_dataptr[2] = c_ubyte(0x04)
				p_dataptr[3] = c_ubyte(0x09)
				return 0
			if idx>0 and setup_packet.wIndex!=0x409:
				print "wIndex: %x" % setup_packet.wIndex
				print type(setup_packet.wIndex)
				return -1
			if idx>=len(callback_strings):
				return -1
			string_desc=callback_strings[idx]
			print string_desc
			if len(string_desc)>setup_packet.wLength:
				p_nbytes[0] = setup_packet.wLength
			else:
				p_nbytes[0] = len(string_desc)
			for i in range(p_nbytes[0]):
				p_dataptr[i] = c_ubyte(ord(string_desc[i]))

		elif value == USB_DT_DEVICE_QUALIFIER:
			return -1
		
		elif value == USB_DT_OTHER_SPEED_CONFIG:
			return -1
	
	elif setup_packet.bRequest == USB_REQ_GET_CONFIGURATION:
		p_nbytes[0] = 1
		p_dataptr[0] = c_ubyte(1)
	
	elif setup_packet.bRequest == USB_REQ_SET_CONFIGURATION:
		print "Setting config %d (As if that does anything)\n" % setup_packet.wValue
	
	elif setup_packet.bRequest == USB_REQ_GET_INTERFACE:
		p_nbytes[0] = 1
		p_dataptr[0] = c_ubyte(1)
	
	else:
		print "Unhandled control request: 0x%02x, 0x%02x, %d, %d\n" % \
				(setup_packet.bRequestType, setup_packet.bRequest,
				setup_packet.wValue, setup_packet.wIndex)
	
	return 0