/*
 * Copyright 2014 Dominic Spill
 * Copyright 2014 Mike Kershaw / dragorn
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

#include "DeviceProxy_dot11.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include <lorcon2/lorcon.h>
#include "HexString.h"
#include "Packet.h"
#include "TRACE.h"
#include "USBString.h"

// Find the right place to pull this in from
#define cpu_to_le16(x) (x)

int DeviceProxy_dot11::debugLevel = 1;

#define STRING_MANUFACTURER 1
#define STRING_PRODUCT      2
#define STRING_SERIAL       3
#define STRING_DOT11	    4

static USBString** dot11_strings;
static int dot11_stringMaxIndex;

DeviceProxy_dot11::DeviceProxy_dot11(ConfigParser *cfg) {
	/* FIXME pull these values from the config object */
	int vendorId = 0xffff;
	int productId = 0x0005;

	p_is_connected = false;
	
	dot11_device_descriptor.bLength = USB_DT_DEVICE_SIZE;
	dot11_device_descriptor.bDescriptorType = USB_DT_DEVICE;
	dot11_device_descriptor.bcdUSB = cpu_to_le16(0x0100);
	dot11_device_descriptor.bDeviceClass = USB_CLASS_HID;
	dot11_device_descriptor.bDeviceSubClass = 0;
	dot11_device_descriptor.bDeviceProtocol = 0;
	dot11_device_descriptor.bMaxPacketSize0=64;
	dot11_device_descriptor.idVendor = cpu_to_le16(vendorId & 0xffff);
	dot11_device_descriptor.idProduct = cpu_to_le16(productId & 0xffff);
	fprintf(stderr,"V: %04x P: %04x\n",dot11_device_descriptor.idVendor,dot11_device_descriptor.idProduct);
	dot11_device_descriptor.bcdDevice = 0;
	dot11_device_descriptor.iManufacturer = STRING_MANUFACTURER;
	dot11_device_descriptor.iProduct = STRING_PRODUCT;
	dot11_device_descriptor.iSerialNumber = STRING_SERIAL;
	dot11_device_descriptor.bNumConfigurations = 1;
	
	dot11_config_descriptor.bLength = USB_DT_CONFIG_SIZE;
	dot11_config_descriptor.bDescriptorType = USB_DT_CONFIG;
	dot11_config_descriptor.bNumInterfaces = 1;
	dot11_config_descriptor.bConfigurationValue = 1;
	dot11_config_descriptor.iConfiguration = STRING_DOT11;
	dot11_config_descriptor.bmAttributes = USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER;
	dot11_config_descriptor.bMaxPower = 1;		/* self-powered */
	
	dot11_interface_descriptor.bLength = USB_DT_INTERFACE_SIZE;
	dot11_interface_descriptor.bDescriptorType = USB_DT_INTERFACE;
	dot11_interface_descriptor.bInterfaceNumber=0;
	dot11_interface_descriptor.bAlternateSetting=0;
	dot11_interface_descriptor.bNumEndpoints = 2;
	dot11_interface_descriptor.bInterfaceClass = USB_CLASS_HID;
	dot11_interface_descriptor.bInterfaceSubClass=0;
	dot11_interface_descriptor.bInterfaceProtocol=0;
	dot11_interface_descriptor.iInterface = STRING_DOT11;

	struct usb_endpoint_descriptor *ep;
	ep = &dot11_eps[0];
	ep->bLength = USB_DT_ENDPOINT_SIZE;
	ep->bDescriptorType = USB_DT_ENDPOINT;
	ep->bEndpointAddress = USB_ENDPOINT_DIR_MASK | 1;
	ep->bmAttributes = USB_ENDPOINT_XFER_INT;
	ep->wMaxPacketSize = 64;
	ep->bInterval = 10;
	
	ep = &dot11_eps[1];
	ep->bLength = USB_DT_ENDPOINT_SIZE;
	ep->bDescriptorType = USB_DT_ENDPOINT;
	ep->bEndpointAddress = 1;
	ep->bmAttributes = USB_ENDPOINT_XFER_INT;
	ep->wMaxPacketSize = 64;
	ep->bInterval = 10;

	dot11_config_descriptor.wTotalLength=dot11_config_descriptor.bLength+dot11_interface_descriptor.bLength+dot11_eps[0].bLength+dot11_eps[1].bLength;

	__u16 string0[2]={0x0409,0x0000};
	dot11_strings=(USBString**)calloc(5,sizeof(USBString*));

	dot11_strings[0]=new USBString(string0,0,0);

	dot11_strings[STRING_MANUFACTURER]=new USBString("Manufacturer",STRING_MANUFACTURER,0x409);
	dot11_strings[STRING_PRODUCT]=new USBString("Product",STRING_PRODUCT,0x409);
	dot11_strings[STRING_SERIAL]=new USBString("Serial",STRING_SERIAL,0x409);
	dot11_strings[STRING_DOT11]=new USBString("802.11",STRING_DOT11,0x409);
	dot11_stringMaxIndex=STRING_DOT11;

}

