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
#include "USBString.h"

// Find the right place to pull this in from
#define cpu_to_le16(x) (x)

#define BUF_LEN 100

int DeviceProxy_Loopback::debugLevel = 1;

#define STRING_MANUFACTURER             1
#define STRING_PRODUCT                  2
#define STRING_SERIAL                   3
#define STRING_LOOPBACK              	4

static USBString** loopback_strings;
static int loopback_stringMaxIndex;

DeviceProxy_Loopback::DeviceProxy_Loopback(int vendorId,int productId) {
	p_is_connected = false;
	
	loopback_device_descriptor.bLength = USB_DT_DEVICE_SIZE;
	loopback_device_descriptor.bDescriptorType = USB_DT_DEVICE;
	loopback_device_descriptor.bcdUSB = cpu_to_le16(0x0100);
	loopback_device_descriptor.bDeviceClass = USB_CLASS_VENDOR_SPEC;
	loopback_device_descriptor.iManufacturer = cpu_to_le16(STRING_MANUFACTURER);
	loopback_device_descriptor.iProduct = cpu_to_le16(STRING_PRODUCT);
	loopback_device_descriptor.iSerialNumber = cpu_to_le16(STRING_SERIAL);
	loopback_device_descriptor.bNumConfigurations = 1;
	loopback_device_descriptor.idVendor = cpu_to_le16(vendorId & 0xffff);
	loopback_device_descriptor.idProduct = cpu_to_le16(productId & 0xffff);
	
	loopback_config_descriptor.bLength = USB_DT_CONFIG_SIZE;
	loopback_config_descriptor.bDescriptorType = USB_DT_CONFIG;
	loopback_config_descriptor.bNumInterfaces = 1;
	loopback_config_descriptor.bConfigurationValue = 2;
	loopback_config_descriptor.iConfiguration = STRING_LOOPBACK;
	loopback_config_descriptor.bmAttributes = USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER;
	loopback_config_descriptor.bMaxPower = 1;		/* self-powered */
	
	loopback_interface_descriptor.bLength = USB_DT_INTERFACE_SIZE;
	loopback_interface_descriptor.bDescriptorType = USB_DT_INTERFACE;
	loopback_interface_descriptor.bNumEndpoints = 2;
	loopback_interface_descriptor.bInterfaceClass = USB_CLASS_VENDOR_SPEC;
	loopback_interface_descriptor.iInterface = STRING_LOOPBACK;

	struct usb_endpoint_descriptor *ep;
	ep = &loopback_eps[0];
	ep->bLength = USB_DT_ENDPOINT_SIZE;
	ep->bDescriptorType = USB_DT_CS_ENDPOINT;
	ep->bEndpointAddress = USB_ENDPOINT_DIR_MASK | 3;
	ep->bmAttributes = USB_ENDPOINT_XFER_INT;
	ep->wMaxPacketSize = 64;
	ep->bInterval = 10;
	
	ep = &loopback_eps[1];
	ep->bLength = USB_DT_ENDPOINT_SIZE;
	ep->bDescriptorType = USB_DT_CS_ENDPOINT;
	ep->bEndpointAddress = 5;
	ep->bmAttributes = USB_ENDPOINT_XFER_INT;
	ep->wMaxPacketSize = 64;
	ep->bInterval = 10;

	loopback_config_descriptor.wTotalLength=loopback_config_descriptor.bLength+loopback_interface_descriptor.bLength+loopback_eps[0].bLength+loopback_eps[1].bLength;

	__u16 string0[2]={0x0409,0x0000};
	loopback_strings=(USBString**)calloc(5,sizeof(USBString*));
	loopback_strings[0]=new USBString(string0,0,0);
	loopback_strings[STRING_MANUFACTURER]=new USBString("Manufacturer",STRING_MANUFACTURER,0x409);
	loopback_strings[STRING_PRODUCT]=new USBString("Product",STRING_PRODUCT,0x409);
	loopback_strings[STRING_SERIAL]=new USBString("Serial",STRING_SERIAL,0x409);
	loopback_strings[STRING_LOOPBACK]=new USBString("Loopback",STRING_LOOPBACK,0x409);
	loopback_stringMaxIndex=STRING_LOOPBACK;
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
int DeviceProxy_Loopback::control_request(const usb_ctrlrequest* setup_packet, int* nbytes, __u8* dataptr, int timeout) {
	if (debugLevel>1) {
		char* hex=hex_string((void*)setup_packet,sizeof(*setup_packet));
		fprintf(stderr, "Loopback> %s\n",hex);
		free(hex);
	}
	if(setup_packet->bRequestType && USB_DIR_IN && setup_packet->bRequest == USB_REQ_GET_DESCRIPTOR) {
		__u8* buf;
		__u8* p;
		__u8 idx;
		const usb_string_descriptor* string_desc;

		switch ((setup_packet->wValue)>>8) {
			case USB_DT_DEVICE:
				//dataptr = (__u8 *) malloc(loopback_device_descriptor.bLength);
				memcpy(dataptr, &loopback_device_descriptor, loopback_device_descriptor.bLength);
				*nbytes = loopback_device_descriptor.bLength;
				break;
			case USB_DT_CONFIG:
				idx=setup_packet->wValue & 0xff;
				if (idx>=loopback_device_descriptor.bNumConfigurations) return -1;
				buf=(__u8*)malloc(loopback_config_descriptor.wTotalLength);
				p=buf;
				memcpy(p, &loopback_config_descriptor, loopback_config_descriptor.bLength);
				p+=loopback_config_descriptor.bLength;
				memcpy(p, &loopback_interface_descriptor, loopback_interface_descriptor.bLength);
				p+=loopback_interface_descriptor.bLength;
				memcpy(p, &loopback_eps[0], loopback_eps[0].bLength);
				p+=loopback_eps[0].bLength;
				memcpy(p, &loopback_eps[1], loopback_eps[1].bLength);
				*nbytes = loopback_config_descriptor.wTotalLength>setup_packet->wLength?setup_packet->wLength:loopback_config_descriptor.wTotalLength;
				memcpy(dataptr, buf, *nbytes);
				free(buf);
				break;
			case USB_DT_STRING:
				idx=setup_packet->wValue & 0xff;
				if (idx>0 && setup_packet->wIndex!=0x409) return -1;
				if (idx>loopback_stringMaxIndex) return -1;
				string_desc=loopback_strings[idx]->get_descriptor();
				*nbytes=string_desc->bLength>setup_packet->wLength?setup_packet->wLength:string_desc->bLength;
				memcpy(dataptr,buf,*nbytes);
				break;
			case USB_DT_DEVICE_QUALIFIER:
				return -1;
				break;
			case USB_DT_OTHER_SPEED_CONFIG:
				return -1;
				break;
		}
	}
	if (debugLevel>1) {
		char* hex=hex_string((void*)dataptr, *nbytes);
		fprintf(stderr, "Loopback> %s\n",hex);
		free(hex);
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
