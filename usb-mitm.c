/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
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
#include "usb-mitm.h"

static int debug=0;
static int libusb_initialized=0;
static int exiting=0;
static int restarting=0;
static libusb_hotplug_callback_handle callback;
static libusb_device_handle* devh;
/*-------------------------------------------------------------------------*/

/* gadgetfs currently has no chunking (or O_DIRECT/zerocopy) support
 * to turn big requests into lots of smaller ones; so this is "small".
 */
#define	USB_BUFSIZE	(7 * 1024)

/*-------------------------------------------------------------------------*/

void usage(char *arg) {
	fprintf(stderr, "usage: %s [-v vendorId] [-p productId] [-d (debug)]\n",
			arg);
	
}

void cleanup(void) {
	if (libusb_initialized) {libusb_exit(NULL);}
	if (callback) {libusb_hotplug_deregister_callback(NULL,&callback);}
	fflush (stderr);
}

//sigterm: stop forwarding threads, and/or hotplug loop and exit
//sighup: reset forwarding threads, reset device and gadget
void handle_signal(int signum)
{
	switch (signum) {
		case SIGTERM:
			printf("Received SIGTERM, exiting...\n");
			exiting=1;
			break;
		case SIGINT:
			printf("Received SIGINT, exiting...\n");
			exiting=1;
			break;
		case SIGHUP:
			printf("Received SIGHUP, restarting...\n");
			restarting=1;
			break;
	}
}

static int LIBUSB_CALL hotplug_connect(struct libusb_context *ctx, struct libusb_device *dev,libusb_hotplug_event event, void *user_data) {
	if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED != event) {
		return 0;
	}
	printf("Device connected.\n");
	int rc=libusb_open(dev,&devh);
	if (rc) {fprintf(stderr,"Error %d opening device handle.\n",rc);return 0;}
	if (debug) {print_device_info(devh);}
	callback=NULL;
	return 1;
}

int register_connect_callback(__u16 vendorId,__u16 productId) {
	return libusb_hotplug_register_callback(NULL,LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED,0,vendorId?vendorId:LIBUSB_HOTPLUG_MATCH_ANY,productId?productId:LIBUSB_HOTPLUG_MATCH_ANY,LIBUSB_HOTPLUG_MATCH_ANY,hotplug_connect,NULL,&callback);
}

int main(int argc, char **argv)
{
	int c;
	char* end;
	__u16 vendorId=0, productId=0;
	
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = handle_signal;
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGHUP, &action, NULL);
	sigaction(SIGINT, &action, NULL);
	
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
		perror("If you supply Vendor ID or Product ID then you must supply both of them.\n");
		usage(argv[0]);
		return 1;
	}

	if (chdir("/gadget") < 0) {
		perror("can't chdir /gadget\n");
		return 1;
	}

	//we have enough info to search now
	int rc=libusb_init(NULL);
	if (rc) {fprintf(stderr,"Error %d initializing libusb.\n",rc);return rc;} else {libusb_initialized=1;}
	libusb_set_debug(NULL, debug);

	devh=get_device_handle(vendorId,productId);

	if (!devh) {
		//if we can't register the connect callback, no point in waiting
		rc=register_connect_callback(vendorId,productId);
		if (rc) {fprintf(stderr,"Error %d registering callback.\n",rc);exiting=1;} else {printf("Waiting for device connection...\n");}
	} else {
		//TODO: start relaying thread(s) for device, register disconnect handler
		if (debug) {print_device_info(devh);}
	}

	struct timeval tv;
	tv.tv_usec=100000;
	
	while (!exiting)
	{
		if (!devh) {
			rc=libusb_handle_events_timeout_completed(NULL,&tv,NULL);
			if (rc) {fprintf(stderr,"Error %d waiting for events.\n",rc);exiting=1;}
			//TODO: handle connection if devh valid now, also register disconnect handler
		} else {
			rc=libusb_handle_events_timeout_completed(NULL,&tv,NULL);
			//TODO: handle debh invalid now, register connect handler
		}
	}
	
	return 0;
	cleanup();
}
