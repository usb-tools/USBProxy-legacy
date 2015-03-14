/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_CONFIGPARSER_H
#define USBPROXY_CONFIGPARSER_H

#include <fstream>
#include <string>
#include <map>
#include <vector>

class ConfigParser {
private:
	std::map<std::string, std::string> strings;
	std::map<std::string, std::vector<std::string> > vectors;
	std::map<std::string, void*> pointers;

public:
	unsigned debugLevel;
	ConfigParser();
	void parse_file(char* filename);
	
	void set(std::string key, std::string value);
	std::string get(const std::string& key);
	
	void add_to_vector(std::string key, std::string value);
	std::vector<std::string> get_vector(std::string key);
	
	void add_pointer(std::string key, void *value);
	void *get_pointer(std::string key);
	
	void print_config();
};

#endif /* USBPROXY_CONFIGPARSER_H */
