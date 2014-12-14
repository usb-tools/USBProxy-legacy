/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_INTERFACEGROUP_H
#define USBPROXY_INTERFACEGROUP_H

#include <linux/types.h>
#include <stdlib.h>
#include "DefinitionErrors.h"

class Device;
class Interface;

class InterfaceGroup {
	private:
		__u8 number;
		__u8 alternateCount;
		Interface** interfaces;

	public:
		int activeAlternateIndex;

		InterfaceGroup(__u8 number);
		~InterfaceGroup();
		size_t get_full_descriptor_length();
		void get_full_descriptor(__u8** p);
		void add_interface(Interface* interface);
		Interface* get_interface(__u8 alternate);
		void print(__u8 tabs=0);
		__u8 get_number();
		__u8 get_alternate_count();
		Interface* get_active_interface();
		const definition_error is_defined(__u8 configId);
};

#endif /* USBPROXY_INTERFACEGROUP_H */
