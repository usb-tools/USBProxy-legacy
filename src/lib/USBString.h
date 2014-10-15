/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
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
 *
 * USBString.h
 *
 * Created on: Nov 7, 2013
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
