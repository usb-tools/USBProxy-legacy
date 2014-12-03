/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_DEVICEPROXYNULL_H
#define USBPROXY_DEVICEPROXYNULL_H

#include "DeviceProxy.h"

class DeviceProxy_Null: public DeviceProxy {
private:
	bool connected=false;
public:
	DeviceProxy_Null() {}
	virtual ~DeviceProxy_Null();

	virtual int connect() {connected=true;return 0;}
	virtual void disconnect() {connected=false;}
	virtual void reset() {}
	virtual bool is_connected() {return connected;}

	//this should be done synchronously
	virtual int control_request(const usb_ctrlrequest *setup_packet, int *nbytes, __u8* dataptr) {return 0;}
	virtual void send_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8* dataptr,int length) {}
	virtual void receive_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8** dataptr, int* length) {*length=0;}

	virtual __u8 get_address() {return 0;}
	virtual const char* toString() {return (char*)"Null Device";}

};

#endif /* USBPROXY_DEVICEPROXYNULL_H */
