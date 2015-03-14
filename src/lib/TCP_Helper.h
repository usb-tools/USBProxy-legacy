/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_TCP_HELPER_H
#define USBPROXY_TCP_HELPER_H

#include <stddef.h>
#include <linux/types.h>
#include <poll.h>

class TCP_Helper {
private:
	bool p_server;
	bool p_is_connected;
	__u8* ep_buf[32];
	bool ep_connect[32];
	int ep_socket[32];
	int ep_listener[32];
	struct pollfd spoll;
	char* p_address;
	int client_connect(int port,int timeout);
	int client_open_endpoints(__u8* eps,__u8 num_eps,int timeout);

	void server_listen(int port);
	int server_connect(int port,int timeout);
	int server_open_endpoints(__u8* eps,__u8 num_eps,int timeout);

public:
	unsigned debugLevel;
	TCP_Helper(const char* address=NULL);
	virtual ~TCP_Helper();

	int connect(int timeout);
	void disconnect();
	int open_endpoints(__u8* eps,__u8 num_eps,int timeout);
	void reset();
	bool is_connected();

	void send_data(int ep,__u8* data,int length);
	void receive_data(int ep,__u8** data,int *length,int timeout);

	char* toString() {return (char *) "TCP network helper";}
};

#endif /* USBPROXY_TCP_HELPER_H */
