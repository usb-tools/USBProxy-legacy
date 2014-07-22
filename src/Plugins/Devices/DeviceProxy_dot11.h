/*
 * Copyright 2014 Dominic Spill
 * Copyright 2014 Mike Kershaw / dragorn
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

#ifndef USBPROXY_DEVICEPROXY_DOT11_H
#define USBPROXY_DEVICEPROXY_DOT11_H

#include "DeviceProxy.h"

class Configuration;

struct pkt {
	int length;
	__u8 *data;

	pkt(): length(0),data(NULL) {}
};

class DeviceProxy_dot11 : public DeviceProxy {
private:
	bool p_is_connected;
	struct pkt *buffer;
	int head, tail;
	bool full;
	struct usb_device_descriptor dot11_device_descriptor;
	struct usb_config_descriptor dot11_config_descriptor;
	struct usb_interface_descriptor dot11_interface_descriptor;
	struct usb_endpoint_descriptor dot11_eps[2];
	struct usb_string ;

public:
	static int debugLevel;
	DeviceProxy_dot11(int vendorId, int productId);
	DeviceProxy_dot11(ConfigParser *cfg);
	~DeviceProxy_dot11();

	int connect(int timeout=250);
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

#endif /* USBPROXY_DEVICEPROXY_DOT11_H */
