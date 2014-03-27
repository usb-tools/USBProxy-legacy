#ifndef _PLUGIN_MANAGER_H
#define _PLUGIN_MANAGER_H

#include <vector>
#include <iostream>

#include "DeviceProxy.h"
#include "HostProxy.h"
#include "PacketFilter.h"
#include "Injector.h"

class PluginManager
{
	private:
		std::vector<DeviceProxy*> DeviceProxyPlugins;
		std::vector<HostProxy*> HostProxyPlugins;
		std::vector<PacketFilter*> PacketFilterPlugins;
		std::vector<Injector*> InjectorPlugins;
		std::vector<void*> handleList;

		void loadFromDisk();
		std::vector<std::string>* getFilenames();
		void initAllPlugins();

	public:
	PluginManager(){};
	void loadAllPlugins();
	void destroyAllPlugins();
	
};
#endif
