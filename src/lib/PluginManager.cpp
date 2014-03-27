#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>

#include "PluginManager.h"


void PluginManager::loadFromDisk()
{
	std::vector<std::string>* filenames = getFilenames();

	for(std::vector<std::string>::iterator it = filenames->begin(); it != filenames->end(); ++it)
	{
		std::cout<<*it<<std::endl;	
		std::string file = *it;
		
		void* handle = dlopen(std::string("plugins/"+file).c_str(), RTLD_LAZY);
		if(!handle)
		{
			std::cout<<"error opening library\n";
		}
		void* ptr = dlsym(handle, "getPlugin");
		DeviceProxy* plugin = ((DeviceProxy* (*)())ptr)();
		DeviceProxyPlugins.push_back(plugin);
		handleList.push_back(handle);
	}
	delete filenames;
}

std::vector<std::string>* PluginManager::getFilenames()
{
	std::vector<std::string>* filenames = new std::vector<std::string>();

	DIR *dp;
	struct dirent *dirp;
	if((dp  = opendir("plugins/")) == NULL) 
	{
		std::cout << "Error opening " << "plugins/" << std::endl;
	}
	while ((dirp = readdir(dp)) != NULL) 
	{
		std::string filename = dirp->d_name;
		if(filename != "." &&  filename != "..")
		{
			filenames->push_back(std::string(dirp->d_name));
		}
	}
	closedir(dp);
	return filenames;

}

void PluginManager::initAllPlugins()
{
	for(std::vector<DeviceProxy*>::iterator it = DeviceProxyPlugins.begin(); it != DeviceProxyPlugins.end(); ++it)
	{
		DeviceProxy* p = (DeviceProxy*)*it;
		p->init_plugin();
	}
}

void PluginManager::loadAllPlugins()
{
	loadFromDisk();
	initAllPlugins();
}

void PluginManager::destroyAllPlugins()
{
	for(std::vector<DeviceProxy*>::iterator it = DeviceProxyPlugins.begin(); it != DeviceProxyPlugins.end(); ++it)
	{
		std::cout<<"deleting plugins\n";
		((DeviceProxy*)*it)->destroy_plugin();
		delete *it;
	}
	for(std::vector<void*>::iterator it = handleList.begin(); it != handleList.end(); ++it)
	{
		dlclose(*it);
	}

}

