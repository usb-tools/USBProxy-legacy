# Copyright 2013 Dominic Spill
# Copyright 2013 Adam Stasiak
#
# This file is part of USBProxy.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#
# USBProxy.py
#
# Created on: Jan 11, 2014

class Packet(object):
	bEndpoint  = -1
	data       = []
	wLength    = -1
	filter_pkt = False
	transmit   = True
	
	def __init__(self, endpoint, data, length, filter_pkt):
		self.bEndpoint = endpoint
		self.data = data
		self.wLength = length
		self.filter_pkt = filter_pkt

class SetupPacket(object):
	bRequestType = 0
	bRequest     = 0
	wValue       = 0
	wIndex       = 0
	wLength      = 0
	source       = 0
	filter_out   = False
	transmit_out = True
	filter_in    = False
	transmit_in  = True
	data = []
	
	def __init__(self, bRequestType, bRequest, wValue, wIndex, wLength, source,
				 filter_out, transmit_out, filter_in, transmit_in, data):
		self.bRequestType = bRequestType
		self.bRequest     = bRequest
		self.wValue       = wValue
		self.wIndex       = wIndex
		self.wLength      = wLength
		self.source       = source
		self.filter_out   = filter_out
		self.transmit_out = transmit_out
		self.filter_in    = filter_in
		self.transmit_in  = transmit_in
		self.data         = data
