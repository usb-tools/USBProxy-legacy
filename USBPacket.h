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
 * Packet.h
 *
 * Created on: Nov 11, 2013
 */
#ifndef PACKET_H_
#define PACKET_H_

#include <linux/usb/ch9.h>

struct USBPacket {
	__u8	bEndpoint;
	__u16	wLength;
	bool	filter;
	bool	transmit;
	__u8*	data;

	USBPacket(__u8 _endpoint,__u8* _data,__u16 _length,bool _filter=true) : bEndpoint(_endpoint),wLength(_length),filter(_filter),transmit(true),data(_data) {}
	~USBPacket() {if (data) {free(data);data=NULL;}}
};

struct USBSetupPacket {
	usb_ctrlrequest ctrl_req;
	bool	filter;
	bool	transmit;
	__u8*	data;

	USBSetupPacket(usb_ctrlrequest _ctrl_req,__u8* _data,bool _filter=true) : ctrl_req(_ctrl_req),filter(_filter),transmit(true),data(_data) {}
	~USBSetupPacket() {if (data) {free(data);data=NULL;}}
};

#endif /* PACKET_H_ */
