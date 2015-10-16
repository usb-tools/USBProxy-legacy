/*
 * This file is part of USBProxy.
 */

#include <dlfcn.h>
#include <sys/types.h>
#include <stdio.h>

#include "Plugins.h"
#include "PluginManager.h"

// FIXME: replace all of this repetative code with something based on Templates

typedef DeviceProxy* (*device_plugin_getter)(ConfigParser *);
typedef HostProxy* (*host_plugin_getter)(ConfigParser *);
typedef PacketFilter* (*filter_plugin_getter)(ConfigParser *);
typedef Injector* (*injector_plugin_getter)(ConfigParser *);

void *PluginManager::load_shared_lib(std::string plugin_name) {
	std::string plugin_file = PLUGIN_PATH + plugin_name + ".so";
	void* plugin_lib = dlopen(plugin_file.c_str(), RTLD_LAZY);
	if(!plugin_lib)
		fprintf(stderr, "error opening library %s\n",  dlerror());
	
	return plugin_lib;
}

int PluginManager::load_plugins(ConfigParser *cfg)
{
	std::string plugin_file;
	void* plugin_func;
	PacketFilter *filter;
	fprintf(stderr, "Loading plugins from %s\n", PLUGIN_PATH);

	device_plugin_getter dp_ptr;
	host_plugin_getter hp_ptr;
	filter_plugin_getter f_ptr;
	injector_plugin_getter i_ptr;
	
	// Device Proxy
	void* dplugin_lib = load_shared_lib(cfg->get("DeviceProxy"));
	if(!dplugin_lib)
		return PLUGIN_MANAGER_CANNOT_FIND_FILE;
	plugin_func = dlsym(dplugin_lib, "get_deviceproxy_plugin");
	if (!plugin_func) {
		dlclose(dplugin_lib);
		return PLUGIN_MANAGER_CANNOT_FIND_FILE;
	}
	handleList.push_back(plugin_func);
	dp_ptr = *reinterpret_cast<device_plugin_getter*>(&plugin_func);
	device_proxy = (*(dp_ptr))(cfg);
	
	// Host Proxy
	void* hplugin_lib = load_shared_lib(cfg->get("HostProxy"));
	if(!hplugin_lib)
		return PLUGIN_MANAGER_CANNOT_FIND_FILE;
	plugin_func = dlsym(hplugin_lib, "get_hostproxy_plugin");
	if (!plugin_func) {
		dlclose(hplugin_lib);
		return PLUGIN_MANAGER_CANNOT_FIND_FILE;
	}
	handleList.push_back(plugin_func);
	hp_ptr = *reinterpret_cast<host_plugin_getter*>(&plugin_func);
	host_proxy = (*(hp_ptr))(cfg);
	
	// Plugins
	std::vector<std::string> plugin_names=cfg->get_vector("Plugins");
	for(std::vector<std::string>::iterator it = plugin_names.begin();
		it != plugin_names.end(); ++it) {
		void* plugin_lib = load_shared_lib(*it);
		if(plugin_lib==NULL)
			return PLUGIN_MANAGER_CANNOT_FIND_FILE;
		plugin_func = dlsym(plugin_lib, "plugin_type");
		if (!plugin_func) {
			dlclose(plugin_lib);
			continue;
		}
		int plugin_type = *static_cast<int *>(plugin_func);
		
		switch (plugin_type) {
			case PLUGIN_FILTER:
				plugin_func = dlsym(plugin_lib, "get_plugin");
				if (!plugin_func) {
					dlclose(plugin_lib);
					continue;
				}
				handleList.push_back(plugin_func);
				f_ptr = *reinterpret_cast<filter_plugin_getter*>(&plugin_func);
				filters.push_back((*(f_ptr))(cfg));
				break;
			case PLUGIN_INJECTOR:
				plugin_func = dlsym(plugin_lib, "get_plugin");
				if (!plugin_func) {
					dlclose(plugin_lib);
					continue;
				}
				handleList.push_back(plugin_func);
				i_ptr = *reinterpret_cast<injector_plugin_getter*>(&plugin_func);
				injectors.push_back((*(i_ptr))(cfg));
				break;
			case (PLUGIN_FILTER|PLUGIN_INJECTOR):
				plugin_func = dlsym(plugin_lib, "get_plugin");
				if (!plugin_func) {
					dlclose(plugin_lib);
					continue;
				}
				handleList.push_back(plugin_func);
				f_ptr = *reinterpret_cast<filter_plugin_getter*>(&plugin_func);
				filter = (*(f_ptr))(cfg);
				filters.push_back(filter);
				injectors.push_back(dynamic_cast<Injector*>(filter));
				break;
			default:
				dlclose(plugin_lib);
				fprintf(stderr, "Invalid plugin type (%s)\n", (*it).c_str());
				break;
		}
	}
	return PLUGIN_MANAGER_SUCCESS;
}

void PluginManager::add_plugin(PacketFilter* plugin)
{
	filters.push_back(plugin);
}

void PluginManager::add_plugin(Injector* plugin)
{
	injectors.push_back(plugin);
}

void PluginManager::destroy_plugins()
{	
	for(std::vector<void*>::iterator it = handleList.begin();
		it != handleList.end(); ++it)
	{
		dlclose(*it);
	}
}
