/*
 * This file is part of USBProxy.
 */

#ifndef RELAYWRITER_H_
#define RELAYWRITER_H_

#include <linux/types.h>
#include <mqueue.h>
#include <vector>
#include <atomic>

class PacketFilter;
class Proxy;
class DeviceProxy;
class Endpoint;
class Manager;

class RelayWriter {
private:
	std::atomic_bool _please_stop;
	//std::vector<mqd_t> recvQueues;
	//std::vector<mqd_t> sendQueues;
	PacketQueue* _recvQueue;
	PacketQueue* _sendQueue;
	//std::vector<PacketQueue*> _sendQueues;

	__u8 endpoint;
	__u8 attributes;
	__u16 maxPacketSize;

	Proxy* proxy;
	DeviceProxy* deviceProxy;
	std::vector<PacketFilter*> filters;
	Manager* manager;

public:
	RelayWriter(Endpoint* _endpoint,Proxy* _proxy, PacketQueue& recvQueue);
	//RelayWriter(Endpoint* _endpoint,DeviceProxy* _deviceProxy,Manager* _manager,mqd_t _recvQueue,mqd_t _sendQueue);
	RelayWriter(Endpoint* _endpoint,DeviceProxy* _deviceProxy,Manager* _manager, PacketQueue& recvQueue, PacketQueue& sendQueue);
	virtual ~RelayWriter();

	PacketQueue& get_recv_queue(void) {
		return *_recvQueue;
	}
	void add_filter(PacketFilter* filter);
	//void add_setup_queue(mqd_t recvQueue,mqd_t sendQueue);
	void set_send_queue(PacketQueue& sendQueue);

	void relay_write();
	void relay_write_setup();

	void please_stop(void) {
		_please_stop = true;
		if (_recvQueue)
			_recvQueue->enqueue(PacketPtr());
	}
};

#endif /* RELAYWRITER_H_ */
