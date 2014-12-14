/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_CONFIGURATION_H
#define USBPROXY_CONFIGURATION_H

#include <stddef.h>
#include <linux/types.h>
#include <linux/usb/ch9.h>
#include "DefinitionErrors.h"

class Device;
class InterfaceGroup;
class Interface;

class DeviceProxy;
class USBString;

class Configuration {
	private:
		usb_config_descriptor descriptor;
		InterfaceGroup** interfaceGroups;
		Device* device;

	public:
		Configuration(Device* _device,DeviceProxy* proxy, int index,bool otherSpeed=false);
		Configuration(Device* _device,const usb_config_descriptor* _descriptor);
		Configuration(Device* _device,__u16 wTotalLength,__u8 bNumInterfaces,__u8 bConfigurationValue,__u8 iConfiguration,__u8 bmAttributes,__u8 bMaxPower,bool highSpeed=false);
		~Configuration();
		const usb_config_descriptor* get_descriptor();
		__u8* get_full_descriptor();
		size_t get_full_descriptor_length();
		void add_interface(Interface* interface);
		Interface* get_interface_alternate(__u8 number,__u8 alternate);
		Interface* get_interface(__u8 number);
		void print(__u8 tabs=0,bool active=false);
		USBString* get_config_string(__u16 languageId=0);

		__u8 get_interface_alternate_count(__u8 number);
		bool is_highspeed();
		const definition_error is_defined(bool highSpeed=false);
		Device* get_device();
};
#endif /* USBPROXY_CONFIGURATION_H */
