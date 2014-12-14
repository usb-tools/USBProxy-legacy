/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_CRITERIA_H
#define USBPROXY_CRITERIA_H

#include <linux/types.h>

class Endpoint;
class Interface;
class Configuration;
class Device;

struct criteria_endpoint {
	__u8 address;
	__u8 addressMask;
	__u8 attributes;
	__u8 attributesMask;
	__u16 packetSizeMin;
	__u16 packetSizeMax;
	__u8 intervalMin;
	__u8 intervalMax;

	criteria_endpoint():
		address(0),
		addressMask(0),
		attributes(0),
		attributesMask(0),
		packetSizeMin(0),
		packetSizeMax(65535),
		intervalMin(0),
		intervalMax(255) {}
	bool test(Endpoint* endpoint);
};


struct criteria_interface {
	short number;
	short alternate;
	short deviceClass;
	short subClass;
	short protocol;

	criteria_interface():
		number(-1),
		alternate(-1),
		deviceClass(-1),
		subClass(-1),
		protocol(-1) {}
	bool test(Interface* interface);
};

struct criteria_configuration {
	short number;
	__u8 attributes;
	__u8 attributesMask;

	criteria_configuration():
		number(-1),
		attributes(0),
		attributesMask(0) {}
	bool test(Configuration* configuration);
};


struct criteria_device {
	short deviceClass;
	short subClass;
	short protocol;
	__u8 ep0packetSizeMin;
	__u8 ep0packetSizeMax;
	int vendor;
	int product;
	int release;

	criteria_device():
		deviceClass(-1),
		subClass(-1),
		protocol(-1),
		ep0packetSizeMin(0),
		ep0packetSizeMax(255),
		vendor(-1),
		product(-1),
		release(-1) {}
	bool test(Device* device);
};
#endif /* USBPROXY_CRITERIA_H */
