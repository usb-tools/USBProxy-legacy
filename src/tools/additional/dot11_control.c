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
#define TIMEOUT     1000

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
	devh = libusb_open_device_with_vid_pid(ctx, DOT11_VID, DOT11_PID);
	if(devh == NULL)
		fprintf(stderr, "Could not find device: %04x:%04x\n", DOT11_VID, DOT11_PID);
	else
		fprintf(stderr, "Found device: %04x:%04x\n", DOT11_VID, DOT11_PID);
	return devh;
}

int cmd_no_data(struct libusb_device_handle* devh, int vendor_req) {
	int r;
	r = libusb_control_transfer(devh, CTRL_OUT, vendor_req, 0, 0,
			NULL, 0, TIMEOUT);
	if (r < 0) {
		libusb_error_name(r);
		return r;
	}
	return 0;
}

int cmd_setter(struct libusb_device_handle* devh, int vendor_req, char *dataptr,
			   int length) {
	int r;
	r = libusb_control_transfer(devh, CTRL_OUT, vendor_req, 0, 0, dataptr,
								length, TIMEOUT);
	if (r < 0) {
		libusb_error_name(r);
		return r;
	}
	return 0;
}

void cmd_getter(struct libusb_device_handle* devh, int vendor_req,
				char *dataptr, int length) {
	int r;
	r = libusb_control_transfer(devh, CTRL_IN, vendor_req, 0, 0, dataptr,
								length, TIMEOUT);
	if (r < 0) {
		libusb_error_name(r);
	}
}

int cmd_open_inject(struct libusb_device_handle* devh) {
	return cmd_no_data(devh, DOT11_OPEN_INJECT);
}

int cmd_open_monitor(struct libusb_device_handle* devh) {
	return cmd_no_data(devh, DOT11_OPEN_MONITOR);
}

int cmd_open_injmon(struct libusb_device_handle* devh) {
	return cmd_no_data(devh, DOT11_OPEN_INJMON);
}

int cmd_set_timeout(struct libusb_device_handle* devh, int timeout) {
	int r;
	char dataptr[4];
	from_int(timeout, dataptr);
	return cmd_setter(devh, DOT11_SET_TIMEOUT, dataptr, 4);
}

int cmd_get_timeout(struct libusb_device_handle* devh) {
	int r;
	char dataptr[4];
	cmd_getter(devh, DOT11_GET_TIMEOUT, dataptr, 4);
	return to_int(dataptr);
}

int cmd_set_datalink(struct libusb_device_handle* devh, int datalink) {
	int r;
	char dataptr[4];
	from_int(datalink, dataptr);
	return cmd_setter(devh, DOT11_SET_DATALINK, dataptr, 4);
}

int cmd_get_datalink(struct libusb_device_handle* devh) {
	int r;
	char dataptr[4];
	cmd_getter(devh, DOT11_GET_DATALINK, dataptr, 4);
	return to_int(dataptr);
}

int cmd_set_channel(struct libusb_device_handle* devh, int channel) {
	int r;
	char dataptr[4];
	from_int(channel, dataptr);
	return cmd_setter(devh, DOT11_SET_CHANNEL, dataptr, 4);
}

int cmd_get_channel(struct libusb_device_handle* devh) {
	int r;
	char dataptr[4];
	cmd_getter(devh, DOT11_GET_CHANNEL, dataptr, 4);
	return to_int(dataptr);
}

int cmd_close_interface(struct libusb_device_handle* devh) {
	return cmd_no_data(devh, DOT11_CLOSE_INTERFACE);
}
