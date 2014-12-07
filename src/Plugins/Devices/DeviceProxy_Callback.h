/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_DEVICEPROXY_CALLBACK_H
#define USBPROXY_DEVICEPROXY_CALLBACK_H

#include "DeviceProxy.h"

class Configuration;

struct pkt {
	int length;
	__u8 *data;

	pkt(): length(0),data(NULL) {}
};

typedef int (*f_connect)(int);
typedef void (*f_disconnect)(void);
typedef void (*f_reset)(void);
typedef int (*f_control_request)(const usb_ctrlrequest*,int*,__u8*,int);
typedef void (*f_send_data)(__u8,__u8,__u16,__u8*,int);
typedef void (*f_receive_data)(__u8,__u8,__u16,__u8**,int*,int);
typedef char* (*f_toString)(void);


class DeviceProxy_Callback : public DeviceProxy {
private:
	bool p_is_connected;
	struct pkt *buffer;
	int head, tail;
	bool full;
	struct usb_device_descriptor callback_device_descriptor;
	struct usb_config_descriptor callback_config_descriptor;
	struct usb_interface_descriptor callback_interface_descriptor;
	struct usb_endpoint_descriptor callback_eps[2];
	struct usb_string ;

	f_connect connect_cb;
	f_disconnect disconnect_cb;
	f_reset reset_cb;
	f_control_request control_request_cb;
	f_send_data send_data_cb;
	f_receive_data receive_data_cb;
	f_toString toString_cb;

public:
	static int debugLevel;
	DeviceProxy_Callback(ConfigParser *cfg);
	~DeviceProxy_Callback();

	int connect(int timeout=250);
	void disconnect();
	void reset();
	bool is_connected();

	bool is_highspeed();

	//return -1 to stall
	int control_request(const usb_ctrlrequest *setup_packet, int *nbytes, __u8* dataptr,int timeout=500);

	void send_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8* dataptr,int length);
	bool send_wait_complete(__u8 endpoint,int timeout=500) {return true;}
	void receive_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8** dataptr, int* length,int timeout=500);
	void setConfig(Configuration* fs_cfg,Configuration* hs_cfg,bool hs);
	char* toString();

	void claim_interface(__u8 interface);
	void release_interface(__u8 interface);

	__u8 get_address();
};

#endif /* USBPROXY_DEVICEPROXY_CALLBACK_H */
