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
 * RelayWriter.h
 *
 * Created on: Dec 8, 2013
 */
#ifndef RELAYWRITER_H_
#define RELAYWRITER_H_

#include <linux/types.h>
#include <mqueue.h>
#include <boost/atomic.hpp>

class PacketFilter;
class Proxy;
class Endpoint;

class RelayWriter {
private:
	__u8 haltSignal;
	mqd_t* inQueues;
	__u8 endpoint;
	__u8 attributes;
	__u16 maxPacketSize;

	Proxy* proxy;
	PacketFilter** filters;
	__u8 filterCount;
	__u8 queueCount;

public:
	RelayWriter(Endpoint* _endpoint,Proxy* _proxy,mqd_t _queue);
	virtual ~RelayWriter();

	void add_filter(PacketFilter* filter);
	void add_queue(mqd_t inQueue);

	void relay_write();
	void relay_write_valgrind();
	void relay_write_setup();
	void relay_write_setup_valgrind();

	void set_haltsignal(__u8 _haltSignal);
	static void* relay_write_helper(void* context);
};

#endif /* RELAYWRITER_H_ */
