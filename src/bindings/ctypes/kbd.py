#!/usr/bin/env python

import usbproxy
import deviceproxy
import keymap
from ctypes import c_ubyte

'''
Device: 12 01 10 01 00 00 00 08 03 06 f2 00 12 01 01 02 00 01
  Manufacturer: NOVATEK
  Product:      USB Keyboard
	*Config(1): 09 02 3b 00 02 01 00 a0 32
		Interface(0):
			*Alt(0): 09 04 00 00 01 03 01 01 00
				HID: 09 21 10 01 00 01 22 41 00
				EP(81): 07 05 81 03 08 00 0a
		Interface(1):
			*Alt(0): 09 04 01 00 01 03 00 00 00
				HID: 09 21 10 01 00 01 22 7d 00
				EP(82): 07 05 82 03 08 00 0a
'''

device_desc = [
	18, 1, 0x10, 0x01, 0x00, 0x00, 0x00,
	0x08, 0x03, 0x06, 0xf2, 0x00, 0x12,
	0x01, 0x00, 0x00, 0x00, 1
]

config_desc = [
	9, 0x02, 0x3b, 0x00, 0x02, 0x01, 0x00, 0xa0, 0x32,
	
	0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x01, 0x01, 0x00,
	0x09, 0x21, 0x10, 0x01, 0x00, 0x01, 0x22, 0x41, 0x00,
	0x07, 0x05, 0x81, 0x03, 0x08, 0x00, 0x0a,
	
	0x09, 0x04, 0x01, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00,
	0x09, 0x21, 0x10, 0x01, 0x00, 0x01, 0x22, 0x7d, 0x00,
	0x07, 0x05, 0x82, 0x03, 0x08, 0x00, 0x0a
]

class Keyboard(deviceproxy.DeviceProxy):
	text = None
	ptr = 0
	key_presed = False
	
	def set_text(self, text):
		self.text = text
		self.ptr = 0
	
	def receive_data(self, endpoint, attributes, maxPacketSize, dataptr,
					 p_length, timeout):
		if endpoint == 0x82:
			return

		if self.ptr < len(self.text):
			pkt = [0x00] * 8
			if self.key_presed:
				self.key_presed = False
				self.ptr += 1
			else:
				self.key_presed = True
				key, mod = keymap.get_keycode(self.text[self.ptr])
				if key:
					pkt[2] = key
				if mod:
					pkt[0] = mod

			for i in range(len(pkt)):
				dataptr[i] = c_ubyte(pkt[i])
			p_length[0] = len(pkt)
	
	def vendor_request(self, p_ctrl_req, p_nbytes, dataptr, timeout):
		print "USB_REQ_GET_INTERFACE"
		setup_packet = p_ctrl_req[0]
		p_nbytes[0] = 1
		if setup_packet.bRequestType & USB_DIR_IN:
			dataptr[0] = c_ubyte(1)

if __name__ == '__main__':
	usbproxy.init()
	device = Keyboard(device_desc, config_desc)
	device.set_text("Shmoocon")
	deviceproxy.init(device)
	x = usbproxy.register_deviceproxy(
		connect=deviceproxy.connect,
		control_request=deviceproxy.control_req,
		send_data=deviceproxy.send_data,
		receive_data=deviceproxy.receive_data
		)
	
	usbproxy.lib.enable_logging()
	usbproxy.lib.print_config()
	usbproxy.lib.load_plugins()
	usbproxy.run()