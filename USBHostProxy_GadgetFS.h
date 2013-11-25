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
 * USBHostProxyGadgetFS.h
 *
 * Created on: Nov 21, 2013
 */
#ifndef USBHOSTPROXYGADGETFS_H_
#define USBHOSTPROXYGADGETFS_H_

extern "C" {
#include <linux/usb/gadgetfs.h>
}
#include "USBHostProxy.h"

class USBHostProxy_GadgetFS: public USBHostProxy {
private:
	const char * device_path;
	bool p_is_connected;
	char *descriptor_buf;

public:
	USBHostProxy_GadgetFS(const char * _device_path);
	virtual ~USBHostProxy_GadgetFS();

	int connect(USBDevice* device);
	void disconnect();
	void reset();
	bool is_connected();

	//return 0 in usb_ctrlrequest->brequest if there is no request
	int control_request(usb_ctrlrequest *setup_packet, int *nbytes, __u8** dataptr);
	void send_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8* dataptr,int length);
	void receive_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8** dataptr, int* length);
};

#endif /* USBHOSTPROXYGADGETFS_H_ */
