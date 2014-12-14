/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_USBSTRING_H_
#define USBPROXY_USBSTRING_H_

#include <linux/usb/ch9.h>
#include <stdio.h>

class DeviceProxy;

class USBString {
private:
	usb_string_descriptor* descriptor;
	__u16 languageId;
	__u8  index;

public:
	USBString(DeviceProxy* proxy,__u8 _index,__u16 _languageId);
	USBString(const usb_string_descriptor* _descriptor,__u8 _index,__u16 _languageId);
	//create from ascii string
	USBString(const char* value,__u8 _index,__u16 _languageId);
	//create from unicode string
	USBString(const __u16* value,__u8 _index,__u16 _languageId);
	~USBString();
	const usb_string_descriptor* get_descriptor();
	__u16 get_languageId();
	__u8  get_index();
	char* get_ascii();
	__u8 get_char_count();
	void append_char(__u16 u);
};


#endif /* USBPROXY_USBSTRING_H_ */
