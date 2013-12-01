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
#include "TRACE.h"
#include "USBManager.h"
#include "USBDeviceProxy_LibUSB.h"
#include "USBHostProxy_GadgetFS.h"
#include "USBInjector_UDP.h"
#include "USBPacketFilter_ROT13.h"
#include "USBPacketFilter_KeyLogger.h"

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
	struct sigaction action;
	switch (signum) {
		case SIGTERM:
			printf("Received SIGTERM, stopping relaying...\n");
			if (manager) {manager->stop_relaying();}
			printf("Exiting\n");
			memset(&action, 0, sizeof(struct sigaction));
			action.sa_handler = SIG_DFL;
			sigaction(SIGTERM, &action, NULL);
			break;
		case SIGINT:
			printf("Received SIGINT, stopping relaying...\n");
			if (manager) {manager->stop_relaying();}
			printf("Exiting\n");
			memset(&action, 0, sizeof(struct sigaction));
			action.sa_handler = SIG_DFL;
			sigaction(SIGINT, &action, NULL);
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
	
	while ((c = getopt (argc, argv, "p:v:dh")) != EOF) {
		switch (c) {
		case 'p':
			productId = strtol(optarg, &end, 16);
			break;
		case 'v':
			vendorId = strtol(optarg, &end, 16);
			break;
		case 'd':		/* verbose */
			debug++;
			break;
		case 'h':
		default:
			usage(argv[0]);
			return 1;
		}
	}

	USBDeviceProxy_LibUSB::debugLevel=1;

	USBDeviceProxy* device_proxy=(USBDeviceProxy *)new USBDeviceProxy_LibUSB(vendorId,productId);
	USBHostProxy* host_proxy=(USBHostProxy* )new USBHostProxy_GadgetFS(1);
	manager=new USBManager(device_proxy,host_proxy);

	USBPacketFilter_streamlog* logfilter=new USBPacketFilter_streamlog(stderr);
	//USBPacketFilter_KeyLogger* keyfilter=new USBPacketFilter_KeyLogger(stderr);
	//USBPacketFilter_ROT13* rotfilter=new USBPacketFilter_ROT13();
	USBInjector_UDP* udpinjector=new USBInjector_UDP(12345);

	manager->add_filter(logfilter);
	//manager->add_filter(rotfilter);
	//manager->add_filter(keyfilter);
	manager->add_injector(udpinjector);

	manager->start_relaying();

	while (manager->get_status()==USBM_RELAYING) {sleep(1);}

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
