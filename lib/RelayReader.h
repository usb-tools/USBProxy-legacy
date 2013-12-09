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
 * RelayReader.h
 *
 * Created on: Dec 8, 2013
 */
#ifndef RELAYREADER_H_
#define RELAYREADER_H_

#include <linux/types.h>
#include <mqueue.h>
#include <boost/atomic.hpp>

class Proxy;
class Endpoint;

class RelayReader {
private:
	mqd_t outQueue;
	Proxy* proxy;
	__u8 endpoint;
	__u8 attributes;
	__u16 maxPacketSize;

public:
	boost::atomic_bool halt;

	RelayReader(Endpoint* _endpoint,Proxy* _proxy,mqd_t _queue);
	virtual ~RelayReader();
	void relay_read();
	static void* relay_read_helper(void* context);
};

#endif /* RELAYREADER_H_ */
