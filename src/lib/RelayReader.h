/*
 * This file is part of USBProxy.
 */

#ifndef RELAYREADER_H_
#define RELAYREADER_H_

#include <atomic>

#include <linux/types.h>

class Proxy;
class HostProxy;
class Endpoint;

class RelayReader {
private:
	std::atomic_bool _please_stop;
	PacketQueue* _sendQueue;
	PacketQueue* _recvQueue;
	Proxy* proxy;
	HostProxy* hostProxy;
	__u8 endpoint;
	__u8 attributes;
	__u16 maxPacketSize;

public:
	RelayReader(Endpoint* _endpoint,Proxy* _proxy, PacketQueue& sendQueue);
	RelayReader(Endpoint* _endpoint,HostProxy* _proxy, PacketQueue& sendQueue, PacketQueue& recvQueue);
	virtual ~RelayReader();

	void relay_read();
	void relay_read_setup();

	void please_stop(void) {
		_please_stop = true;
		if (_recvQueue)
			_recvQueue->enqueue(PacketPtr());
	}
};

#endif /* RELAYREADER_H_ */
