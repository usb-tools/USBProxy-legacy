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


class TCP_Helper {
private:
	bool p_server;
	bool p_is_connected;

public:
	static int debugLevel;
	HostProxy_TCP(bool server);
	virtual ~HostProxy_TCP();

	int connect();
	void disconnect();
	void reset();
	bool is_connected();

	int open_endpoint(__u8 epndpoint);
	char* toString() {return (char *) "TCP network helper";}
};

#endif /* USBPROXY_TCP_HELPER_H */
