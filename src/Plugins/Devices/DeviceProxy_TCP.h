/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_DEVICEPROXY_TCP_H
#define USBPROXY_DEVICEPROXY_TCP_H

#include "DeviceProxy.h"
#include "TCP_Helper.h"

class DeviceProxy_TCP:public DeviceProxy {
private:
	bool p_is_connected;
	TCP_Helper* network;

public:
	static int debugLevel;
	DeviceProxy_TCP(const char* address=NULL);
	DeviceProxy_TCP(ConfigParser *cfg);
	~DeviceProxy_TCP();

	int connect(int timeout=250);
	void disconnect();
	void reset();
	bool is_connected();
	bool is_highspeed();

	int control_request(const usb_ctrlrequest *setup_packet, int *nbytes, __u8 *dataptr, int timeout=500);
	void send_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8* dataptr,int length);
	void receive_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8** dataptr, int* length,int timeout=500);

	void setConfig(Configuration* fs_cfg,Configuration* hs_cfg,bool hs);

	void claim_interface(__u8 interface);
	void release_interface(__u8 interface);

	// modified 20141003 atsumi@aizulab.com
	void setEp2inf( __u8 *ep2inf_, __u8 *claimedInterface_) {};
	__u8 get_address();
	char* toString() {return (char *) "TCP device proxy";}
};

#endif /* USBPROXY_DEVICEPROXY_TCP_H */
