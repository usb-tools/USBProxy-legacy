/*
 * This file is part of USBProxy.
 */

#ifndef EP_IO_H_
#define EP_IO_H_

enum ep_io_action {
	EP_ACTION_FUNCTION=1,
	EP_ACTION_FD=2,
	EP_ACTION_AIO=3
};

enum ep_io_notify {
	EP_NOTIFY_TIMEOUT=1,
	EP_NOTIFY_POLL=2,
	EP_NOTIFY_AIO=3
};

struct ep_transfer_info {
	__u8 endpoint;
	__u8* data;
	int length;
};

struct ep_setup_info {
	__u8 endpoint;
	__u8 attributes;
	__u16 maxPacketSize;
};

class USBHostProxy;
class USBDeviceProxy;

struct ep_io_info {
	enum ep_io_action io_action;
	enum ep_io_notify io_notify;
	union {
		int fd;
		USBHostProxy* hostProxy;
		USBDeviceProxy* deviceProxy;
	};
};

//TODO the thinking here is we can call a host/device proxy with
//virtual ep_io_info initialize_io(struct ep_setup_info ep_info)=0;
//and it will indicate which monitoring method to use
	//EP_NOTIFY_TIMEOUT will call a function with a timeout
	//EP_NOTIFY_POLL will provide a pollable FD
	//EP_NOTIFY_AIO will wait on an async IO handle
//when the EP is available we can use several different methods to actually accomplish the transfer
	//EP_ACTION_FUNCTION call the standar device/hostproxy function
	//EP_ACTION_FD standard direct read/write to the FD
	//EP_ACTION_AIO async direct read/write to the FD
//EP_NOTIFY_TIMEOUT will require EP_ACTION_FUNCTION
//EP_NOTIFY_POLL would probably do EP_ACTION_FD,
//but could do EP_ACTION_FUNCTION (but we would need a new virtual member function in the base clasee to handle this)
//EP_NOTIFY_AIO would probably go with //EP_ACTION_AIO but could use a function as above.


#endif /* EP_IO_H_ */
