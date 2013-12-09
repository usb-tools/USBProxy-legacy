/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
 *
 * This file is part of USBProxy.
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
 * mqueue_helpers.c
 *
 * Created on: Dec 9, 2013
 */

#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <stdio.h>
#include <sys/mount.h>
#include <dirent.h>
#include <mntent.h>
#include <mqueue.h>
#include "mqueue_helpers.h"

static char* mqueue_path;

int mount_mqueue() {
	char** mountDirs=NULL;
	int mountCount=0;

	FILE* mtab=setmntent("/etc/mtab","r");
	struct mntent* m;
	struct mntent mnt;
	char strings[4096];
	while ((m=getmntent_r(mtab,&mnt,strings,sizeof(strings)))) {
	    if (strcmp(mnt.mnt_type,"mqueue")==0 && strcmp(mnt.mnt_fsname,"USBProxy")==0) {
	    	mountCount++;
	    	if (mountDirs) {
	    		mountDirs=realloc(mountDirs,sizeof(char*)*mountCount);
	    	} else {
	    		mountDirs=malloc(sizeof(char*));
	    	}
	    	mountDirs[mountCount-1]=strdup(mnt.mnt_dir);
	    }
	}
	endmntent(mtab);

	int i;
	for (i=0;i<mountCount;i++) {
		umount2(mountDirs[i],0);
		rmdir(mountDirs[i]);
		free(mountDirs[i]);
	}
	free(mountDirs);

	int status;
	char mount_template[] = "/tmp/mqueue-XXXXXX";

	mqueue_path =(char*)malloc(sizeof(mount_template));
	memcpy(mqueue_path, mount_template, sizeof(mount_template));

	mqueue_path = mkdtemp(mqueue_path);
	fprintf(stderr, "Made directory %s for mqueue\n", mqueue_path);
	status = mount("USBProxy", mqueue_path, "mqueue", 0, "");
	if (status!=0) {fprintf(stderr,"Error mounting mqueue at [%s].\n",mqueue_path);return 1;}

	clean_mqueue();

	return 0;
}

/* Unmount gadgetfs filesystem and remove temporary directory */
int unmount_mqueue() {
	if (mqueue_path) {
		int status;
		status=umount2(mqueue_path,0);
		if (status!=0) {fprintf(stderr,"Error unmounting mqueue from [%s].\n",mqueue_path);}
		status=rmdir(mqueue_path);
		if (status!=0) {fprintf(stderr,"Error removing directory [%s].\n",mqueue_path);}
		free(mqueue_path);
		mqueue_path=NULL;
	}
	return 0;
}

int clean_mqueue() {
	int rc=mount_mqueue;
	if (rc) return rc;
	DIR *dir;
	struct dirent *entry;
	struct dirent *result;
	int i;
	char **rmQueues=NULL;
	int rmCount=0;

	dir = opendir(mqueue_path);
	if (!dir) return 1;

	entry = malloc(offsetof(struct dirent, d_name) + pathconf("/tmp", _PC_NAME_MAX) + 1);

	fprintf(stderr,"cleaning up %s\n",mqueue_path);

	if (!entry) {
		closedir (dir);
		return 1;
	}

	while(1) {
		if (readdir_r(dir, entry, &result) < 0) break;
		if (!result) {break;}
		//format is USBProxy-[0-9A-F]{2}-(EP|([0-9A-F]{2}))
		if (strlen(entry->d_name)==14 && strncmp(entry->d_name,"USBProxy-",9)==0) {
	    	rmCount++;
	    	if (rmQueues) {
	    		rmQueues=realloc(rmQueues,sizeof(char*)*rmCount);
	    	} else {
	    		rmQueues=malloc(sizeof(char*));
	    	}
	    	rmQueues[rmCount-1]=strdup(entry->d_name);
		}
	}
	free(entry);

	fprintf(stderr,"removing %d\n",rmCount);
	for (i=0;i<rmCount;i++) {
		char buf[20]={0x0};
		strcat(buf,"/");
		strcat(buf,rmQueues[i]);
		mq_unlink(buf);
		free(rmQueues[i]);
	}
	free(rmQueues);

	closedir(dir);

	return unmount_mqueue();
}

