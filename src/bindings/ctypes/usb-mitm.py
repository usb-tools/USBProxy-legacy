#!/usr/bin/env python

import usbproxy

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

def control_req(p_ctrl_req, p_nbytes, p_dataptr, timeout):
	setup_packet = p_ctrl_req[0]
	if setup_packet.bRequestType & USB_DIR_IN and setup_packet.bRequest == USB_REQ_GET_DESCRIPTOR:
		print setup_packet.wValue >> 8
		p_nbytes = 9
	print "Woo woo!"
	return 0

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
	#usbproxy.register_packet_filter(rot13_filter)
	usbproxy.register_deviceproxy(control_request_func=control_req)
	usbproxy.lib.print_config()
	usbproxy.lib.load_plugins()
	usbproxy.run()
