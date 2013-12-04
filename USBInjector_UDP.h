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
 * USBInjectorUDP.h
 *
 * Created on: Nov 22, 2013
 */
#ifndef USBINJECTORUDP_H_
#define USBINJECTORUDP_H_

#include "USBInjector.h"
#include <sys/socket.h>
#include <poll.h>

#define UDP_BUFFER_SIZE 1472

class USBInjector_UDP: public USBInjector {
private:
	__u16 port;
	int sck;
	__u8* buf;
	struct pollfd spoll;

protected:
	void start_injector();
	void stop_injector();
	USBPacket* get_packets();

public:
	USBInjector_UDP(__u16 _port);
	virtual ~USBInjector_UDP();
};

#endif /* USBINJECTORUDP_H_ */
