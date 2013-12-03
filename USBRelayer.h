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
 * USBRelayer.h
 *
 * Created on: Nov 11, 2013
 */
#ifndef USBRELAYER_H_
#define USBRELAYER_H_

#include "linux/types.h"
#include <unistd.h>
#include <stdio.h>
#include <boost/atomic.hpp>
#include <boost/lockfree/queue.hpp>
#include "USBDeviceProxy.h"
#include "USBHostProxy.h"
#include "USBEndpoint.h"
#include "USBPacket.h"
#include "USBPacketFilter.h"
#include "USBManager.h"

class USBManager;

class USBRelayer {
private:
	boost::lockfree::queue<USBPacket*>* queue;
	boost::lockfree::queue<USBSetupPacket*>* queue_ep0;
	USBEndpoint* endpoint;
	USBDeviceProxy* device;
	USBHostProxy* host;
	USBPacketFilter** filters;
	USBManager* manager;
	__u8 filterCount;

public:
	boost::atomic_bool halt;

	USBRelayer(USBEndpoint* _endpoint,USBDeviceProxy* _device,USBHostProxy* _host,boost::lockfree::queue<USBPacket*>* _queue);
	USBRelayer(USBManager* _manager,USBEndpoint* _endpoint,USBDeviceProxy* _device,USBHostProxy* _host,boost::lockfree::queue<USBSetupPacket*>* _queue);
	virtual ~USBRelayer();
	void relay();
	void relay_ep0();
	void add_filter(USBPacketFilter* filter);
	void set_manager(USBManager* _manager) {manager=_manager;}

	static void* relay_helper(void* context);
};

#endif /* USBRELAYER_H_ */
