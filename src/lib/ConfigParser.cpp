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
#include <getopt.h>
#include <algorithm>

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

std::string StrLower(std::string instr) {
	std::transform(instr.begin(), instr.end(), instr.begin(),
				   [](unsigned char c) { return std::tolower(c); });
	return instr;
}

int ConfigParser::debugLevel = 1;

ConfigParser::ConfigParser() {
}

/* Very much inspired by Kismet's config parser */
void ConfigParser::parse_file(const char* configfilename) {
	configfile.open(filename, std::ifstream::in);
	if (!configfile) {
		fprintf(stderr, "ERROR: Reading config file '%s': %d (%s)\n", filename,
				errno, strerror(errno));
		return;
	}
	if(debugLevel)
		fprintf(stderr, "Reading confilg file\n");

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
		fprintf(stderr, "Storing %s\n", key.c_str());
	config_map[StrLower(key)] = value;
}

std::string ConfigParser::get(std::string key) {
    // Empty string to return
    std::string errstr;

    std::map<std::string, std::string>::iterator cmitr = config_map.find(StrLower(key));
    // No such key
    if (cmitr == config_map.end())
        return errstr;

    return cmitr->second;
}

int ConfigParser::get_as_int(std::string key) {
    std::map<std::string, std::string>::iterator cmitr = config_map.find(StrLower(key));
    // No such key
    if (cmitr == config_map.end()) {
		fprintf(stderr, "key not found\n");
        return 0;
    }

    return atoi(cmitr->second.c_str());
}
