/*
 * This file is part of USBProxy.
 */

#ifndef INJECTORUDPHID_H_
#define INJECTORUDPHID_H_

#include "Injector.h"
#include <sys/socket.h>
#include <poll.h>

#define UDP_BUFFER_SIZE 1472

class Injector_UDPHID: public Injector {
private:
	__u16 port;
	int sck;
	__u8* buf;
	struct pollfd spoll;
	__u8 reportBuffer[20];

protected:
	void start_injector();
	void stop_injector();
	int* get_pollable_fds();
	void full_pipe(Packet* p);

	void get_packets(Packet** packet,SetupPacket** setup,int timeout=500);

public:
	Injector_UDPHID(ConfigParser *cfg);
	virtual ~Injector_UDPHID();
	__u8* getReportBuffer() {return reportBuffer;}
};

#endif /* INJECTORUDPHID_H_ */
