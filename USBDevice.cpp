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
 */

#include <linux/types.h>
#include "USBDevice.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

USBDevice::USBDevice(USBDeviceProxy* proxy) {
	__u8 buf[18];
	usb_ctrlrequest setup_packet;
	setup_packet.bRequestType=USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
	setup_packet.bRequest=USB_REQ_GET_DESCRIPTOR;
	setup_packet.wValue=USB_DT_DEVICE<<8;
	setup_packet.wIndex=0;
	setup_packet.wLength=18;
	int len=0;
	proxy->control_request(&setup_packet,&len,buf);
	memcpy(&descriptor,buf,len);
	int i;
	configurations=(USBConfiguration **)malloc(sizeof(*configurations)*descriptor.bNumConfigurations);
	for(i=0;i<descriptor.bNumConfigurations;i++) {
		configurations[i]=new USBConfiguration(proxy,i);
	}
	//TODO: read string descriptors for this and all descendants
}

USBDevice::USBDevice(usb_device_descriptor* _descriptor) {
	descriptor=*_descriptor;
	configurations=(USBConfiguration **)malloc(sizeof(*configurations)*descriptor.bNumConfigurations);
}

USBDevice::USBDevice(__le16 bcdUSB,	__u8  bDeviceClass,	__u8  bDeviceSubClass,	__u8  bDeviceProtocol,	__u8  bMaxPacketSize0,	__le16 idVendor,	__le16 idProduct,	__le16 bcdDevice,	__u8  iManufacturer,	__u8  iProduct,	__u8  iSerialNumber,	__u8  bNumConfigurations) {
	descriptor.bcdUSB=bcdUSB;
	descriptor.bDeviceClass=bDeviceClass;
	descriptor.bDeviceSubClass=bDeviceSubClass;
	descriptor.bDeviceProtocol=bDeviceProtocol;
	descriptor.bMaxPacketSize0=bMaxPacketSize0;
	descriptor.idVendor=idVendor;
	descriptor.idProduct=idProduct;
	descriptor.bcdDevice=bcdDevice;
	descriptor.iManufacturer=iManufacturer;
	descriptor.iProduct=iProduct;
	descriptor.iSerialNumber=iSerialNumber;
	descriptor.bNumConfigurations=bNumConfigurations;
	configurations=(USBConfiguration **)malloc(sizeof(*configurations)*descriptor.bNumConfigurations);
}

USBDevice::~USBDevice() {
	int i;
	for(i=0;i<descriptor.bNumConfigurations;i++) {delete(configurations[i]);}
	free(configurations);
}

const usb_device_descriptor* USBDevice::getDescriptor() {
	return &descriptor;
};

//TODO: should this check whether the element already exists?
void USBDevice::add_configuration(USBConfiguration* config) {
	configurations[config->getDescriptor()->bConfigurationValue-1]=config;
}

//TODO: this should this check whether the element already exists
USBConfiguration* USBDevice::get_configuration(__u8 index) {
	return configurations[index-1];
}
