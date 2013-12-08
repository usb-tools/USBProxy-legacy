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
 * Manager.h
 *
 * Created on: Nov 12, 2013
 */
#ifndef USBPROXY_MANAGER_H
#define USBPROXY_MANAGER_H

#include <linux/usb/ch9.h>
#include <pthread.h>
#include <boost/atomic.hpp>
#include <boost/lockfree/queue.hpp>

class Injector;
class Relayer;
class Device;
class Endpoint;

class Packet;
class SetupPacket;
class DeviceProxy;
class HostProxy;
class PacketFilter;

enum Manager_status {
	USBM_IDLE=0,
	USBM_SETUP=1,
	USBM_RELAYING=2,
	USBM_STOPPING=3
};

class Manager {
private:
	Manager_status status;
	DeviceProxy* deviceProxy;
	HostProxy* hostProxy;
	Device* device;

	PacketFilter** filters;
	__u8 filterCount;

	Injector** injectors;
	__u8 injectorCount;

	pthread_t* injectorThreads;

	Endpoint* in_endpoints[16];
	Relayer* in_relayers[16];
	pthread_t in_relayerThreads[16];
	boost::lockfree::queue<Packet*>* in_queue[16];
	//boost::lockfree::queue<SetupPacket*>* in_queue_ep0;

	Endpoint* out_endpoints[16];
	Relayer* out_relayers[16];
	pthread_t out_relayerThreads[16];
	boost::lockfree::queue<Packet*>* out_queue[16];
	boost::lockfree::queue<SetupPacket*>* out_queue_ep0;
	void start_data_relaying();

public:
	Manager(DeviceProxy* _deviceProxy,HostProxy* _hostProxy);
	virtual ~Manager();
	void inject_packet(Packet *packet);
	void inject_setup_in(usb_ctrlrequest request,__u8** data,__u16 *transferred, bool filter);
	void inject_setup_out(usb_ctrlrequest request,__u8* data,bool filter);

	void add_injector(Injector* _injector);
	void remove_injector(__u8 index,bool freeMemory=true);
	Injector* get_injector(__u8 index);
	__u8 get_injector_count();

	void add_filter(PacketFilter* _filter);
	void remove_filter(__u8 index,bool freeMemory=true);
	PacketFilter* get_filter(__u8 index);
	__u8 get_filter_count();

	void setConfig(__u8 index);

	enum Manager_status get_status() {return status;}

	void start_control_relaying();
	void stop_relaying();
};

#endif /* USBPROXY_MANAGER_H */
