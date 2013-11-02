/*
 * Copyright 2013 Dominic Spill
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
 */

#include <linux/types.h>
#include <linux/usb/gadgetfs.h>
#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <memory.h>
#include <asm/byteorder.h>
#include <stdlib.h>

struct endpoint_item {
	char desc[7];
	struct endpoint_item *next;
};

/*-------------------------------------------------------------------------*/

/* Copy config from given vId/pId device */
int clone_descriptors(libusb_device_handle* devh, char *buf);
