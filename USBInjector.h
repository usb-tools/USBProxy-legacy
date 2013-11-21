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
 * USBInjector.h
 *
 * Created on: Nov 12, 2013
 */
#ifndef USBINJECTOR_H_
#define USBINJECTOR_H_

#include "USBManager.h"
#include "USBPacket.h"

class USBManager;

class USBInjector {
private:
	USBManager* manager;
public:
	bool halt;

	USBInjector(USBManager* _manager);
	virtual ~USBInjector() {}
	virtual USBPacket* get_packets()=0;

	void listen();

	static void *listen_helper(void* context);

	virtual const char* toString() {return "Injector";}

};

#endif /* USBINJECTOR_H_ */
