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
#include <dirent.h>
#include <iostream>

#include "Plugins.h"
#include "PluginManager.h"

using namespace std;

typedef DeviceProxy* (*plugin_infocall)(ConfigParser *);

void *PluginManager::load_plugins(const char* DeviceProxyName/*,
								  const char* HostProxyName,
								  const vector<char*>,
								  const vector<char*>*/,
								  ConfigParser *cfg)
{
	// FIXME make this more generic/install plugins to known location
	string plugin_dir = string("Plugins/Devices/");
	string plugin_lib = plugin_dir + DeviceProxyName + ".so";
	void* DeviceProxyPlugin = dlopen(plugin_lib.c_str(), RTLD_LAZY);
	if(!DeviceProxyPlugin)
	{
		cout<<"error opening library\n";
	}
	
	plugin_infocall ptr;
	ptr = (plugin_infocall) dlsym(DeviceProxyPlugin, "get_deviceproxy_plugin");
	
	DeviceProxy* plugin;
	plugin = (*(ptr))(cfg);
	return (void *) plugin;
}

void PluginManager::destroy_plugins()
{	
	for(vector<void*>::iterator it = handleList.begin(); it != handleList.end(); ++it)
	{
		dlclose(*it);
	}
}