DeviceProxy_dot11::~DeviceProxy_dot11() {
	disconnect();

	int i;
	if (dot11_strings) {
	for (i=0;i<dot11_stringMaxIndex;i++) {
		if (dot11_strings[i]) {
			delete(dot11_strings[i]);
			dot11_strings[i]=NULL;
		}
	}
	free(dot11_strings);
	dot11_strings=NULL;
	}
}

int DeviceProxy_dot11::connect(int timeout) {
	p_is_connected = true;
	return 0;
}

void DeviceProxy_dot11::disconnect() {
	p_is_connected = false;
}

void DeviceProxy_dot11::reset() {
}

bool DeviceProxy_dot11::is_connected() {
	return p_is_connected;
}

bool DeviceProxy_dot11::is_highspeed() {
	return false;
}

//return -1 to stall
int DeviceProxy_dot11::control_request(const usb_ctrlrequest* setup_packet, int* nbytes, __u8* dataptr, int timeout) {
	if (debugLevel>1) {
		char* hex=hex_string((void*)setup_packet,sizeof(*setup_packet));
		fprintf(stderr, "802.11< %s\n",hex);
		free(hex);
	}
	if((setup_packet->bRequestType & USB_DIR_IN) && setup_packet->bRequest == USB_REQ_GET_DESCRIPTOR) {
		__u8* buf;
		__u8* p;
		__u8 idx;
		const usb_string_descriptor* string_desc;

		switch ((setup_packet->wValue)>>8) {
			case USB_DT_DEVICE:
				memcpy(dataptr, &dot11_device_descriptor, dot11_device_descriptor.bLength);
				*nbytes = dot11_device_descriptor.bLength;
				break;
			case USB_DT_CONFIG:
				idx=setup_packet->wValue & 0xff;
				if (idx>=dot11_device_descriptor.bNumConfigurations) return -1;
				buf=(__u8*)malloc(dot11_config_descriptor.wTotalLength);
				p=buf;
				memcpy(p, &dot11_config_descriptor, dot11_config_descriptor.bLength);
				p+=dot11_config_descriptor.bLength;
				memcpy(p, &dot11_interface_descriptor, dot11_interface_descriptor.bLength);
				p+=dot11_interface_descriptor.bLength;
				memcpy(p, &dot11_eps[0], dot11_eps[0].bLength);
				p+=dot11_eps[0].bLength;
				memcpy(p, &dot11_eps[1], dot11_eps[1].bLength);
				*nbytes = dot11_config_descriptor.wTotalLength>setup_packet->wLength?setup_packet->wLength:dot11_config_descriptor.wTotalLength;
				memcpy(dataptr, buf, *nbytes);
				free(buf);
				break;
			case USB_DT_STRING:
				idx=setup_packet->wValue & 0xff;
				if (idx>0 && setup_packet->wIndex!=0x409) return -1;
				if (idx>dot11_stringMaxIndex) return -1;
				string_desc=dot11_strings[idx]->get_descriptor();
				*nbytes=string_desc->bLength>setup_packet->wLength?setup_packet->wLength:string_desc->bLength;
				memcpy(dataptr,string_desc,*nbytes);
				break;
			case USB_DT_DEVICE_QUALIFIER:
				return -1;
				break;
			case USB_DT_OTHER_SPEED_CONFIG:
				return -1;
				break;
		}
	} else if ((setup_packet->bRequestType & USB_DIR_IN) && setup_packet->bRequest == USB_REQ_GET_CONFIGURATION){
		dataptr[0]=1;
		*nbytes=1;
	} else if ((setup_packet->bRequestType & USB_DIR_IN) && setup_packet->bRequest == USB_REQ_GET_INTERFACE){
		dataptr[0]=1;
		*nbytes=1;
	} else {
		fprintf(stderr,"Unhandled control request\n");
	}
	if (debugLevel>1 && nbytes) {
		char* hex=hex_string(dataptr, *nbytes);
		fprintf(stderr, "802.11> %s\n",hex);
		free(hex);
	}
	return 0;
}

void DeviceProxy_dot11::send_data(__u8 endpoint,__u8 attributes, __u16 maxPacketSize, __u8* dataptr, int length) {
	;
}

void DeviceProxy_dot11::receive_data(__u8 endpoint,__u8 attributes, __u16 maxPacketSize, __u8** dataptr, int* length, int timeout) {
	;
}

void DeviceProxy_dot11::setConfig(Configuration* fs_cfg, Configuration* hs_cfg, bool hs) {
	;
}

void DeviceProxy_dot11::claim_interface(__u8 interface) {
	;
}

void DeviceProxy_dot11::release_interface(__u8 interface) {
	;
}

__u8 DeviceProxy_dot11::get_address() {
	return 1;
}

static DeviceProxy_dot11 *proxy;

extern "C" {
	DeviceProxy * get_deviceproxy_plugin(ConfigParser *cfg) {
		proxy = new DeviceProxy_dot11(cfg);
		return (DeviceProxy *) proxy;
	}
	
	void destroy_plugin() {
		delete proxy;
	}
}
