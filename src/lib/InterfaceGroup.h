/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
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
 *
 * InterfaceGroup.h
 *
 * Created on: Nov 6, 2013
 */
#ifndef USBPROXY_INTERFACEGROUP_H
#define USBPROXY_INTERFACEGROUP_H

#include <linux/types.h>
#include <stdlib.h>
#include "DefinitionErrors.h"

class Device;
class Interface;

class InterfaceGroup {
	private:
		__u8 number;
		__u8 alternateCount;
		Interface** interfaces;

	public:
		int activeAlternateIndex;

		InterfaceGroup(__u8 number);
		~InterfaceGroup();
		size_t get_full_descriptor_length();
		void get_full_descriptor(__u8** p);
		void add_interface(Interface* interface);
		Interface* get_interface(__u8 alternate);
		void print(__u8 tabs=0);
		__u8 get_number();
		__u8 get_alternate_count();
		Interface* get_active_interface();
		const definition_error is_defined(__u8 configId);
};

#endif /* USBPROXY_INTERFACEGROUP_H */
