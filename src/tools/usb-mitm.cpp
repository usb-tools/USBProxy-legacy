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
#include <memory.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include "TRACE.h"
#include "Manager.h"
#include "DeviceProxy_LibUSB.h"
#include "DeviceProxy_TCP.h"
#include "DeviceProxy_Loopback.h"
#include "Injector_UDP.h"
#include "HostProxy_GadgetFS.h"
#include "HostProxy_TCP.h"
#include "PacketFilter_PcapLogger.h"
#include "PacketFilter_KeyLogger.h"
#include "PacketFilter_ROT13.h"
#include "PacketFilter_StreamLog.h"
#include "PacketFilter_Python.h"


static int debug=0;

Manager* manager;

void usage(char *arg) {
	printf("usb-mitm - command line tool for controlling USBProxy\n");
	printf("Usage: %s [OPTIONS]\n", arg);
	printf("Options:\n");
	printf("\t-v <vendorId> VendorID of target device\n");
	printf("\t-p <productId> ProductID of target device\n");
	printf("\t-d Enable debug messages (-dd for increased verbosity)\n");
	printf("\t-s Server mode, listen on port 10400\n");
	printf("\t-c <hostname | address> Client mode, connect to server at hostname or address\n");
	printf("\t-l Enable stream logger (logs to stderr)\n");
	printf("\t-i Enable UDP injector\n");
	printf("\t-k Keylogger with ROT13 filter (for demo)\n");
	printf("\t-w <filename> Write to pcap file for viewing in Wireshark\n");
	printf("\t-h Display this message\n");
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
		case SIGINT:
			if(signum == SIGTERM)
				fprintf(stderr, "Received SIGTERM, stopping relaying...\n");
			else
				fprintf(stderr, "Received SIGINT, stopping relaying...\n");
			if (manager) {manager->stop_relaying();}
			fprintf(stderr, "Exiting\n");
			memset(&action, 0, sizeof(struct sigaction));
			action.sa_handler = SIG_DFL;
			sigaction(SIGTERM, &action, NULL);
			break;
		case SIGHUP:
			fprintf(stderr, "Received SIGHUP, restarting relaying...\n");
			if (manager) {manager->stop_relaying();}
			if (manager) {manager->start_control_relaying();}
			break;
	}
}

extern "C" int main(int argc, char **argv)
{
	int c;
	char* end, *host;
	bool client=false,server=false;
	fprintf(stderr,"SIGRTMIN: %d\n",SIGRTMIN);

	int vendorId=LIBUSB_HOTPLUG_MATCH_ANY, productId=LIBUSB_HOTPLUG_MATCH_ANY;
	
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = handle_signal;
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGHUP, &action, NULL);
	sigaction(SIGINT, &action, NULL);
	
	Injector_UDP* udpinjector;
	PacketFilter_StreamLog* logfilter;
	PacketFilter_ROT13* rotfilter;
	PacketFilter_KeyLogger* keyfilter;
	PacketFilter_PcapLogger* pcaplogger;
	
	manager=new Manager();

	while ((c = getopt (argc, argv, "v:p:dsc:likw:h")) != EOF) {
		switch (c) {
		case 'v':
			vendorId = strtol(optarg, &end, 16);
			break;
		case 'p':
			productId = strtol(optarg, &end, 16);
			break;
		case 'd':		/* verbose */
			debug++;
			break;
		case 's':
			server=true;
			break;
		case 'c':
			client=true;
			host = optarg;
			break;
		case 'l':
			logfilter=new PacketFilter_StreamLog(stderr);
			manager->add_filter(logfilter);
			break;
		case 'i':
			udpinjector=new Injector_UDP(12345);
			manager->add_injector(udpinjector);
			break;
		case 'k':
			rotfilter=new PacketFilter_ROT13();
			keyfilter=new PacketFilter_KeyLogger(stderr);
			manager->add_filter(rotfilter);
			manager->add_filter(keyfilter);
			break;
		case 'w':
			pcaplogger=new PacketFilter_PcapLogger(optarg);
			manager->add_filter(pcaplogger);
			break;
		case 'h':
		default:
			usage(argv[0]);
			return 1;
		}
	}

	DeviceProxy_LibUSB::debugLevel=debug;
	DeviceProxy_Loopback::debugLevel=debug;
	DeviceProxy_TCP::debugLevel=debug;
	HostProxy_TCP::debugLevel=debug;
	HostProxy_GadgetFS::debugLevel=debug;
	PacketFilter_Python::debugLevel=debug;
	
	DeviceProxy* device_proxy;
	HostProxy* host_proxy;

	if (client) {
		device_proxy=(DeviceProxy *)new DeviceProxy_LibUSB(vendorId,productId);
		host_proxy=(HostProxy* )new HostProxy_TCP(host);
	} else if(server) {
		device_proxy=(DeviceProxy *)new DeviceProxy_TCP();
		host_proxy=(HostProxy* )new HostProxy_GadgetFS();
	} else {
		device_proxy=(DeviceProxy *)new DeviceProxy_LibUSB(vendorId,productId);
		host_proxy=(HostProxy* )new HostProxy_GadgetFS();
	}
	manager->add_proxies(device_proxy,host_proxy);

	//PacketFilter_Python* pyexample=new PacketFilter_Python("example_filter");
	//manager->add_filter(pyexample);

	manager->start_control_relaying();

	while (manager->get_status()==USBM_RELAYING) {usleep(10000);}

	// Tidy up
	manager->stop_relaying();
	manager->cleanup();
	delete(manager);

	printf("done\n");
	return 0;
}
