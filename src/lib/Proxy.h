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
 * Proxy.h
 *
 * Created on: Dec 8, 2013
 */
#ifndef USBPROXY_PROXY_H
#define USBPROXY_PROXY_H

#include "Plugins.h"
#include <stddef.h>
#include <linux/types.h>

class Configuration;

class Proxy {
public:
	static const __u8 plugin_type=0;

	virtual ~Proxy() {}

	virtual void send_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8* dataptr,int length)=0;
	virtual bool send_wait_complete(__u8 endpoint,int timeout=500) {return true;}
	virtual void receive_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8** dataptr, int* length,int timeout=500)=0;
	virtual void setConfig(Configuration* fs_cfg,Configuration* hs_cfg,bool hs)=0;
	virtual char* toString() {return NULL;}
};

#endif /* USBPROXY_PROXY_H */
