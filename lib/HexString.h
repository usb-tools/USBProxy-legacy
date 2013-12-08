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
 * HexPrint.h
 *
 * Created on: Dec 3, 2013
 */
#ifndef USBPROXY_HEXSTRING_H
#define USBPROXY_HEXSTRING_H

#include <linux/types.h>
#include <stdlib.h>
#include "TRACE.h"
#include <string.h>

//TODO also need to deal with print_ascii in USBString
//TODO look for looped putchar as well

static char* hex_string_wide(void* buf,int length,int width=32) {
	char* outbuf;
	if (!buf) {
		outbuf=(char*)malloc(1);
		*outbuf=0;
		return outbuf;
	}
	int lines=length/width;
	if (length%width) lines++;
	outbuf=(char* )malloc(length*3+lines+1);
	char* p=outbuf;
	int i;
	for(i=0;i<length;i++) {
			sprintf(p,(i%width)?" %02x":"\n\t%02x",((__u8*)buf)[i]);
			p+=(i%width)?3:4;
	}
	*p=0;
	return outbuf;
}

static char* hex_string(void* buf,int length) {
	char* outbuf;
	if (!buf) {
		outbuf=(char*)malloc(1);
		*outbuf=0;
		return outbuf;
	}
	if (length>32) return hex_string_wide(buf,length);
	outbuf=(char* )malloc(length*3+1);
	char* p=outbuf;
	int i;
	for(i=0;i<length;i++) {
			sprintf(p,i?" %02x":"%02x",((__u8*)buf)[i]);
			p+=i?3:2;
	}
	*p=0;
	return outbuf;
}



#endif /* USBPROXY_HEXSTRING_H */
