/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_DEVICEPROXY_LOOPBACK_H
#define USBPROXY_DEVICEPROXY_LOOPBACK_H

#include "DeviceProxy.h"

class Configuration;

struct pkt {
	int length;
	__u8 *data;

	pkt(): length(0),data(NULL) {}
};

class DeviceProxy_Loopback : public DeviceProxy {
private:
	bool p_is_connected;
	struct pkt *buffer;
	int head, tail;
	bool full;
	struct usb_device_descriptor loopback_device_descriptor;
	struct usb_config_descriptor loopback_config_descriptor;
	struct usb_interface_descriptor loopback_interface_descriptor;
	struct usb_endpoint_descriptor loopback_eps[2];
	struct usb_string ;

public:
	DeviceProxy_Loopback(int vendorId, int productId);
	DeviceProxy_Loopback(ConfigParser *cfg);
	~DeviceProxy_Loopback();

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
	char* toString() {return (char *) "Lookback device";}

	void set_endpoint_interface(__u8 endpoint, __u8 interface);
	void claim_interface(__u8 interface);
	void release_interface(__u8 interface);

	__u8 get_address();
};

#endif /* USBPROXY_DEVICEPROXY_LOOPBACK_H */
