/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_HOSTPROXY_H
#define USBPROXY_HOSTPROXY_H

#include <linux/usb/ch9.h>
#include "Proxy.h"
#include "Device.h"

class HostProxy: public Proxy {
public:
	static const __u8 plugin_type=PLUGIN_HOSTPROXY;

	HostProxy(const ConfigParser& cfg)
		: Proxy(cfg.debugLevel)
	{}
	HostProxy()
	{}
	virtual ~HostProxy() {}

	//return ETIMEDOUT if it times out
	virtual int connect(Device* device,int timeout=250)=0;
	virtual void disconnect()=0;
	virtual void reset()=0;
	virtual bool is_connected()=0;

	//return 0 in usb_ctrlrequest->brequest if there is no request
	virtual int control_request(usb_ctrlrequest *setup_packet, int *nbytes, __u8** dataptr,int timeout=500)=0;

	virtual void send_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8* dataptr,int length)=0;
	virtual bool send_wait_complete(__u8 endpoint,int timeout=500) {return true;}
	virtual void receive_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8** dataptr, int* length,int timeout=500)=0;
	virtual void setConfig(Configuration* fs_cfg,Configuration* hs_cfg,bool hs)=0;
	virtual char* toString() {return NULL;}

	virtual void control_ack()=0;
	virtual void stall_ep(__u8 endpoint)=0;
};

extern "C" {
	HostProxy *get_hostproxy_plugin(ConfigParser *cfg);
	void destroy_plugin();
}
#endif /* USBPROXY_HOSTPROXY_H */
