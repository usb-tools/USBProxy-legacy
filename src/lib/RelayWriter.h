/*
 * This file is part of USBProxy.
 */

#ifndef RELAYWRITER_H_
#define RELAYWRITER_H_

#include <linux/types.h>
#include <mqueue.h>
#include <vector>

class PacketFilter;
class Proxy;
class DeviceProxy;
class Endpoint;
class Manager;

class RelayWriter {
private:
	__u8 haltSignal;
	std::vector<mqd_t> recvQueues;
	std::vector<mqd_t> sendQueues;
	__u8 endpoint;
	__u8 attributes;
	__u16 maxPacketSize;

	Proxy* proxy;
	DeviceProxy* deviceProxy;
	std::vector<PacketFilter*> filters;
	Manager* manager;

public:
	RelayWriter(Endpoint* _endpoint,Proxy* _proxy,mqd_t _recvQueue);
	RelayWriter(Endpoint* _endpoint,DeviceProxy* _deviceProxy,Manager* _manager,mqd_t _recvQueue,mqd_t _sendQueue);
	virtual ~RelayWriter();

	void add_filter(PacketFilter* filter);
	void add_queue(mqd_t inQueue);
	void add_setup_queue(mqd_t recvQueue,mqd_t sendQueue);

	void relay_write();
	void relay_write_setup();

#ifndef NVALGRIND
	void relay_write_valgrind();
	void relay_write_setup_valgrind();
#endif //NVALGRIND

	void set_haltsignal(__u8 _haltSignal);
	static void* relay_write_helper(void* context);
};

#endif /* RELAYWRITER_H_ */
