/*
 * Copyright 2014 Dominic Spill
 * Copyright 2014 Jahmel Harris
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
 */

#include <dlfcn.h>
#include <sys/types.h>
#include <stdio.h>

#include "Plugins.h"
#include "PluginManager.h"

typedef DeviceProxy* (*device_plugin_getter)(ConfigParser *);
typedef HostProxy* (*host_plugin_getter)(ConfigParser *);
typedef PacketFilter* (*filter_plugin_getter)(ConfigParser *);
typedef Injector* (*injector_plugin_getter)(ConfigParser *);

void PluginManager::load_plugins(ConfigParser *cfg)
{
	std::string plugin_lib;
	fprintf(stderr, "Loading plugins from %s\n", PLUGIN_PATH);
	
	plugin_lib = PLUGIN_PATH + cfg->get("DeviceProxy") + ".so";
	void* DeviceProxyPlugin = dlopen(plugin_lib.c_str(), RTLD_LAZY);
	if(!DeviceProxyPlugin)
	{
		fprintf(stderr, "error opening library %s\n", plugin_lib.c_str());
	}
	
	device_plugin_getter dp_ptr;
	dp_ptr = (device_plugin_getter) dlsym(DeviceProxyPlugin, "get_deviceproxy_plugin");
	handleList.push_back((void *)dp_ptr);
	device_proxy = (*(dp_ptr))(cfg);
	
	plugin_lib = PLUGIN_PATH + cfg->get("HostProxy") + ".so";
	void* HostProxyPlugin = dlopen(plugin_lib.c_str(), RTLD_LAZY);
	if(!HostProxyPlugin)
	{
		fprintf(stderr, "error opening library %s\n", plugin_lib.c_str());
	}
	
	host_plugin_getter hp_ptr;
	hp_ptr = (host_plugin_getter) dlsym(HostProxyPlugin, "get_hostproxy_plugin");
	handleList.push_back((void *)hp_ptr);
	host_proxy = (*(hp_ptr))(cfg);
	
	std::vector<std::string> filter_names = cfg->get_vector("Filters");
	for(std::vector<std::string>::iterator it = filter_names.begin(); it != filter_names.end(); ++it)
	{
		plugin_lib = PLUGIN_PATH + it[0] + ".so";
		fprintf(stderr, "Opening %s\n", plugin_lib.c_str());
		void* FilterPlugin = dlopen(plugin_lib.c_str(), RTLD_LAZY);
		if(!FilterPlugin)
		{
			fprintf(stderr, "error opening library %s\n", plugin_lib.c_str());
		}
		
		filter_plugin_getter f_ptr;
		f_ptr = (filter_plugin_getter) dlsym(FilterPlugin, "get_filter_plugin");
		handleList.push_back((void *)f_ptr);
		filters.push_back((*(f_ptr))(cfg));
	}
}

void PluginManager::destroy_plugins()
{	
	for(std::vector<void*>::iterator it = handleList.begin(); it != handleList.end(); ++it)
	{
		dlclose(*it);
	}
}

