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

/*
 * This file is based on http://www.linux-usb.org/gadget/usb.c
 * That file lacks any copyright information - but it belongs to someone
 * probably David Brownell - so thank you very much to him too!
 */

/* $(CROSS_COMPILE)cc -Wall -g -o proxy proxy.c usbstring.c -lpthread */

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include "descriptors.h"

static int debug;

/*-------------------------------------------------------------------------*/

/* gadgetfs currently has no chunking (or O_DIRECT/zerocopy) support
 * to turn big requests into lots of smaller ones; so this is "small".
 */
#define	USB_BUFSIZE	(7 * 1024)

static libusb_context *ctx;
static libusb_device_handle* devh;

/*-------------------------------------------------------------------------*/

void usage(char *arg) {
	fprintf(stderr, "usage: %s [-v vendorId] [-p productId] [-d (debug)]\n",
			arg);
	
}

int open_single_nonhub_device() {
	libusb_device **list;
	libusb_device *found = NULL;

	ssize_t cnt=libusb_get_device_list(ctx,&list);
	if (cnt<0) {fprintf(stderr,"Error %d retrieving device list.",cnt);return cnt;}
	
	ssize_t i;
	
	struct libusb_device_descriptor desc;
	int device_count=0;
	int rc;
	
	for(i = 0; i < cnt; i++){
		libusb_device *device = list[i];
		rc = libusb_get_device_descriptor(device,&desc);
		if (rc<0) {fprintf(stderr,"Error %d retrieving device descriptor.",rc);return rc;}
		if (desc.bDeviceClass!=LIBUSB_CLASS_HUB) {
			device_count++;
			found=device;
		}
	}
	if (device_count==1) {	
		rc=libusb_open(found,&devh);
		if (rc<0) {fprintf(stderr,"Error %d opening device handle.",rc);return rc;}
	}
	libusb_free_device_list(list,1);
	return device_count;
}

void print_device_info() {
	uint8_t str_mfr[200];
	uint8_t str_prd[200];
	struct libusb_device_descriptor desc;
	libusb_device* dev=libusb_get_device(devh);
	int rc=libusb_get_device_descriptor (dev,&desc);
	if (rc<0) {fprintf(stderr,"Error %d retrieving device descriptor.",rc);return;}
	rc=libusb_get_string_descriptor_ascii(devh,desc.iManufacturer,str_mfr,200);
	if (rc<0) {fprintf(stderr,"Error %d retrieving string descriptor.",rc);return;}
	rc=libusb_get_string_descriptor_ascii(devh,desc.iProduct,str_prd,200);
	if (rc<0) {fprintf(stderr,"Error %d retrieving string descriptor.",rc);return;}
	fprintf(stdout,"%04x:%04x %s - %s\n",desc.idVendor,desc.idProduct,str_mfr,str_prd);
}

int main(int argc, char **argv)
{
	int c;
	char* end;
	__u16 vendorId=0, productId=0;

	while ((c = getopt (argc, argv, "p:v:d")) != EOF) {
		switch (c) {
		case 'p':
			productId = strtol(optarg, &end, 16);
			continue;
		case 'v':
			vendorId = strtol(optarg, &end, 16);
			continue;
		case 'd':		/* verbose */
			debug++;
			continue;
		}
		usage(argv[0]);
		return 1;
	}

	if ((!productId && vendorId) || (productId && !vendorId)) {
		fprintf(stderr, "If you supply Vendor ID or Product ID then you must supply both of them.\n");
		usage(argv[0]);
		return 1;
	}

	//we have enough info to search now

	int rc=libusb_init(&ctx);
	if (rc<0) {fprintf(stderr,"Error %d initializing libusb.",rc);return rc;}
	libusb_set_debug(ctx, debug);

	if (productId && vendorId) {
		devh=libusb_open_device_with_vid_pid(ctx,vendorId,productId);
		if (devh==NULL) {
			fprintf(stderr,"Device not found for Vendor ID [%04x] and Product ID [%04x].\n",vendorId,productId);
			return 1;
		}
	} 
	if (!productId && !vendorId) {
		int found_device_count=open_single_nonhub_device();
		if (devh==NULL) {
			fprintf(stderr,"Device auto-detection failed, requires exactly one non-hub device, %d were found.",found_device_count);
			return 1;
		}
		//libusb_set_auto_detach_kernel_driver(devh,1);
	}

	if (debug) {print_device_info();}
	
	if (chdir("/gadget") < 0) {
		perror ("can't chdir /gadget");
		return 1;
	}

	printf("Calling init_device\n");
	//fd = init_device(vendorId, productId);
	//if (fd < 0)	return 1;

	//fprintf (stderr, "/gadget/%s ep0 configured\n", DEVNAME);
	fflush (stderr);

	//ep0_thread(&fd);
	return 0;
	libusb_exit(ctx);
}
