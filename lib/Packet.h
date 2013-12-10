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
 * Packet.h
 *
 * Created on: Nov 11, 2013
 */
#ifndef USBPROXY_PACKET_H
#define USBPROXY_PACKET_H

#include <linux/usb/ch9.h>

struct Packet {
	__u8	bEndpoint;
	__u16	wLength;
	bool	filter;
	bool	transmit;
	__u8*	data;

	Packet(__u8 _endpoint,__u8* _data,__u16 _length,bool _filter=true) : bEndpoint(_endpoint),wLength(_length),filter(_filter),transmit(true),data(_data) {}
	~Packet() {if (data) {free(data);data=NULL;}}
};

#define SETUPPACKET_REQUEST 0
#define SETUPPACKET_RESPONSE_ACK 1
#define SETUPPACKET_RESPONSE_DATA 2
#define SETUPPACKET_RESPONSE_STALL 3

struct SetupPacket {
	usb_ctrlrequest ctrl_req;
	int				source;
	__u8			status;
	bool			filter;
	bool			transmit;
	__u8*			data;

	SetupPacket(usb_ctrlrequest _ctrl_req,__u8* _data,bool _filter=true) : ctrl_req(_ctrl_req),source(0),status(0),filter(_filter),transmit(true),data(_data) {}
	SetupPacket(usb_ctrlrequest _ctrl_req,int _source,__u8* _data,bool _filter=true) : ctrl_req(_ctrl_req),source(_source),status(0),filter(_filter),transmit(true),data(_data) {}
	~SetupPacket() {if (data) {free(data);data=NULL;}}
};

#endif /* USBPROXY_PACKET_H */
