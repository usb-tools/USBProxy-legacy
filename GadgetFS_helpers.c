/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
 * 
 * Based on libusb-gadget - Copyright 2009 Daiki Ueno <ueno@unixuser.org>
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
 *
 * GadgetFS_helpers.c
 *
 * Created on: Nov 24, 2013
 */

#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <stddef.h>

/* Find the appropriate gadget file on the GadgetFS filesystem */
const char *find_gadget(const char *path)
{
	const char *filename = NULL;
	DIR *dir;
	struct dirent *entry;
	struct dirent *result;
	int i;

	static const char *devices[] = {
		"dummy_udc",
		"net2280",
		"gfs_udc",
		"pxa2xx_udc",
		"goku_udc",
		"sh_udc",
		"omap_udc",
		"musb_hdrc",
		"at91_udc",
		"lh740x_udc",
		"atmel_usba_udc",
		NULL
	};

	dir = opendir(path);
	if (!dir)
		return NULL;

	entry = malloc(offsetof(struct dirent, d_name)
				   + pathconf(path, _PC_NAME_MAX)
				   + 1);

	if (entry) {
		while(1) {
			if (readdir_r(dir, entry, &result) > 0 && result) {
				for (i = 0; devices[i] && strcmp (devices[i], entry->d_name); i++)
					;
				filename = devices[i];
			}
		}
		free(entry);
	}

	closedir(dir);

	return filename;
}