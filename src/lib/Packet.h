/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_PACKET_H
#define USBPROXY_PACKET_H

#include <memory> // shared_ptr

#include <stdlib.h>
#include <linux/usb/ch9.h>

#include "SafeQueue.hpp"

class Packet {
public:
	__u8	bEndpoint;
	__u16	wLength;
	bool	filter;
	bool	transmit;
	__u8*	data;

	Packet(__u8 _endpoint,__u8* _data,__u16 _length,bool _filter=true) : bEndpoint(_endpoint),wLength(_length),filter(_filter),transmit(true),data(_data) {}
	virtual ~Packet() {if (data) {free(data);data=NULL;}}
};
typedef std::shared_ptr<Packet> PacketPtr;
typedef SafeQueue<PacketPtr> PacketQueue;

class SetupPacket : public Packet {
public:
	usb_ctrlrequest ctrl_req;
	int				source;
	// TODO: share more fields with Packet
	bool			filter_out;
	bool 			transmit_out;
	bool			filter_in;
	bool			transmit_in;

	SetupPacket(usb_ctrlrequest _ctrl_req,__u8* _data,bool _filter=true)
		: Packet(0, _data, 0, _filter)
		, ctrl_req(_ctrl_req)
		, source(0)
		, filter_out(_filter)
		, transmit_out(true)
		, filter_in(_filter)
		, transmit_in(true)
	{}
};

#endif /* USBPROXY_PACKET_H */
