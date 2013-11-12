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

	USBInjector** injectors;
	pthread_t* injectorThreads;

	USBRelayer** relayers;
	pthread_t* relayerThreads;

	USBEndpoint** endpoints;
	USBPacketFilter** filters;

	//TODO PacketQueue** injectionQueueus;

public:
	USBManager();
	virtual ~USBManager();
	int inject_packet(USBPacket *packet);
	int inject_setup_in(usb_ctrlrequest request,__u8** data,__u16 *transferred, bool filter);
	int inject_setup_out(usb_ctrlrequest request,__u8* data,bool filter);

	//add/remove_injector();

	//connect device proxy, populate device model, enumerate endpoints
	//set up relayers for each endpoint, apply filters to each relayer
	//don't forget to include EP0
	//connect to host proxy
	//start relayer threads, start injector threads
	//start_relaying();

	//stop & join injector/relayer threads
	//disconnect from host
	//clean up relayers
	//disconnect device proxy, clean up device model & endpoints
	//don't forget to include EP0
	//stop_relaying();
};

#endif /* USBMANAGER_H_ */
