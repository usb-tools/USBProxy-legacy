/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
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

#include "TRACE.h"
#include "Manager.h"
#include "DeviceProxy_LibUSB.h"
#include "Injector_UDP.h"
#include "HostProxy_GadgetFS.h"
#include "PacketFilter_PcapLogger.h"

static int debug=0;

Manager* manager;

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
			if (manager) {manager->start_control_relaying();}
			break;
	}
}

extern "C" int main(int argc, char **argv)
{
	int c;
	char* end;
	fprintf(stderr,"SIGRTMIN: %d\n",SIGRTMIN);

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

	DeviceProxy_LibUSB::debugLevel=1;

	DeviceProxy* device_proxy=(DeviceProxy *)new DeviceProxy_LibUSB(vendorId,productId);
	HostProxy* host_proxy=(HostProxy* )new HostProxy_GadgetFS(1);
	manager=new Manager(device_proxy,host_proxy);

	PacketFilter_streamlog* logfilter=new PacketFilter_streamlog(stderr);
	Injector_UDP* udpinjector=new Injector_UDP(12345);

	PacketFilter_PcapLogger* pcaplogger=new PacketFilter_PcapLogger("/tmp/usb.pcap");

	//manager->add_filter(logfilter);
	//manager->add_filter(rotfilter);
	//manager->add_filter(keyfilter);
	manager->add_injector(udpinjector);
	manager->add_filter(pcaplogger);

	manager->start_control_relaying();

	while (manager->get_status()==USBM_RELAYING) {usleep(10000);}

	manager->stop_relaying();

	delete(pcaplogger);
	pcaplogger=NULL;

	delete(logfilter);
	logfilter=NULL;

	delete(udpinjector);
	udpinjector=NULL;

	delete(manager);
	manager=NULL;
	delete(host_proxy);
	host_proxy=NULL;
	delete(device_proxy);
	device_proxy=NULL;

	printf("done\n");
	return 0;
}
