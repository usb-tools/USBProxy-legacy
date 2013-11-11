/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
 *
 * This file is part of USB-MitM.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 *
 * USBPacketFilter.cpp
 *
 * Created on: Nov 11, 2013
 */

#include <linux/usb/ch9.h>
#include "USBPacketFilter.h"

bool USBPacketFilter::test_device(USBDevice* _device) {
	const usb_device_descriptor* desc=_device->get_descriptor();
	if (device.deviceClass!=-1 && device.deviceClass!=desc->bDeviceClass) {return false;}
	if (device.subClass!=-1 && device.subClass!=desc->bDeviceSubClass) {return false;}
	if (device.protocol!=-1 && device.protocol!=desc->bDeviceProtocol) {return false;}
	if (device.ep0packetSizeMin>desc->bMaxPacketSize0 || device.ep0packetSizeMax<desc->bMaxPacketSize0) {return false;}
	if (device.vendor!=-1 && device.vendor!=desc->idVendor) {return false;}
	if (device.product!=-1 && device.product!=desc->idProduct) {return false;}
	if (device.release!=-1 && device.release!=desc->bcdDevice) {return false;}
	return true;
}
bool USBPacketFilter::test_configuration(USBConfiguration* _configuration) {
	const usb_config_descriptor* desc=_configuration->get_descriptor();
	if (configuration.number!=-1 && configuration.number!=desc->bConfigurationValue) {return false;}
	if (configuration.highSpeed!=-255 && (configuration.highSpeed?USB_DT_OTHER_SPEED_CONFIG:USB_DT_CONFIG)!=desc->bDescriptorType) {return false;}
	if (configuration.attributesMask&configuration.attributes!=configuration.attributesMask&desc->bmAttributes) {return false;}
	return true;
}

bool USBPacketFilter::test_interface(USBInterface* _interface) {
	const usb_interface_descriptor* desc=_interface->get_descriptor();
	if (interface.number!=-1 && interface.number!=desc->bInterfaceNumber) {return false;}
	if (interface.alternate!=-1 && interface.alternate!=desc->bAlternateSetting) {return false;}
	if (interface.deviceClass!=-1 && interface.deviceClass!=desc->bInterfaceClass) {return false;}
	if (interface.subClass!=-1 && interface.subClass!=desc->bInterfaceSubClass) {return false;}
	if (interface.protocol!=-1 && interface.protocol!=desc->bInterfaceProtocol) {return false;}
	return true;
}

bool USBPacketFilter::test_endpoint(USBEndpoint* _endpoint) {
	const usb_endpoint_descriptor* desc=_endpoint->get_descriptor();
	if (endpoint.addressMask&endpoint.address!=endpoint.addressMask&desc->bEndpointAddress) {return false;}
	if (endpoint.attributesMask&endpoint.attributes!=endpoint.attributesMask&desc->bmAttributes) {return false;}
	if (endpoint.packetSizeMin>desc->wMaxPacketSize || endpoint.packetSizeMax<desc->wMaxPacketSize) {return false;}
	if (endpoint.intervalMin>desc->bInterval || endpoint.intervalMax<desc->bInterval) {return false;}
	return true;
}

void USBPacketFilter::set_packet_filter(__u8 header[8],__u8 mask[8]) {
	int i;
	packetHeaderMaskLength=1;
	for(i=0;i<8;i++) {
		if (mask[i]) {packetHeaderMaskLength=i;}
	}
}

bool USBPacketFilter::test_packet(USBPacket* packet,usb_ctrlrequest *setup_packet) {
	__u8* data;
	if (((packet->bEndpoint)&0x7f) == 0) {
		data=(__u8 *)setup_packet;
	} else {
		if (packet->wLength<packetHeaderMaskLength) {return false;}
		data=packet->data;
	}
	int i;
	for(i=0;i<packetHeaderMaskLength;i++) {
		if (packetHeaderMask[i] && (packetHeaderMask[i]&data[i]!=packetHeaderMask[i]&packetHeader[i])) {return false;}
	}
	return true;
}



