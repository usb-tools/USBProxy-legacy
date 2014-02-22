/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
 *
 * This file is part of USBProxy.
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
 * InjectorUDPHID.h
 *
 * Created on: Feb 22, 2014
 */
#ifndef INJECTORUDPHID_H_
#define INJECTORUDPHID_H_

#include "Injector.h"
#include <sys/socket.h>
#include <poll.h>

#define UDP_BUFFER_SIZE 1472

class Injector_UDPHID: public Injector {
private:
	__u16 port;
	int sck;
	__u8* buf;
	struct pollfd spoll;
	__u8 reportBuffer[20];

protected:
	void start_injector();
	void stop_injector();
	int* get_pollable_fds();
	void full_pipe(Packet* p);

	void get_packets(Packet** packet,SetupPacket** setup,int timeout=500);

public:
	Injector_UDPHID(__u16 _port);
	virtual ~Injector_UDPHID();
	__u8* getReportBuffer() {return reportBuffer;}
};

#endif /* INJECTORUDPHID_H_ */
