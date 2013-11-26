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
#include <stdio.h>
#include <sys/mount.h>
#include <fcntl.h>
#include "TRACE.h"

static char *gadgetfs_path;

/* Mount gadgetfs filesystem in a temporary directory */
int mount_gadget() {
	int status;
	char mount_template[] = "/tmp/gadget-XXXXXX";
	
	
	gadgetfs_path = malloc(sizeof(mount_template));
	memcpy(gadgetfs_path, mount_template, sizeof(mount_template));
	
	gadgetfs_path = mkdtemp(gadgetfs_path);
	fprintf(stderr, "Made directory %s for gadget\n", gadgetfs_path);
	
	status = mount(NULL, gadgetfs_path, "gadgetfs", 0, "");
	TRACE1(status)
	
	return 0;
}

/* Unmount gadgetfs filesystem and remove temporary directory */
int unmount_gadget() {
	int status;
	status = umount2(gadgetfs_path, 0);
	TRACE1(status)
	status = rmdir (gadgetfs_path);
	TRACE1(status)
	free(gadgetfs_path);
	return 0;
}

/* Find the appropriate gadget file on the GadgetFS filesystem */
int find_gadget() {
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
		"musb-hdrc",
		NULL
	};

	dir = opendir(gadgetfs_path);
	if (!dir)
		return -1;

	entry = malloc(offsetof(struct dirent, d_name)
				   + pathconf(gadgetfs_path, _PC_NAME_MAX)
				   + 1);

	fprintf(stderr,"searching in [%s]\n",gadgetfs_path);

	if (!entry) {
		closedir (dir);
		return -1;
	}

	while(1) {
		if (readdir_r(dir, entry, &result) < 0) break;
		if (!result) {
			fprintf(stderr,"%s device file not found.\n", gadgetfs_path);
			break;
		}
		for (i = 0; devices[i] && strcmp (devices[i], entry->d_name); i++)
			;
		if (devices[i]) {filename = devices[i] ;break;}
	}

	free(entry);
	closedir(dir);
	
	if (filename == NULL) {
		fprintf(stderr, "Error, unable to find gadget file.\n");
		return -1;
	}
	
	char path[256]={0x0};
	strcat(path, gadgetfs_path);
	strcat(path, "/");
	strcat(path, filename);
	
	return open(path, O_RDWR);
}
