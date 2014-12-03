/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_HID_H
#define USBPROXY_HID_H

#include <linux/types.h>
#include <stdlib.h>
#include "DefinitionErrors.h"

struct usb_hid_descriptor_record  {
	__u8 bDescriptorType;
	__le16 wDescriptorLength;
} __attribute__ ((packed));

struct usb_hid_descriptor {
	__u8 bLength;
	__u8 bDescriptorType;
	__le16 bcdHID;
	__u8 bCountryCode;
	__u8 bNumDescriptors;
	usb_hid_descriptor_record descriptors[1];
} __attribute__ ((packed));

class HID {
private:
	usb_hid_descriptor* descriptor;

public:
	HID(const __u8* p);
	HID(const usb_hid_descriptor* _descriptor);
	HID(__u16 bcdHID,__u8 bCountryCode,__u8 bNumDescriptors,usb_hid_descriptor_record* descriptors);
	~HID();
	const usb_hid_descriptor* get_descriptor();
	size_t get_full_descriptor_length();
	void get_full_descriptor(__u8** p);
	void print(__u8 tabs=0);
	const definition_error is_defined();
};

#endif /* USBPROXY_HID_H */
