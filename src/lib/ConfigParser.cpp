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

#include "ConfigParser.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

/* Taken from Kismet utils.cc */
std::string StrStrip(std::string in_str) {
    std::string temp;
    unsigned int start, end, x;

    start = 0;
    end = in_str.length();

    if (in_str[0] == '\n')
        return "";

    for (x = 0; x < in_str.length(); x++) {
        if (in_str[x] != ' ' && in_str[x] != '\t') {
            start = x;
            break;
        }
    }
    for (x = in_str.length(); x > 1; ) {
		x--;
        if (in_str[x] != ' ' && in_str[x] != '\t' && in_str[x] != '\n') {
            end = x;
            break;
        }
    }

    return in_str.substr(start, end-start+1);
}

int ConfigParser::debugLevel = 1;

ConfigParser::ConfigParser() {
}

/* Very much inspired by Kismet's config parser */
void ConfigParser::parse_file(char* filename) {
	if(debugLevel)
		fprintf(stderr, "Reading confilg file: %s\n", filename);

	std::ifstream configfile;
	configfile.open(filename, std::ifstream::in);
	if (!configfile) {
		fprintf(stderr, "ERROR: Reading config file '%s': %d (%s)\n", filename,
				errno, strerror(errno));
		return;
	}

	std::string confline;
	while (!configfile.eof()) {
		std::getline(configfile, confline);

		std::string parsestr = StrStrip(confline);
		std::string key, value;

		if (parsestr.length() == 0)
			continue;
		if (parsestr[0] == '#')
			continue;

		unsigned int eq;
		if ((eq = parsestr.find("=")) > parsestr.length()) {
			key = parsestr;
			value = "";
		} else {
			key = StrStrip(parsestr.substr(0, eq));
			value = StrStrip(parsestr.substr(eq+1, parsestr.length()));
			set(key, value);
		}
	}
	configfile.close();
}

void ConfigParser::set(std::string key, std::string value) {
	if(debugLevel)
		fprintf(stderr, "CP: String %s = %s\n", key.c_str(), value.c_str());
	strings[key] = value;
}

std::string ConfigParser::get(std::string key) {
    // Empty string to return
    std::string errstr;

    std::map<std::string, std::string>::iterator cmitr = strings.find(key);
    // No such key
    if (cmitr == strings.end())
        return errstr;

    return cmitr->second;
}

void ConfigParser::add_to_vector(std::string key, std::string value) {
	if(debugLevel)
		fprintf(stderr, "CP: Vector %s\n", key.c_str());
	std::map<std::string, std::vector<std::string>>::iterator vitr = vectors.find(key);
	// No such key
	if (vitr == vectors.end()) {
		std::vector<std::string> vec;
		vec.push_back(value);
		vectors[key] = vec;
	} else
		vitr->second.push_back(value);
}

std::vector<std::string> ConfigParser::get_vector(std::string key) {
	std::vector<std::string> errvec;
	std::map<std::string, std::vector<std::string>>::iterator vitr = vectors.find(key);
	// No such key
	if (vitr == vectors.end())
		return errvec;

	return vitr->second;
}

void ConfigParser::add_pointer(std::string key, void *value) {
	if(debugLevel)
		fprintf(stderr, "CP: Pointer %s\n", key.c_str());
	pointers[key] = value;
}

void *ConfigParser::get_pointer(std::string key) {
    std::map<std::string, void*>::iterator pitr = pointers.find(key);
    // No such key
    if (pitr == pointers.end())
        return NULL;

    return pitr->second;
}

/* Iterate over strings/vectors/pointers to print config */
void ConfigParser::print_config() {
	std::clog << "Printing Config data\n";
	std::clog << "\tStrings: " << strings.size() << "\n";
	for(std::map<std::string, std::string>::iterator it = strings.begin();
		it != strings.end(); ++it) {
		std::clog << "\t\t" << it->first << ": " << it->second << "\n";
	}
	
	std::clog << "\tVectors: " << vectors.size() << "\n";
	for(std::map<std::string, std::vector<std::string>>::iterator it = vectors.begin();
		it != vectors.end(); ++it) {
		std::clog << "\t\t" << it->first << ":\n";
		for(std::vector<std::string>::iterator itv = it->second.begin();
			itv != it->second.end(); ++itv) {
			std::clog << "\t\t\t" << *itv << "\n";
		}
	}
	
	std::clog << "Pointer: " << pointers.size() << "\n";
	for(std::map<std::string, void*>::iterator it = pointers.begin();
		it != pointers.end(); ++it) {
		std::clog << "\t\t" << it->first << ": " << it->second << "\n";
	}
}
