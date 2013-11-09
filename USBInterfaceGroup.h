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

#include <stdlib.h>
#include "USBDevice.h"
#include "USBInterface.h"

class USBDevice;
class USBInterface;

class USBInterfaceGroup {
	private:
		__u8 number;
		__u8 alternateCount;
		USBInterface** interfaces;
		//TODO: USBDevice (and set upon creation)

	public:
		int activeAlternateIndex=-1;

		USBInterfaceGroup(__u8 number);
		~USBInterfaceGroup();
		size_t get_full_descriptor_length();
		void get_full_descriptor(__u8** p);
		void add_interface(USBInterface* interface);
		USBInterface* get_interface(__u8 alternate);
		void print(__u8 tabs=0);
<<<<<<< HEAD
		void set_usb_device(USBDevice* _device);
		__u8 get_alternate_count();
		USBInterface* get_active_interface();
=======
>>>>>>> 7a7b72cec231ba1cbb14b165e6b3ab4c5721b057
};

#endif /* USBINTERFACEGROUP_H_ */
