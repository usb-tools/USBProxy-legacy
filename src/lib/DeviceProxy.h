/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_DEVICEPROXY_H
#define USBPROXY_DEVICEPROXY_H

#include <linux/usb/ch9.h>
#include "Proxy.h"

class Configuration;

class DeviceProxy : public Proxy {
public:
	static const __u8 plugin_type=PLUGIN_DEVICEPROXY;
	
	DeviceProxy(const ConfigParser& cfg)
		: Proxy(cfg.debugLevel)
	{}
	DeviceProxy()
	{}
	virtual ~DeviceProxy() {}

	//return ETIMEDOUT if it times out
	virtual int connect(int timeout=250)=0;
	virtual void disconnect()=0;
	virtual void reset()=0;
	virtual bool is_connected()=0;

	virtual bool is_highspeed()=0;

	//return -1 to stall
	virtual int control_request(const usb_ctrlrequest *setup_packet, int *nbytes, __u8* dataptr,int timeout=500)=0;

	virtual void send_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8* dataptr,int length)=0;
	virtual bool send_wait_complete(__u8 endpoint,int timeout=500) {return true;}
	virtual void receive_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8** dataptr, int* length,int timeout=500)=0;
	virtual void setConfig(Configuration* fs_cfg,Configuration* hs_cfg,bool hs)=0;
	virtual char* toString() {return NULL;}

	virtual void claim_interface(__u8 interface)=0;
	virtual void release_interface(__u8 interface)=0;

	virtual __u8 get_address()=0;
};

extern "C" {
	DeviceProxy *get_deviceproxy_plugin(ConfigParser *cfg);
	void destroy_plugin();
}
#endif /* USBPROXY_DEVICEPROXY_H */
