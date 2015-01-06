/*
 * This file is part of USBProxy.
 */

#ifndef RELAYREADER_H_
#define RELAYREADER_H_

#include <atomic>

#include <linux/types.h>
#include <mqueue.h>

class Proxy;
class HostProxy;
class Endpoint;

class RelayReader {
private:
	std::atomic_bool _please_stop;
	mqd_t sendQueue;
	mqd_t recvQueue;
	Proxy* proxy;
	HostProxy* hostProxy;
	__u8 endpoint;
	__u8 attributes;
	__u16 maxPacketSize;

public:
	RelayReader(Endpoint* _endpoint,Proxy* _proxy,mqd_t _sendQueue);
	RelayReader(Endpoint* _endpoint,HostProxy* _proxy,mqd_t _sendQueue,mqd_t _recvQueue);
	virtual ~RelayReader();

	void relay_read();
	void relay_read_setup();

	void please_stop(void) {
		_please_stop = true;
	}
};

#endif /* RELAYREADER_H_ */
