/*
 * Copyright 2014 Dominic Spill
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

#ifndef USBPROXY_CONFIGPARSER_H
#define USBPROXY_CONFIGPARSER_H

#include <fstream>
#include <string>
#include <map>
#include <vector>

class ConfigParser {
private:
	std::ifstream configfile;
	std::map<std::string, std::vector<std::string>> vectors;
	std::map<std::string, std::string> config_map;
	std::map<std::string, void*> pointers;

public:
	static int debugLevel;
	ConfigParser();
	void parse_file(char* filename);
	
	void set(std::string key, std::string value);
	std::string get(std::string key);
	//int get_as_int(std::string key, int base=10);
	
	void add_to_vector(std::string key, std::string value);
	std::vector<std::string> get_vector(std::string key);
	
	void add_pointer(std::string key, void *value);
	void * get_pointer(std::string key);
};

#endif /* USBPROXY_CONFIGPARSER_H */
