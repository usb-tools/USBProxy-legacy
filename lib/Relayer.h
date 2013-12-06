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
 * Relayer.h
 *
 * Created on: Nov 11, 2013
 */
#ifndef USBPROXY_RELAYER_H
#define USBPROXY_RELAYER_H

#include "linux/types.h"
#include <unistd.h>
#include <stdio.h>
#include <boost/atomic.hpp>
#include <boost/lockfree/queue.hpp>
#include "DeviceProxy.h"
#include "HostProxy.h"
#include "Endpoint.h"
#include "Packet.h"
#include "PacketFilter.h"
#include "Manager.h"

class Manager;

class Relayer {
private:
	boost::lockfree::queue<Packet*>* queue;
	boost::lockfree::queue<SetupPacket*>* queue_ep0;
	Endpoint* endpoint;
	DeviceProxy* device;
	HostProxy* host;
	PacketFilter** filters;
	Manager* manager;
	__u8 filterCount;

public:
	boost::atomic_bool halt;

	Relayer(Endpoint* _endpoint,DeviceProxy* _device,HostProxy* _host,boost::lockfree::queue<Packet*>* _queue);
	Relayer(Manager* _manager,Endpoint* _endpoint,DeviceProxy* _device,HostProxy* _host,boost::lockfree::queue<SetupPacket*>* _queue);
	virtual ~Relayer();
	void relay();
	void relay_ep0();
	void add_filter(PacketFilter* filter);
	void set_manager(Manager* _manager) {manager=_manager;}

	static void* relay_helper(void* context);
};

#endif /* USBPROXY_RELAYER_H */
