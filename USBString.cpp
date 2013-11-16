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
 * USBString.cpp
 *
 * Created on: Nov 7, 2013
 */

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include "USBString.h"

USBString::USBString(USBDeviceProxy* proxy,__u8 _index,__u16 _languageId) {
	index=_index;
	languageId=_languageId;
	descriptor=(usb_string_descriptor *)malloc(8);
	usb_ctrlrequest setup_packet;
	setup_packet.bRequestType=USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
	setup_packet.bRequest=USB_REQ_GET_DESCRIPTOR;
	setup_packet.wValue=USB_DT_STRING<<8|index;
	setup_packet.wIndex=languageId;
	setup_packet.wLength=8;
	int len=0;
	proxy->control_request(&setup_packet,&len,(__u8*)descriptor);
	len=descriptor->bLength;
	descriptor=(usb_string_descriptor*)realloc(descriptor,len);
	if (len>8) {
		setup_packet.wLength=len;
		proxy->control_request(&setup_packet,&len,(__u8*)descriptor);
	}
}

//create from descriptor
USBString::USBString(const usb_string_descriptor* _descriptor,__u8 _index,__u16 _languageId) {
	int len=_descriptor->bLength;
	index=_index;
	languageId=_languageId;
	descriptor=(usb_string_descriptor *)malloc(len);
	memcpy(descriptor,_descriptor,len);
}
//create from ascii string
USBString::USBString(const char* value,__u8 _index,__u16 _languageId) {
	int len=strlen(value);
	index=_index;
	languageId=_languageId;
	descriptor=(usb_string_descriptor *)calloc(len+1,2);
	descriptor->bLength=((len+1)<<1);
	descriptor->bDescriptorType=3;
	int i;
	for(i=0;i<len;i++) {
		descriptor->wData[i]=value[i];
	}
}

//create from unicode-LE16 string
USBString::USBString(const __u16* value,__u8 _index,__u16 _languageId) {
	index=_index;
	languageId=_languageId;
	const __u16* p=value;
	int len=0;
	while (*p++) {len++;}
	descriptor=(usb_string_descriptor *)calloc(len+1,2);
	descriptor->bLength=((len+1)<<1);
	descriptor->bDescriptorType=3;
	p=value;
	int i;
	for(i=0;i<len;i++) {
		descriptor->wData[i]=value[i];
	}
}

USBString::~USBString() {
	if (descriptor) {free(descriptor);}
}

void USBString::get_ascii(char* buf,int buflen) {
	int len=(descriptor->bLength)>>1;
	int i_uni;
	int i_asc;
	if (!buflen) {return;}
	for (i_uni=0,i_asc=0;i_uni<len;i_uni++) {
		if (i_asc==(buflen-1)) {buf[i_asc]=0;return;}
		if (descriptor->wData[i_uni]&0xff00) {
			buf[i_asc++]='?';
		} else {
			buf[i_asc++]=descriptor->wData[i_uni]&0xff;
		}
	}
	buf[i_asc]=0;
}

const usb_string_descriptor* USBString::get_descriptor() {return descriptor;}

__u16 USBString::get_languageId() {return languageId;}

__u8  USBString::get_index() {return index;}

__u8  USBString::get_char_count() {return ((descriptor->bLength)>>1)-1;}

void USBString::print_ascii(FILE *stream) {
	__u8 strlen=get_char_count();
	char* buf=(char*)malloc(strlen+1);
	get_ascii(buf,strlen+1);
	fprintf(stream,"%s",buf);
	free(buf);
}

void USBString::append_char(__u16 u) {
	if (index!=0 || languageId!=0) {fprintf(stderr,"append_char() may only be called on the zero USBString.\n");return;}
	int len=(descriptor->bLength)+2;
	descriptor=(usb_string_descriptor*)realloc(descriptor,len);
	descriptor->bLength=len;
	descriptor->wData[(len>>1)-2]=u;
}
