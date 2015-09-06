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

#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <linux/types.h>
#include <mntent.h>
#include "TRACE.h"

static char *gadgetfs_path;

void clean_tmp() {
	DIR *dir;
	struct dirent *entry;
	struct dirent *result;
	int i;
	char **rmDirs=NULL;
	int rmCount=0;

	dir = opendir("/tmp");
	if (!dir) return;

	entry = malloc(offsetof(struct dirent, d_name) + fpathconf(dirfd(dir), _PC_NAME_MAX) + 1);

	fprintf(stderr,"cleaning up /tmp\n");

	if (!entry) {
		closedir (dir);
		return;
	}

	while(1) {
		if (readdir_r(dir, entry, &result)) break;
		if (!result) {break;}
		if (strlen(entry->d_name)==13 && strncmp(entry->d_name,"gadget-",7)==0) {
			rmCount++;
			if (rmDirs) {
				rmDirs=realloc(rmDirs,sizeof(char*)*rmCount);
			} else {
				rmDirs=malloc(sizeof(char*));
			}
			rmDirs[rmCount-1]=strdup(entry->d_name);
		}
	}
	free(entry);

	fprintf(stderr,"removing %d\n",rmCount);
	for (i=0;i<rmCount;i++) {
		char buf[20]={0x0};
		strcat(buf,"/tmp/");
		strcat(buf,rmDirs[i]);
		rmdir(buf);
		free(rmDirs[i]);
	}
	free(rmDirs);

	closedir(dir);
}

/* Mount gadgetfs filesystem in a temporary directory */
int mount_gadget() {
	char** mountDirs=NULL;
	char** mountNames=NULL;
	int mountCount=0;

	FILE* mtab=setmntent("/etc/mtab","r");
	struct mntent* m;
	struct mntent mnt;
	char strings[4096];
	while ((m=getmntent_r(mtab,&mnt,strings,sizeof(strings)))) {
	    if (strcmp(mnt.mnt_type,"gadgetfs")==0) {
	    	mountCount++;
	    	if (mountDirs) {
	    		mountDirs=realloc(mountDirs,sizeof(char*)*mountCount);
	    		mountNames=realloc(mountNames,sizeof(char*)*mountCount);
	    	} else {
	    		mountDirs=malloc(sizeof(char*));
	    		mountNames=malloc(sizeof(char*));
	    	}
	    	mountDirs[mountCount-1]=strdup(mnt.mnt_dir);
	    	mountNames[mountCount-1]=strdup(mnt.mnt_fsname);
	    }
	}
	endmntent(mtab);

	int i;
	for (i=0;i<mountCount;i++) {
		umount2(mountDirs[i],0);
	}

	for (i=0;i<mountCount;i++) {
		if (strcmp(mountNames[i],"USBProxy")!=0) {
			mount(mountNames[i],mountDirs[i],"gadgetfs",0,"");
		}
		free(mountNames[i]);
		free(mountDirs[i]);
	}
	free(mountNames);
	free(mountDirs);

	clean_tmp();

	int status;
	char mount_template[] = "/tmp/gadget-XXXXXX";

	gadgetfs_path = malloc(sizeof(mount_template));
	memcpy(gadgetfs_path, mount_template, sizeof(mount_template));

	gadgetfs_path = mkdtemp(gadgetfs_path);
	fprintf(stderr, "Made directory %s for gadget\n", gadgetfs_path);
	status = mount("USBProxy", gadgetfs_path, "gadgetfs", 0, "");
	if (status!=0) {fprintf(stderr,"Error mounting gadgetfs from [%s].\n",gadgetfs_path);fprintf(stderr, "Error code from mount is: [%s]\n", strerror(errno));return 1;}
	return 0;
}

/* Unmount gadgetfs filesystem and remove temporary directory */
int unmount_gadget() {
	if (gadgetfs_path) {
		int status;
		status=umount2(gadgetfs_path,0);
		if (status!=0) {fprintf(stderr,"Error unmounting gadgetfs from [%s].\n",gadgetfs_path);}
		status=rmdir (gadgetfs_path);
		if (status!=0) {fprintf(stderr,"Error removing directory [%s].\n",gadgetfs_path);}
		free(gadgetfs_path);
		gadgetfs_path=NULL;
	}
	return 0;
}

/* Find the appropriate gadget file on the GadgetFS filesystem */
int open_gadget(const char * gadget_filename ) {
	const char *filename = gadget_filename;

	if (filename == NULL) {
		fprintf(stderr, "Error, unable to find gadget file.\n");
		return -1;
	}

	char path[256];
	int ret;
	snprintf(path, sizeof(path), "%s/%s", gadgetfs_path, filename);

	ret = open(path, O_CLOEXEC | O_RDWR);

	return ret;
}

const char * find_gadget_filename()
{
	const char *filename = NULL;
	DIR *dir;
	struct dirent *entry;
	struct dirent *result;
	int i;

	static const char *devices[] = {
		"2184000.usb",
		"at91_udc",
		"atmel_usba_udc",
		"dummy_udc",
		"dwc_otg_pcd",
		"gfs_udc",
		"goku_udc",
		"lh740x_udc",
		"musb-hdrc",
		"musb_hdrc",
		"omap_udc",
		"net2280",
		"pxa2xx_udc",
		"sh_udc",
		"sw_usb_udc",
		"usb20_otg",
		NULL
	};

	// modified 20141001 atsumi@aizulab.com
	// gadgetfs_path is NULL when device reset.
	if ( gadgetfs_path == NULL) mount_gadget();

	dir = opendir(gadgetfs_path);
	if (!dir)
		return NULL;

	entry = malloc(offsetof(struct dirent, d_name)
				   + fpathconf(dirfd(dir), _PC_NAME_MAX)
				   + 1);

	fprintf(stderr,"searching in [%s]\n",gadgetfs_path);

	if (!entry) {
		closedir (dir);
		return NULL;
	}

	while(1) {
		if (readdir_r(dir, entry, &result)) break;
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
	
	return filename;
}

int open_endpoint(__u8 epAddress, const char * gadget_filename) {
	int number=epAddress&0xf;
	if (number==0) return -1;
	char* direction=NULL;

	if(!strcmp(gadget_filename, "sw_usb_udc")) {
		direction="-bulk";
	}
	else {
		if (number>12) {
			direction="";
		} else {
			direction=(epAddress&0x80)?"in":"out";
		}
	}

	char path[256];
	snprintf(path, sizeof(path), "%s/ep%d%s", gadgetfs_path, number,direction);

	return open(path, O_CLOEXEC | O_RDWR);
}
