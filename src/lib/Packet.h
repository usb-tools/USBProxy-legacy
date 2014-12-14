/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_PACKET_H
#define USBPROXY_PACKET_H

#include <stdlib.h>
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

struct SetupPacket {
	usb_ctrlrequest ctrl_req;
	int				source;
	bool			filter_out;
	bool 			transmit_out;
	bool			filter_in;
	bool			transmit_in;
	__u8*			data;

	SetupPacket(usb_ctrlrequest _ctrl_req,__u8* _data,bool _filter=true) : ctrl_req(_ctrl_req),source(0),filter_out(_filter),transmit_out(true),filter_in(_filter),transmit_in(true),data(_data) {}
	~SetupPacket() {if (data) {free(data);data=NULL;}}
};

#endif /* USBPROXY_PACKET_H */
