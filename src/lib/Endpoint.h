/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_ENDPOINT_H
#define USBPROXY_ENDPOINT_H

#include <linux/usb/ch9.h>
#include <stdlib.h>
#include "DefinitionErrors.h"

class Interface;
class DeviceProxy;

class Endpoint {
	private:
		usb_endpoint_descriptor descriptor;
		Interface* interface;

	public:
		Endpoint(Interface* _interface,const __u8* p);
		Endpoint(Interface* _interface,const usb_endpoint_descriptor* _descriptor);
		Endpoint(Interface* _interface,__u8 bEndpointAddress,__u8 bmAttributes,__u16 wMaxPacketSize,__u8 bInterval);
		~Endpoint();
		const usb_endpoint_descriptor* get_descriptor();
		size_t get_full_descriptor_length();
		void get_full_descriptor(__u8** p);
		void print(__u8 tabs=0);
		const definition_error is_defined(__u8 configId,__u8 interfaceNum,__u8 interfaceAlternate);
		Interface* get_interface();
};

#endif /* USBPROXY_ENDPOINT_H */
