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
 * USBInjector.cpp
 *
 * Created on: Nov 12, 2013
 */
#include "USBInjector.h"

USBInjector::USBInjector(USBManager* _manager) {
	manager=_manager;
}

USBInjector::~USBInjector() {
	// TODO Auto-generated destructor stub
}

void USBInjector::listen() {
	while (!halt) {
		//TODO: 2 we also need to handle setup packets and getting the response back
		USBPacket* p=get_packets();
		if (p) {manager->inject_packet(p);}
	}
}
