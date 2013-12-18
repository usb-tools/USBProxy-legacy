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
 */

#ifndef USBPROXY_DEVICEPROXY_LOOPBACK_H
#define USBPROXY_DEVICEPROXY_LOOPBACK_H

#include "DeviceProxy.h"
#include <linux/usb/ch9.h>

class Configuration;

struct pkt {
	int length;
	__u8 *data;
};

struct usb_string {
  __u8 id;
  const char *s;
};

struct usb_strings {
  __u16 language;	/* 0x0409 for en-us */
  struct usb_string *strings;
};

class DeviceProxy_Loopback : public DeviceProxy {
private:
	bool p_is_connected;
	struct pkt *buffer;
	int head, tail;
	bool full;
	struct usb_device_descriptor loopback_device_descriptor;
	struct usb_config_descriptor loopback_config_descriptor;
	struct usb_interface_descriptor loopback_interface_descriptor;
	struct usb_string ;

public:
	static int debugLevel;
	DeviceProxy_Loopback(int vendorId, int productId);
	~DeviceProxy_Loopback();

	int connect();
	void disconnect();
	void reset();
	bool is_connected();

	bool is_highspeed();

	//return -1 to stall
	int control_request(const usb_ctrlrequest *setup_packet, int *nbytes, __u8* dataptr,int timeout=500);

	void send_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8* dataptr,int length);
	bool send_wait_complete(__u8 endpoint,int timeout=500) {return true;}
	void receive_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8** dataptr, int* length,int timeout=500);
	void setConfig(Configuration* fs_cfg,Configuration* hs_cfg,bool hs);
	char* toString() {return (char *) "Lookback device";}

	void claim_interface(__u8 interface);
	void release_interface(__u8 interface);

	__u8 get_address();
};

#endif /* USBPROXY_DEVICEPROXY_LOOPBACK_H */
