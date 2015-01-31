/*
 * This file is part of USBProxy.
 */

#include <dlfcn.h>
#include <sys/types.h>
#include <stdio.h>
#include <assert.h>

#include "Plugins.h"
#include "PluginManager.h"

// FIXME: replace all of this repetative code with something based on Templates

typedef DeviceProxy* (*device_plugin_getter)(ConfigParser *);
typedef HostProxy* (*host_plugin_getter)(ConfigParser *);
typedef PacketFilter* (*filter_plugin_getter)(ConfigParser *);
typedef Injector* (*injector_plugin_getter)(ConfigParser *);

struct RustPlugin {
};

typedef void (*handle_func)(Packet*);
typedef handle_func (*handle_func_getter)(ConfigParser*);
using namespace std;


class RustShim : public PacketFilter {
private:
	handle_func func;
public:
	RustShim(handle_func f) : func(f) {};
	virtual char* toString() { return (char*)"Rust Filter"; }
	void filter_packet(Packet* packet) {
		func(packet);
	}
};

void* PluginManager::load_shared_lib(std::string plugin_name) {
	std::string plugin_file = PLUGIN_PATH + plugin_name + ".so";
	void* plugin_lib = dlopen(plugin_file.c_str(), RTLD_LAZY);
	if(!plugin_lib)
		fprintf(stderr, "error opening library %s\n", plugin_file.c_str());

	return plugin_lib;
}

int PluginManager::load_plugins(ConfigParser *cfg)
{
	std::string plugin_file;
	int plugin_type;
	void *plugin_lib, *plugin_func;
	PacketFilter *filter;
	fprintf(stderr, "Loading plugins from %s\n", PLUGIN_PATH);

	device_plugin_getter dp_ptr;
	host_plugin_getter hp_ptr;
	filter_plugin_getter f_ptr;
	injector_plugin_getter i_ptr;

	// Device Proxy
	plugin_lib = load_shared_lib(cfg->get("DeviceProxy"));
	if(plugin_lib==NULL)
		return PLUGIN_MANAGER_CANNOT_FIND_FILE;
	plugin_func = dlsym(plugin_lib, "get_deviceproxy_plugin");
	handleList.push_back(plugin_func);
	dp_ptr = (device_plugin_getter) plugin_func;
	device_proxy = (*(dp_ptr))(cfg);

	// Host Proxy
	plugin_lib = load_shared_lib(cfg->get("HostProxy"));
	if(plugin_lib==NULL)
		return PLUGIN_MANAGER_CANNOT_FIND_FILE;
	plugin_func = dlsym(plugin_lib, "get_hostproxy_plugin");
	handleList.push_back(plugin_func);
	hp_ptr = (host_plugin_getter) plugin_func;
	host_proxy = (*(hp_ptr))(cfg);

	// Plugins
	std::vector<std::string> plugin_names = cfg->get_vector("Plugins");
	for(std::vector<std::string>::iterator it = plugin_names.begin();
		it != plugin_names.end(); ++it) {
		plugin_lib = load_shared_lib(*it);
		if(plugin_lib==NULL)
			return PLUGIN_MANAGER_CANNOT_FIND_FILE;
		plugin_type = *(int *) dlsym(plugin_lib, "plugin_type");
		int c_abi = *(int *) dlsym(plugin_lib, "c_abi");

		switch (plugin_type) {
			case PLUGIN_FILTER:
				plugin_func = dlsym(plugin_lib, "get_plugin");
				handleList.push_back(plugin_func);
				if (c_abi) {
					handle_func_getter getter = (handle_func_getter) plugin_func;
					handle_func f = ((*(getter))(cfg));
					filters.push_back(new RustShim(f));
				} else {
					f_ptr = (filter_plugin_getter) plugin_func;
					filters.push_back((*(f_ptr))(cfg));
				}
				break;
			case PLUGIN_INJECTOR:
				plugin_func = dlsym(plugin_lib, "get_plugin");
				handleList.push_back(plugin_func);
				i_ptr = (injector_plugin_getter) plugin_func;
				if (c_abi) {
					// Lol good luck
				} else {
					injectors.push_back((*(i_ptr))(cfg));
				}
				break;
			case (PLUGIN_FILTER|PLUGIN_INJECTOR):
				plugin_func = dlsym(plugin_lib, "get_plugin");
				handleList.push_back(plugin_func);
				f_ptr = (filter_plugin_getter) plugin_func;
				if (c_abi) {
					// Lol good luck
				} else {
					filter = (*(f_ptr))(cfg);
				}
				filters.push_back(filter);
				injectors.push_back(dynamic_cast<Injector*>(filter));
				break;
			default:
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

int main(int argc, char** argv) {
	// Load a plugin, then pass a dummy packet through it
	void* lib = dlopen(argv[1], RTLD_LAZY);
	assert(dlsym(lib, "c_abi"));
	void* plugin_func = dlsym(lib, "get_plugin");

	handle_func_getter getter = (handle_func_getter) plugin_func;
	handle_func f = ((*(getter))(NULL));
	PacketFilter *filter = new RustShim(f);

	__u8 _data[9] = {0x41, 0x41, 0, 0, 0, 0, 0, 0, 0};
	__u8 *data = (__u8*) &_data;
	Packet *packet = new Packet(0, data, 0, 8);

	filter->filter_packet(packet);
	printf("%s\n", data);
}
