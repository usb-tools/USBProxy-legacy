/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_DEVICEQUALIFIER_H
#define USBPROXY_DEVICEQUALIFIER_H

#include <linux/usb/ch9.h>
#include "DefinitionErrors.h"

class Device;
class Configuration;
class DeviceProxy;

class DeviceQualifier {
private:
    usb_qualifier_descriptor descriptor;
    Configuration** configurations;
    Device* device;

public:
    DeviceQualifier(Device* _device,DeviceProxy* _proxy);
    DeviceQualifier(Device* _device,const usb_qualifier_descriptor* _descriptor);
    DeviceQualifier(Device* _device,__le16 bcdUSB,	__u8  bDeviceClass,	__u8  bDeviceSubClass,	__u8  bDeviceProtocol,	__u8  bMaxPacketSize0, __u8 bNumConfigurations);
	~DeviceQualifier();
	const usb_qualifier_descriptor* get_descriptor();
	void add_configuration(Configuration* config);
	Configuration* get_configuration(__u8 index);
	void print(__u8 tabs=0);
	void set_device(Device* _device);
	const definition_error is_defined();
};

#endif /* USBPROXY_DEVICEQUALIFIER_H */
