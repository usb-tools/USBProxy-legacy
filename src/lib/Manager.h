/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_MANAGER_H
#define USBPROXY_MANAGER_H

#include <thread>
#include <vector>

#include <linux/usb/ch9.h>

#include "Packet.h"

class PluginManager;
class ConfigParser;

class Injector;
class RelayReader;
class RelayWriter;

class Device;
class Endpoint;

class DeviceProxy;
class HostProxy;
class PacketFilter;


enum Manager_status {
	USBM_IDLE=0,
	USBM_SETUP=1,
	USBM_RELAYING=2,
	USBM_STOPPING=3,
	USBM_SETUP_ABORT=4,
	USBM_RESET=5
};

class Manager {
private:
	Manager_status status;
	DeviceProxy* deviceProxy;
	HostProxy* hostProxy;
	PluginManager *plugin_manager;
	Device* device;

	PacketFilter** filters;
	__u8 filterCount;

	Injector** injectors;
	__u8 injectorCount;

	std::vector<std::thread> injectorThreads;

	Endpoint* in_endpoints[16];
	PacketQueue* in_queues[16];
	RelayReader* in_readers[16];
	RelayWriter* in_writers[16];
	std::thread in_readerThreads[16];
	std::thread in_writerThreads[16];

	Endpoint* out_endpoints[16];
	PacketQueue* out_queues[16];
	RelayReader* out_readers[16];
	RelayWriter* out_writers[16];
	std::thread out_readerThreads[16];
	std::thread out_writerThreads[16];
	PacketQueue _readersend;
	PacketQueue _writersend;

	void start_data_relaying();
	unsigned _debug_level;

public:
	Manager(unsigned debug_level);
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
