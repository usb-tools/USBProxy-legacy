/*
 * This file is part of USBProxy.
 */

#include <iostream>

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include "USBString.h"
#include "DeviceProxy.h"

USBString::USBString(DeviceProxy* proxy,__u8 _index,__u16 _languageId) {
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
	if (proxy->control_request(&setup_packet,&len,(__u8*)descriptor) < 0) {
		std::cerr << "Error sending control request!\n";
		exit(1);
	}
	len=descriptor->bLength;
	descriptor=(usb_string_descriptor*)realloc(descriptor,len);
	if (len>8) {
		setup_packet.wLength=len;
		if (proxy->control_request(&setup_packet,&len,(__u8*)descriptor) < 0) {
			std::cerr << "Error sending control request!\n";
			exit(1);
		}
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
	if (descriptor) {
		free(descriptor);
		descriptor=NULL;
	}
}

char * USBString::get_ascii() {
	__u8 strlen=get_char_count();
	char* buf=(char*)malloc(strlen+1);

	int len=((descriptor->bLength)>>1)-1;
	int i_uni;
	int i_asc;
	for (i_uni=0,i_asc=0;i_uni<len;i_uni++) {
		if (descriptor->wData[i_uni]&0xff00) {
			buf[i_asc++]='?';
		} else {
			buf[i_asc++]=descriptor->wData[i_uni]&0xff;
		}
	}
	buf[i_asc]=0;
	return buf;
}

const usb_string_descriptor* USBString::get_descriptor() {return descriptor;}

__u16 USBString::get_languageId() {return languageId;}

__u8  USBString::get_index() {return index;}

__u8  USBString::get_char_count() {return ((descriptor->bLength)>>1)-1;}

void USBString::append_char(__u16 u) {
	if (index!=0 || languageId!=0) {fprintf(stderr,"append_char() may only be called on the zero USBString.\n");return;}
	int len=(descriptor->bLength)+2;
	descriptor=(usb_string_descriptor*)realloc(descriptor,len);
	descriptor->bLength=len;
	descriptor->wData[(len>>1)-2]=u;
}
