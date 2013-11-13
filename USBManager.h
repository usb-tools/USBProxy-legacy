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
 * USBManager.h
 *
 * Created on: Nov 12, 2013
 */
#ifndef USBMANAGER_H_
#define USBMANAGER_H_

#include <linux/usb/ch9.h>
#include "USBDeviceProxy.h"
#include "USBHostProxy.h"
#include "USBRelayer.h"
#include "USBInjector.h"
#include "USBPacketFilter.h"

#include "USBDevice.h"
#include "USBEndpoint.h"
#include "USBPacket.h"
#include <pthread.h>

class USBInjector;

class USBManager {
private:
	USBDeviceProxy* deviceProxy;
	USBHostProxy* hostProxy;
	USBDevice* device;

	USBPacketFilter** filters;
	__u8 filterCount;

	USBInjector** injectors;
	__u8 injectorCount;

	pthread_t* injectorThreads;

	USBEndpoint* in_endpoints[16];
	USBRelayer* in_relayers[16];
	pthread_t in_relayerThreads[16];
	boost::lockfree::queue<USBPacket*>* in_queue[16];

	USBEndpoint* out_endpoints[16];
	USBRelayer* out_relayers[16];
	pthread_t out_relayerThreads[16];
	boost::lockfree::queue<USBPacket*>* out_queue[16];

public:
	USBManager(USBDeviceProxy* _deviceProxy,USBHostProxy* _hostProxy);
	virtual ~USBManager();
	int inject_packet(USBPacket *packet);
	int inject_setup_in(usb_ctrlrequest request,__u8** data,__u16 *transferred, bool filter);
	int inject_setup_out(usb_ctrlrequest request,__u8* data,bool filter);

	void add_injector(USBInjector* _injector);
	void remove_injector(__u8 index);
	USBInjector* get_injector(__u8 index);
	__u8 get_injector_count();

	void add_filter(USBPacketFilter* _filter);
	void remove_filter(__u8 index);
	USBPacketFilter* get_filter(__u8 index);
	__u8 get_filter_count();

	void start_relaying();
	void stop_relaying();
};

#endif /* USBMANAGER_H_ */
