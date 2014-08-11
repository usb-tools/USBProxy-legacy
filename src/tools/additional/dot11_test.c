/*
 * Copyright 2014 Dominic Spill
 * Copyright 2014 Mike Kershaw / dragorn
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
 */

#include "dot11_control.h"
#include <stdio.h>

int main(int argc, char** argv) {
	char *x;
	struct libusb_device_handle *devh = find_dot11_device();
	int timeout = 10;
	printf("Setting timeout=%d\n", timeout);
	cmd_set_timeout(devh, 10);
	printf("Timeout set\n");
	timeout = cmd_get_timeout(devh);
	printf("Timeout=%d\n", timeout);
	x = cmd_get_capiface(devh);
	printf("Opened capture inteface (%s)\n", x);
	cmd_rx_data(devh);
	
}
