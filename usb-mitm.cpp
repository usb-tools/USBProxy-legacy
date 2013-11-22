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

//FIXME make sure we are setting pointers to NULL after delete/freeing them if needed
#include "usb-mitm.h"
#include "USBManager.h"
#include "USBDeviceProxy_LibUSB.h"
#include "USBHostProxy_GadgetFS.h"
#include "USBInjector_UDP.h"

static int debug=0;

USBManager* manager;

void usage(char *arg) {
	fprintf(stderr, "usage: %s [-v vendorId] [-p productId] [-d (debug)]\n",
			arg);
	
}

void cleanup(void) {
}

//sigterm: stop forwarding threads, and/or hotplug loop and exit
//sighup: reset forwarding threads, reset device and gadget
void handle_signal(int signum)
{
	switch (signum) {
		case SIGTERM:
			printf("Received SIGTERM, stopping relaying...\n");
			if (manager) {manager->stop_relaying();}
			printf("Exiting\n");
			break;
		case SIGINT:
			printf("Received SIGINT, stopping relaying...\n");
			if (manager) {manager->stop_relaying();}
			printf("Exiting\n");
			break;
		case SIGHUP:
			printf("Received SIGHUP, restarting relaying...\n");
			if (manager) {manager->stop_relaying();}
			if (manager) {manager->start_relaying();}
			break;
	}
}

extern "C" int main(int argc, char **argv)
{
	int c;
	char* end;
	int vendorId=LIBUSB_HOTPLUG_MATCH_ANY, productId=LIBUSB_HOTPLUG_MATCH_ANY;
	
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

	if (chdir("/dev/gadget") < 0) {
		perror("can't chdir /dev/gadget\n");
		return 1;
	}


	USBDeviceProxy* device_proxy=(USBDeviceProxy *)new USBDeviceProxy_LibUSB(vendorId,productId);
	USBHostProxy* host_proxy=(USBHostProxy* )new USBHostProxy_GadgetFS("/dev/gadget");
	manager=new USBManager(device_proxy,host_proxy);

	USBPacketFilter_streamlog* logfilter=new USBPacketFilter_streamlog(stderr);
	USBInjector_UDP* udpinjector=new USBInjector_UDP(12345);

	manager->add_filter(logfilter);
	manager->add_injector(udpinjector);

	manager->start_relaying();

	int i;
	for (i=10;i>0 && manager->get_status()==USBM_RELAYING;i--) {printf("%d...\n",i);sleep(1);}

	manager->stop_relaying();

	delete(manager);
	manager=NULL;
	delete(host_proxy);
	host_proxy=NULL;
	delete(device_proxy);
	device_proxy=NULL;

	printf("done\n");
	return 0;
}
