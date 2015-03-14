/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_PLUGIN_MANAGER_H
#define USBPROXY_PLUGIN_MANAGER_H

#include <vector>
#include <unordered_map>
#include <string>

#include "DeviceProxy.h"
#include "HostProxy.h"
#include "PacketFilter.h"
#include "Injector.h"

/* Expand this list as needed */
enum plugin_manager_error {
	PLUGIN_MANAGER_SUCCESS = 0,
	PLUGIN_MANAGER_UNKNOWN_ERROR = 1,
	PLUGIN_MANAGER_CANNOT_FIND_FILE = 2
};

class PluginManager
{
	private:
		std::vector<void*> handleList;
		void *load_shared_lib(std::string plugin_name);

	public:
		/* These should have getter/setters */
		DeviceProxy* device_proxy;
		HostProxy* host_proxy;
		std::vector<PacketFilter*> filters;
		std::vector<Injector*> injectors;
		
		PluginManager(){};
		int load_plugins(ConfigParser *cfg);
		void add_plugin(PacketFilter* plugin);
		void add_plugin(Injector* plugin);
		void destroy_plugins();
	
};
#endif /* USBPROXY_PLUGIN_MANAGER_H */
