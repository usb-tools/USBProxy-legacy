/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
 *
 * This file is part of USB-MitM.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
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
 * USBInterfaceGroup.h
 *
 * Created on: Nov 6, 2013
 */
#ifndef USBINTERFACEGROUP_H_
#define USBINTERFACEGROUP_H_

#include "USBInterface.h"

class USBInterfaceGroup {
	private:
		__u8 number;
		__u8 alternateCount;
		USBInterface* activeInterface=NULL;
		USBInterface** interfaces;

	public:
		USBInterfaceGroup(__u8 number);
		~USBInterfaceGroup();
		void getFullDescriptor(__u8** p);
		void add_interface(USBInterface* interface);
		USBInterface* get_interface(__u8 alternate);
};

#endif /* USBINTERFACEGROUP_H_ */
