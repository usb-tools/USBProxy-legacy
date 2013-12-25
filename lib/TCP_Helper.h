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
 * TCP_Helper.h
 *
 * Created on: Dec 19, 2013
 */

#ifndef USBPROXY_TCP_HELPER_H
#define USBPROXY_TCP_HELPER_H

#include <stddef.h>
#include <linux/types.h>
#include <poll.h>

class TCP_Helper {
private:
	bool p_server;
	bool p_is_connected;
	__u8* ep_buf[32];
	bool ep_connect[32];
	int ep_socket[32];
	int ep_listener[32];
	struct pollfd spoll;
	char* p_address;
	int client_connect(int port,int timeout);
	int client_open_endpoints(__u8* eps,__u8 num_eps,int timeout);

	void server_listen(int port);
	int server_connect(int port,int timeout);
	int server_open_endpoints(__u8* eps,__u8 num_eps,int timeout);

public:
	static int debugLevel;
	TCP_Helper(const char* address=NULL);
	virtual ~TCP_Helper();

	int connect(int timeout);
	void disconnect();
	int open_endpoints(__u8* eps,__u8 num_eps,int timeout);
	void reset();
	bool is_connected();

	char* toString() {return (char *) "TCP network helper";}
};

#endif /* USBPROXY_TCP_HELPER_H */
