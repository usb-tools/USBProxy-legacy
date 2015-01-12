#!/usr/bin/env python3

from USBDevice import USBDeviceRequest

import usbproxy
import deviceproxy

class USBProxyDevice(object):
	
	def __init__(self, app):
		self.ep_queues = {}
		self.app = app
		self.stall_flag = False
	
	def control_req(self, p_ctrl_req, p_nbytes, dataptr, timeout):
		# call handler
		setup_packet = p_ctrl_req[0]
		b = [
			setup_packet.bRequestType,
			setup_packet.bRequest,
			setup_packet.wValue & 0xff,
			setup_packet.wValue >> 8,
			setup_packet.wIndex & 0xff,
			setup_packet.wIndex >> 8,
			setup_packet.wLength & 0xff,
			setup_packet.wLength >> 8
		]
		req = USBDeviceRequest(b)
		self.app.connected_device.handle_request(req)
		# Check for stall
		if self.stall_flag:
			print("Stalling EP0")
			self.stall_flag = False
			return -1
		# Copy from queue to buffer
		data = self.read_data(0)
		if data:
			p_nbytes[0] = len(data)
			for i, b in enumerate(data):
				dataptr[i] = b
		else:
			p_nbytes[0] = 0
		return 0
	
	def receive_data(self, endpoint, attributes, maxPacketSize, dataptr,
					 p_length, timeout):
		# call handler
		ep_num = endpoint & 0x7f
		self.app.connected_device.handle_buffer_available(ep_num)
		# Copy from queue to buffer
		data = self.read_data(ep_num)
		if data:
			p_length[0] = len(data)
			for i, b in enumerate(data):
				dataptr[i] = b
		else:
			p_length[0] = 0
	
	def send_data(self, endpoint, attributes, maxPacketSize, dataptr, length):
		# Copy data to queue
		data = [dataptr[i] for i in range(length)]
		# call handler
		self.app.connected_device.handle_data_available(endpoint, data)
	
	def read_data(self, endpoint):
		if endpoint in self.ep_queues and len(self.ep_queues[endpoint]):
			print(len(self.ep_queues[endpoint]))
			return self.ep_queues[endpoint].pop(0)
	
	def write_data(self, endpoint, data):
		if endpoint not in self.ep_queues:
			self.ep_queues[endpoint] = []
		self.ep_queues[endpoint].append(data)
	
	def connect(self, timeout):
		return 0
	
	def disconnect(self):
		pass


class USBProxyApp(object):
	app_name = "USBProxy"
	
	def __init__(self, verbose=0):
		self.verbose = verbose
		usbproxy.init()
		self.usbproxy_dev = USBProxyDevice(self)
		deviceproxy.init(self.usbproxy_dev)
		self.x = usbproxy.register_deviceproxy(
			connect=deviceproxy.connect,
			disconnect=deviceproxy.disconnect,
			control_request=deviceproxy.control_req,
			send_data=deviceproxy.send_data,
			receive_data=deviceproxy.receive_data
			)
		
		usbproxy.lib.enable_logging()
		usbproxy.lib.print_config()
		usbproxy.lib.load_plugins()
	
	def connect(self, usb_device):
		print("connect()")
		self.connected_device = usb_device
	
	def disconnect(self):
		print("disconnect()")
		self.connected_device = None
	
	def service_irqs(self):
		usbproxy.run()

	def ack_status_stage(self):
		print("ack_status_stage()")

	def send_on_endpoint(self, endpoint, data):
		if data:
			self.usbproxy_dev.write_data(endpoint, data)

	def read_from_endpoint(self, endpoint):
		data = self.usbproxy_dev.read_data(endpoint)
		#if data:
		#	data = bytes(data)
		return data

	def stall_ep0(self):
		self.usbproxy_dev.stall_flag = True
