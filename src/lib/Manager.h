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

class PluginManager;
class ConfigParser;

class Injector;
class RelayReader;
class RelayWriter;

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
	USBM_STOPPING=3,
	USBM_SETUP_ABORT=4,
	// midified 20141015 atsumi@aizulab.com for reset bust
	USBM_RESET=5
};

class Manager {
private:
	Manager_status status;
	DeviceProxy* deviceProxy;
	HostProxy* hostProxy;
	PluginManager *plugin_manager;
	Device* device;
	__u8 haltSignal;

	PacketFilter** filters;
	__u8 filterCount;

	Injector** injectors;
	__u8 injectorCount;

	pthread_t* injectorThreads;

	Endpoint* in_endpoints[16];
	RelayReader* in_readers[16];
	RelayWriter* in_writers[16];
	pthread_t in_readerThreads[16];
	pthread_t in_writerThreads[16];

	Endpoint* out_endpoints[16];
	RelayReader* out_readers[16];
	RelayWriter* out_writers[16];
	pthread_t out_readerThreads[16];
	pthread_t out_writerThreads[16];

	void start_data_relaying();

public:
	Manager();
	virtual ~Manager();

	void load_plugins(ConfigParser *cfg);

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

	// modified 20140924 atsumi@aizulab.com
  void set_status( Manager_status status_) { status = status_;}

	void start_control_relaying();
	void stop_relaying();
	void cleanup();
};

#endif /* USBPROXY_MANAGER_H */
