/*
 * This file is part of USBProxy.
 */

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include "HID.h"
#include "HexString.h"

HID::HID(const __u8* p) {
	descriptor=(usb_hid_descriptor *)malloc(p[0]);
	memcpy(descriptor,p,p[0]);
}

HID::HID(const usb_hid_descriptor* _descriptor) {
	descriptor=(usb_hid_descriptor *)malloc(_descriptor->bLength);
	memcpy(descriptor,_descriptor,_descriptor->bLength);
}

HID::HID(__u16 bcdHID,__u8 bCountryCode,__u8 bNumDescriptors,usb_hid_descriptor_record* descriptors) {
	size_t bLength=6+bNumDescriptors*3;
	descriptor=(usb_hid_descriptor *)malloc(bLength);
	descriptor->bLength=bLength;
	descriptor->bDescriptorType=0x21;
	descriptor->bcdHID=bcdHID;
	descriptor->bCountryCode=bCountryCode;
	descriptor->bNumDescriptors=bNumDescriptors;
	if (bNumDescriptors) {memcpy(descriptor->descriptors,descriptors,bNumDescriptors*3);}
}

HID::~HID() {
	if (descriptor) {
		free(descriptor);
		descriptor=NULL;
	}
}

const usb_hid_descriptor* HID::get_descriptor() {
	return descriptor;
}

size_t HID::get_full_descriptor_length() {
	return descriptor->bLength;
}

void HID::get_full_descriptor(__u8** p) {
	memcpy(*p,descriptor,descriptor->bLength);
	*p=*p+descriptor->bLength;
}

void HID::print(__u8 tabs) {
	char* hex=hex_string((void*)descriptor,descriptor->bLength);
	printf("%.*sHID: %s\n",tabs,TABPADDING,hex);
	free(hex);
}
