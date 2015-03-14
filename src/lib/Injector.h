/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_INJECTOR_H
#define USBPROXY_INJECTOR_H

#include <atomic>

#include <poll.h>
#include <mqueue.h>

#include "Plugins.h"
#include "Packet.h"
#include "Criteria.h"
#include "ConfigParser.h"

class Injector {
private:
	mqd_t outQueues[16],inQueues[16];
	int outPollIndex[16],inPollIndex[16];
	std::atomic_bool _please_stop;

protected:
	virtual void get_packets(Packet** packet,SetupPacket** setup,int timeout=500)=0;
	virtual void start_injector() {}
	virtual void stop_injector() {}
	virtual void setup_ack() {}
	virtual void setup_stall() {}
	virtual void setup_data(__u8* buf, int length) {}
	virtual int* get_pollable_fds() {return NULL;}
	virtual void full_pipe(Packet* p) {}
	virtual void full_pipe(SetupPacket* p) {}

public:
	static const __u8 plugin_type=PLUGIN_INJECTOR;
	struct criteria_endpoint endpoint;
	struct criteria_interface interface;
	struct criteria_configuration configuration;
	struct criteria_device device;

	Injector();
	virtual ~Injector() {}

	void please_stop(void) {
		_please_stop = true;
	}

	void set_queue(__u8 epAddress,mqd_t queue);

	void listen();

	virtual const char* toString() {return "Injector";}

};

extern "C" {
	Injector *get_injector_plugin();
}
#endif /* USBPROXY_INJECTOR_H */
