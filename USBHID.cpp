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
 * USBHID.cpp
 *
 * Created on: Nov 8, 2013
 */

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include "USBHID.h"



USBHID::USBHID(const __u8* p) {
	descriptor=(usb_hid_descriptor *)malloc(p[0]);
	memcpy(descriptor,p,p[0]);
}

USBHID::USBHID(const usb_hid_descriptor* _descriptor) {
	descriptor=(usb_hid_descriptor *)malloc(_descriptor->bLength);
	memcpy(descriptor,_descriptor,_descriptor->bLength);
}

USBHID::USBHID(__u16 bcdHID,__u8 bCountryCode,__u8 bNumDescriptors,usb_hid_descriptor_record* descriptors) {
	size_t bLength=6+bNumDescriptors*3;
	descriptor=(usb_hid_descriptor *)malloc(bLength);
	descriptor->bLength=bLength;
	descriptor->bDescriptorType=0x21;
	descriptor->bcdHID=bcdHID;
	descriptor->bCountryCode=bCountryCode;
	descriptor->bNumDescriptors=bNumDescriptors;
	if (bNumDescriptors) {memcpy(descriptor->descriptors,descriptors,bNumDescriptors*3);}
}

USBHID::~USBHID() {
	if (descriptor) {
		free(descriptor);
		descriptor=NULL;
	}
}

const usb_hid_descriptor* USBHID::get_descriptor() {
	return descriptor;
}

size_t USBHID::get_full_descriptor_length() {
	return descriptor->bLength;
}

void USBHID::get_full_descriptor(__u8** p) {
	memcpy(*p,descriptor,descriptor->bLength);
	*p=*p+descriptor->bLength;
}

void USBHID::print(__u8 tabs) {
	unsigned int i;
	for(i=0;i<tabs;i++) {putchar('\t');}
	printf("HID:");
	for(i=0;i<descriptor->bLength;i++) {printf(" %02x",((__u8*)descriptor)[i]);}
	putchar('\n');
}
