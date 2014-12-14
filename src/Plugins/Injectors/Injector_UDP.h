/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_INJECTORUDP_H
#define USBPROXY_INJECTORUDP_H

#include "Injector.h"
#include <sys/socket.h>
#include <poll.h>

#define UDP_BUFFER_SIZE 1472

class Injector_UDP: public Injector {
private:
	__u16 port;
	int sck;
	__u8* buf;
	struct pollfd spoll;

protected:
	void start_injector();
	void stop_injector();
	int* get_pollable_fds();
	void full_pipe(Packet* p);

	void get_packets(Packet** packet,SetupPacket** setup,int timeout=500);

public:
	Injector_UDP(ConfigParser *cfg);
	virtual ~Injector_UDP();
};

#endif /* USBPROXY_INJECTORUDP_H */
