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
 * InjectorUDP.h
 *
 * Created on: Nov 22, 2013
 */
#ifndef USBPROXY_INJECTORUDP_H
#define USBPROXY_INJECTORUDP_H

#include "Injector.h"
#include <sys/socket.h>
#include <poll.h>

#define UDP_BUFFER_SIZE 1472

class Injector_UDP: public Injector {
private:
	__u16 port;
	int sck;
	__u8* buf;
	struct pollfd spoll;

protected:
	void start_injector();
	void stop_injector();

	void get_packets(Packet** packet,SetupPacket** setup,int timeout=500);

public:
	Injector_UDP(__u16 _port);
	virtual ~Injector_UDP();
};

#endif /* USBPROXY_INJECTORUDP_H */
