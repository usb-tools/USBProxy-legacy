/*
 * Copyright 2014 Dominic Spill
 *
 * This file is part of USBProcy.
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
 */

#include "Dot11_Interface.h"
#include "dot11_control.h"
#include <stdio.h>

#define DATA_IN     (0x82 | LIBUSB_ENDPOINT_IN)
#define DATA_OUT    (0x05 | LIBUSB_ENDPOINT_OUT)
#define CTRL_IN     (LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN)
#define CTRL_OUT    (LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT)
#define TIMEOUT     20000

int to_int(char *dataptr) {
	return dataptr[3] << 24 | dataptr[2] << 16 | dataptr[1] << 8 | dataptr[0];
}

void from_int(int value, char *dataptr) {
	dataptr[0] = value & 0xff;
	value >>= 8;
	dataptr[1] = value & 0xff;
	value >>= 8;
	dataptr[2] = value & 0xff;
	value >>= 8;
	dataptr[3] = value & 0xff;
}

struct libusb_device_handle* find_dot11_device()
{
	struct libusb_context *ctx = NULL;
	struct libusb_device_handle *devh = NULL;
	struct libusb_device **usb_list = NULL;
	libusb_init(&ctx);
	devh = libusb_open_device_with_vid_pid(ctx, 0xffff, 0x0005);
	if(devh == NULL)
		fprintf(stderr, "Could not find device: %04x:%04x\n", DOT11_VID, DOT11_PID);
	else
		fprintf(stderr, "Found device: %04x:%04x\n", DOT11_VID, DOT11_PID);
	return devh;
}

int cmd_open_inject(struct libusb_device_handle* devh) {
	int r;
	r = libusb_control_transfer(devh, CTRL_OUT, DOT11_OPEN_INJECT, 0, 0,
			NULL, 0, 1000);
	if (r < 0) {
		libusb_error_name(r);
		return r;
	}
	return 0;
}

int cmd_open_monitor(struct libusb_device_handle* devh) {
	int r;
	r = libusb_control_transfer(devh, CTRL_OUT, DOT11_OPEN_MONITOR, 0, 0,
			NULL, 0, 1000);
	if (r < 0) {
		libusb_error_name(r);
		return r;
	}
	return 0;
}

int cmd_open_injmon(struct libusb_device_handle* devh) {
	int r;
	r = libusb_control_transfer(devh, CTRL_OUT, DOT11_OPEN_INJMON, 0, 0,
			NULL, 0, 1000);
	if (r < 0) {
		libusb_error_name(r);
		return r;
	}
	return 0;
}

int cmd_set_timeout(struct libusb_device_handle* devh, int timeout) {
	int r;
	char dataptr[4];
	from_int(timeout, dataptr);
	r = libusb_control_transfer(devh, CTRL_OUT, DOT11_SET_TIMEOUT, 0, 0,
			dataptr, 4, 1000);
	if (r < 0) {
		libusb_error_name(r);
		return r;
	}
	return 0;
}

int cmd_get_timeout(struct libusb_device_handle* devh) {
	int r;
	char dataptr[4];
	r = libusb_control_transfer(devh, CTRL_IN, DOT11_GET_TIMEOUT, 0, 0,
			dataptr, 4, 1000);
	if (r < 0) {
		libusb_error_name(r);
		return r;
	}
	return to_int(dataptr);
}
