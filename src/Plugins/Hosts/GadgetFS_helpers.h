/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
 * 
 * Based on libusb-gadget - Copyright 2009 Daiki Ueno <ueno@unixuser.org>
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
 * GadgetFS_helpers.c
 *
 * Created on: Nov 24, 2013
 */

#ifndef USBPROXY_GADGETFS_HELPERS_H
#define USBPROXY_GADGETFS_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

/* gadgetfs currently has no chunking (or O_DIRECT/zerocopy) support
 * to turn big requests into lots of smaller ones; so this is "small".
 */
#define	USB_BUFSIZE	(7 * 1024)

/* open the appropriate gadget file on the GadgetFS filesystem */
int open_gadget(const char *);

/* Find the appropriate gadget file on the GadgetFS filesystem */
const char * find_gadget_filename();

/* Find & open an endpoint file */
int open_endpoint(__u8 epAddress, const char * gadget_filename);

/* Mount gadgetfs filesystem in a temporary directory */
int mount_gadget();

/* Unmount gadgetfs filesystem and remove temporary directory */
int unmount_gadget();

void clean_tmp();

#ifdef __cplusplus
} // __cplusplus defined.
#endif

#endif /* USBPROXY_GADGETFS_HELPERS_H */
