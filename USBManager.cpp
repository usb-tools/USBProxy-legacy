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
 * USBManager.cpp
 *
 * Created on: Nov 12, 2013
 */
#include "USBManager.h"

USBManager::USBManager() {
	// TODO Auto-generated constructor stub

}

USBManager::~USBManager() {
	// TODO Auto-generated destructor stub
}

int USBManager::inject_packet(USBPacket *packet) {
	//TODO 1 stub
}

int USBManager::inject_setup_in(usb_ctrlrequest request,__u8** data,__u16 *transferred, bool filter) {
	//TODO 1 stub
}

int USBManager::inject_setup_out(usb_ctrlrequest request,__u8* data,bool filter) {
	//TODO 1 stub
}
