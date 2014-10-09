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
#include "ConfigParser.h"

static int debug=0;

Manager* manager;

void usage(char *arg) {
	printf("usb-mitm - command line tool for controlling USBProxy\n");
	printf("Usage: %s [OPTIONS]\n", arg);
	printf("Options:\n");
	printf("\t-v <vendorId> VendorID of target device\n");
	printf("\t-p <productId> ProductID of target device\n");
	printf("\t-P <PluginName> Use PluginName (order is preserved)\n");
	printf("\t-D <DeviceProxy> Use DeviceProxy\n");
	printf("\t-H <HostProxy> Use HostProxy\n");
	printf("\t-d Enable debug messages (-dd for increased verbosity)\n");
	printf("\t-s Server mode, listen on port 10400\n");
	printf("\t-c <hostname | address> Client mode, connect to server at hostname or address\n");
	printf("\t-l Enable stream logger (logs to stderr)\n");
	printf("\t-i Enable UDP injector\n");
	printf("\t-x Enable Xbox360 UDPHID injector & filter\n");
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
			// modified 20140924 atsumi@aizulab.com
			// restart manager for handling reset bus.
			// begin
			fprintf(stderr, "Received SIGHUP, restarting relaying...\n");
			if (manager) {manager->stop_relaying();}
			if (manager) {manager->start_control_relaying();}
			// fprintf(stderr, "Received SIGHUP, restarting manager..\n");
			// if ( manager) {manager->set_status( USBM_RESET);}
			// end
			break;
	}
}

extern "C" int main(int argc, char **argv)
{
	int opt;
	char *host;
	bool client=false, server=false, device_set=false, host_set=false;
	fprintf(stderr,"SIGRTMIN: %d\n",SIGRTMIN);

	// int vendorId=LIBUSB_HOTPLUG_MATCH_ANY, productId=LIBUSB_HOTPLUG_MATCH_ANY;
	
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = handle_signal;
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGHUP, &action, NULL);
	sigaction(SIGINT, &action, NULL);
	
	ConfigParser *cfg = new ConfigParser();

	while ((opt = getopt (argc, argv, "v:p:P:D:H:dsc:C:lmikw:hx")) != EOF) {
		switch (opt) {
		case 'v':
			cfg->set("vendorId", optarg);
			break;
		case 'p':
			cfg->set("productId", optarg);
			break;
		case 'P':
			cfg->add_to_vector("Plugins", optarg);
			break;
		case 'D':
			cfg->set("DeviceProxy", optarg);
			device_set = true;
			break;
		case 'H':
			cfg->set("HostProxy", optarg);
			host_set = true;
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
		case 'C':
			cfg->parse_file(optarg);
			break;
		case 'l':
			cfg->add_to_vector("Plugins", "PacketFilter_StreamLog");
			cfg->add_pointer("PacketFilter_StreamLog::file", stderr);
			break;
		case 'm':
			// Will be added to both filter and injector lists
			cfg->add_to_vector("Plugins", "PacketFilter_MassStorage");
			break;
		case 'i':
			cfg->add_to_vector("Plugins", "Injector_UDP");
			cfg->set("Injector_UDP::Port", "12345");
			break;
		case 'k':
			cfg->add_to_vector("Plugins", "PacketFilter_KeyLogger");
			cfg->add_pointer("PacketFilter_KeyLogger::file", stderr);
			cfg->add_to_vector("Plugins", "PacketFilter_ROT13");
			break;
		case 'w':
			cfg->add_to_vector("Plugins", "PacketFilter_PcapLogger");
			cfg->set("PacketFilter_PcapLogger::Filename", optarg);
			break;
		case 'x':
			cfg->add_to_vector("Plugins", "Injector_UDPHID");
			cfg->set("Injector_UDP::port", "12345");
			cfg->add_to_vector("Plugins", "PacketFilter_UDPHID");
			break;
		case 'h':
		default:
			usage(argv[0]);
			return 1;
		}
	}

	if (client) {
		cfg->set("DeviceProxy", "DeviceProxy_LibUSB");
		cfg->set("HostProxy", "HostProxy_TCP");
		cfg->set("HostProxy_TCP::TCPAddress", host);
	} else if(server) {
		cfg->set("DeviceProxy", "DeviceProxy_TCP");
		cfg->set("HostProxy", "HostProxy_GadgetFS");
	} else {
		if(!device_set)
			cfg->set("DeviceProxy", "DeviceProxy_LibUSB");
		if(!host_set)
			cfg->set("HostProxy", "HostProxy_GadgetFS");
	}

	int status;
	do {
		manager=new Manager();
		manager->load_plugins(cfg);
		cfg->print_config();

		manager->start_control_relaying();
		while ( ( status = manager->get_status()) == USBM_RELAYING) {
			usleep(10000);
		}

		// Tidy up
		manager->stop_relaying();
		manager->cleanup();
		delete(manager);
	} while ( status == USBM_RESET);
	
	printf("done\n");
	return 0;
}
