/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
 * 
 * Based on libusb-gadget - Copyright 2009 Daiki Ueno <ueno@unixuser.org>
 *
 * This file is part of USBProxy.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
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
 */

#include "DeviceProxy_Loopback.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "Packet.h"
#include "HexString.h"
#include "TRACE.h"

// Find the right place to pull this in from
#define cpu_to_le16(x) (x)

#define BUF_LEN 100

int DeviceProxy_Loopback::debugLevel = 0;

#define STRING_MANUFACTURER             25
#define STRING_PRODUCT                  45
#define STRING_SERIAL                   101
#define STRING_LOOPBACK              250

static struct usb_string strings[] = {
  {STRING_MANUFACTURER, "Manufacturer",},
  {STRING_PRODUCT, "Product",},
  {STRING_SERIAL, "123456789",},
  {STRING_LOOPBACK, "Loopback",},
};

static struct usb_strings loopback_strings;

DeviceProxy_Loopback::DeviceProxy_Loopback(int vendorId,int productId) {
	p_is_connected = false;
	
	loopback_strings.language = 0x0409;  /* en-us */
	loopback_strings.strings = strings;
	
	loopback_device_descriptor.bLength = sizeof(loopback_device_descriptor);
	loopback_device_descriptor.bDescriptorType = USB_DT_DEVICE;
	loopback_device_descriptor.bcdUSB = cpu_to_le16(0x0100);
	loopback_device_descriptor.bDeviceClass = USB_CLASS_VENDOR_SPEC;
	loopback_device_descriptor.iManufacturer = cpu_to_le16(STRING_MANUFACTURER);
	loopback_device_descriptor.iProduct = cpu_to_le16(STRING_PRODUCT);
	loopback_device_descriptor.iSerialNumber = cpu_to_le16(STRING_SERIAL);
	loopback_device_descriptor.bNumConfigurations = 1;
	loopback_device_descriptor.idVendor = cpu_to_le16(vendorId & 0xffff);
	loopback_device_descriptor.idProduct = cpu_to_le16(productId & 0xffff);
	
	loopback_config_descriptor.bLength = sizeof(loopback_config_descriptor);
	loopback_config_descriptor.bDescriptorType = USB_DT_CONFIG;
	loopback_config_descriptor.bNumInterfaces = 1;
	loopback_config_descriptor.bConfigurationValue = 2;
	loopback_config_descriptor.iConfiguration = STRING_LOOPBACK;
	loopback_config_descriptor.bmAttributes = USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER;
	loopback_config_descriptor.bMaxPower = 1;		/* self-powered */
	
	loopback_interface_descriptor.bLength = sizeof(loopback_interface_descriptor);
	loopback_interface_descriptor.bDescriptorType = USB_DT_INTERFACE;
	loopback_interface_descriptor.bNumEndpoints = 2;
	loopback_interface_descriptor.bInterfaceClass = USB_CLASS_VENDOR_SPEC;
	loopback_interface_descriptor.iInterface = STRING_LOOPBACK;

}

DeviceProxy_Loopback::~DeviceProxy_Loopback() {
	disconnect();
}

int DeviceProxy_Loopback::connect() {
	buffer = (struct pkt *) malloc(BUF_LEN * sizeof(struct pkt));
	head = tail = 0;
	full = false;
	p_is_connected = true;
	return 0;
}

void DeviceProxy_Loopback::disconnect() {
	if(buffer) {
		free(buffer);
		buffer = NULL;
	}
	full = false;
	p_is_connected = false;
}

void DeviceProxy_Loopback::reset() {
	head = tail = 0;
	full = false;
}

bool DeviceProxy_Loopback::is_connected() {
	return p_is_connected;
}

bool DeviceProxy_Loopback::is_highspeed() {
	return false;
}

//return -1 to stall
int DeviceProxy_Loopback::control_request(const usb_ctrlrequest *setup_packet, int *nbytes, __u8* dataptr, int timeout) {
	if (debugLevel>1) {
		char* hex=hex_string((void*)setup_packet,sizeof(*setup_packet));
		fprintf(stderr, "Loopback> %s\n",hex);
		free(hex);
	}
	fprintf(stderr, "Loopback> %02x\n", setup_packet->bRequest);
	switch (setup_packet->bRequest) {
		case USB_REQ_GET_DESCRIPTOR:
			dataptr = (__u8 *) malloc(loopback_device_descriptor.bLength);
			memcpy(dataptr, &loopback_device_descriptor, loopback_device_descriptor.bLength);
			*nbytes = loopback_device_descriptor.bLength;
			break;
		case USB_REQ_GET_CONFIGURATION:
			dataptr = (__u8 *) malloc(loopback_config_descriptor.bLength);
			memcpy(dataptr, &loopback_config_descriptor, loopback_config_descriptor.bLength);
			*nbytes = loopback_config_descriptor.bLength;
			break;
		case USB_REQ_GET_INTERFACE:
			dataptr = (__u8 *) malloc(loopback_interface_descriptor.bLength);
			memcpy(dataptr, &loopback_interface_descriptor, loopback_interface_descriptor.bLength);
			*nbytes = loopback_interface_descriptor.bLength;
			break;
	}
	return 0;
}

void DeviceProxy_Loopback::send_data(__u8 endpoint,__u8 attributes, __u16 maxPacketSize, __u8* dataptr, int length) {
	if(head==tail && full)
		return; // Buffer is full, silently drop data

	struct pkt *next = buffer + tail;
	next->length = length;
	next->data = (__u8 *) malloc(length);
	memcpy(next->data, dataptr, length);
	tail = (tail + 1) % BUF_LEN;
	full = (head==tail);
}

void DeviceProxy_Loopback::receive_data(__u8 endpoint,__u8 attributes, __u16 maxPacketSize, __u8** dataptr, int* length, int timeout) {
	if(head==tail && !full) {
		// No packet data (could wait for timeout to see if data arrives)
		*length = 0;
	} else {
		struct pkt *next = buffer + head;
		*dataptr = next->data;
		*length = next->length;
		head = (head + 1) % BUF_LEN;
		full = !(head==tail);
	}
}

void DeviceProxy_Loopback::setConfig(Configuration* fs_cfg, Configuration* hs_cfg, bool hs) {
	;
}

void DeviceProxy_Loopback::claim_interface(__u8 interface) {
	;
}

void DeviceProxy_Loopback::release_interface(__u8 interface) {
	;
}

__u8 DeviceProxy_Loopback::get_address() {
	return 1;
}
