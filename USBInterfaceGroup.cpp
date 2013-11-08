/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
 *
 * This file is part of USB-MitM.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 *
 * USBInterfaceGroup.cpp
 *
 * Created on: Nov 6, 2013
 */

#include "USBInterfaceGroup.h"
#include "stdio.h"
#include <stdlib.h>
#include <memory.h>

USBInterfaceGroup::USBInterfaceGroup(__u8 _number) {
	number=_number;
	alternateCount=0;
	interfaces=NULL;
}
USBInterfaceGroup::~USBInterfaceGroup() {
	if (interfaces) {
		int i;
		for(i=0;i<alternateCount;i++) {delete(interfaces[i]);}
		free(interfaces);
	}
}
void USBInterfaceGroup::getFullDescriptor(__u8** p) {
	int i;
	for(i=0;i<alternateCount;i++) {interfaces[i]->get_full_descriptor(p);}
}

void USBInterfaceGroup::add_interface(USBInterface* interface) {
	__u8 alternate=interface->get_descriptor()->bAlternateSetting;
	if (alternate>=alternateCount) {
		USBInterface** newInterfaces=(USBInterface **)calloc(alternate+1,sizeof(*interfaces));
		if (alternateCount) {
			memcpy(newInterfaces,interfaces,sizeof(*interfaces)*alternateCount);
			free(interfaces);
		}
		interfaces=newInterfaces;
		alternateCount=alternate+1;
	} else {
		if (interfaces[alternate]) {delete(interfaces[alternate]);}
	}
	interfaces[alternate]=interface;
}

USBInterface* USBInterfaceGroup::get_interface(__u8 alternate) {
	return interfaces[alternate];
}

void USBInterfaceGroup::print(__u8 tabs) {
	int i;
	for(i=0;i<tabs;i++) {putchar('\t');}
	printf("Interface(%d):",number);
	putchar('\n');
	for(i=0;i<alternateCount;i++) {
		interfaces[i]->print(tabs+1);
	}
}

void USBInterfaceGroup::set_usb_device(USBDevice* _device) {
	if (interfaces) {
		int i;
		for(i=0;i<alternateCount;i++) {interfaces[i]->set_usb_device(_device);}
	}
}

__u8 USBInterfaceGroup::get_alternate_count() {
	return alternateCount;
}
