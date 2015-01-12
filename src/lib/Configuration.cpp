/*
 * This file is part of USBProxy.
 */

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "TRACE.h"
#include "HexString.h"
#include "DefinitionErrors.h"

#include "Configuration.h"

#include "Device.h"
#include "InterfaceGroup.h"
#include "Interface.h"

#include "DeviceProxy.h"
#include "USBString.h"

Configuration::Configuration(Device* _device,DeviceProxy* proxy, int idx,bool otherSpeed)
{
	device=_device;
	__u8* buf=(__u8 *)malloc(9);
	usb_ctrlrequest setup_packet;
	setup_packet.bRequestType=USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
	setup_packet.bRequest=USB_REQ_GET_DESCRIPTOR;
	setup_packet.wValue=((otherSpeed?USB_DT_OTHER_SPEED_CONFIG:USB_DT_CONFIG)<<8)|idx;
	setup_packet.wIndex=0;
	setup_packet.wLength=9;
	int len=0;
	if (proxy->control_request(&setup_packet,&len,buf) < 0) {
		std::cerr << "Error sending control request!\n";
		exit(1);
	}
	len = ( buf[3] << 8) + buf[2];
	buf=(__u8*)realloc(buf,len);
	setup_packet.wLength=len;
	if (proxy->control_request(&setup_packet,&len,buf) < 0) {
		std::cerr << "Error sending control request!\n";
		exit(1);
	}
	//copy descriptor
	memcpy(&descriptor,buf,9);
	interfaceGroups=(InterfaceGroup **)calloc(descriptor.bNumInterfaces,sizeof(*interfaceGroups));

	__u8* e=buf+len;
	__u8* p=buf+9;
	while (p<e) {
		add_interface(new Interface(this,&p,e));
	}
	free(buf);

	int i;
	for (i=0;i<descriptor.bNumInterfaces;i++) {
		if (interfaceGroups[i]->get_alternate_count()==1) {
			interfaceGroups[i]->activeAlternateIndex=0;
		} else {
			// modified 20140905 atsumi@aizulab.com
			// setup_packet.bRequestType=USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
			setup_packet.bRequestType=USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE;
			setup_packet.bRequest=USB_REQ_GET_INTERFACE;
			setup_packet.wValue=0;
			setup_packet.wIndex=i;
			setup_packet.wLength=1;
			__u8 result;
			if (proxy->control_request(&setup_packet,&len,&result) < 0) {
				std::cerr << "Error sending control request!\n";
				exit(1);
			}
			interfaceGroups[i]->activeAlternateIndex=result;
		}
	}
}

Configuration::Configuration(Device* _device,const usb_config_descriptor* _descriptor) {
	device=_device;
	descriptor=*_descriptor;
	interfaceGroups=(InterfaceGroup **)calloc(descriptor.bNumInterfaces,sizeof(*interfaceGroups));
}

Configuration::Configuration(Device* _device,__u16 wTotalLength,__u8 bNumInterfaces,__u8 bConfigurationValue,__u8 iConfiguration,__u8 bmAttributes,__u8 bMaxPower,bool highSpeed) {
	device=_device;
	descriptor.bLength=9;
	descriptor.bDescriptorType=highSpeed?USB_DT_OTHER_SPEED_CONFIG:USB_DT_CONFIG;
	descriptor.wTotalLength=wTotalLength;
	descriptor.bNumInterfaces=bNumInterfaces;
	descriptor.bConfigurationValue=bConfigurationValue;
	descriptor.iConfiguration=iConfiguration;
	descriptor.bmAttributes=bmAttributes;
	descriptor.bMaxPower=bMaxPower;
	interfaceGroups=(InterfaceGroup **)calloc(descriptor.bNumInterfaces,sizeof(*interfaceGroups));
}

Configuration::~Configuration() {
	if (interfaceGroups) {
		int i;
		for(i=0;i<descriptor.bNumInterfaces;i++) {
			if (interfaceGroups[i]) {
				delete(interfaceGroups[i]);
				interfaceGroups[i]=NULL;}
		}
		free(interfaceGroups);
		interfaceGroups=NULL;
	}
}

const usb_config_descriptor* Configuration::get_descriptor() {
	return &descriptor;
}

size_t Configuration::get_full_descriptor_length() {
	size_t total=descriptor.bLength;
	int i;
	for(i=0;i<descriptor.bNumInterfaces;i++) {
		// modified 20140903 atsumi@aizulab.com
		if ( interfaceGroups[i]) {
			total+=interfaceGroups[i]->get_full_descriptor_length();
		}
	}
	return total;
}

__u8* Configuration::get_full_descriptor() {
	__u8* buf=(__u8*)malloc(get_full_descriptor_length());
	__u8* p=buf;
	memcpy(p,&descriptor,descriptor.bLength);
	p=p+descriptor.bLength;
	int i;
	for(i=0;i<descriptor.bNumInterfaces;i++) {
		// modified 20140903 atsumi@aizulab.com
		if ( interfaceGroups[i]) {
			interfaceGroups[i]->get_full_descriptor(&p);
		}
	}
	return buf;
}

void Configuration::add_interface(Interface* interface) {
	__u8 number=interface->get_descriptor()->bInterfaceNumber;
	if (!interfaceGroups[number]) {
		interfaceGroups[number]=new InterfaceGroup(number);
	}
	interfaceGroups[number]->add_interface(interface);
}

Interface* Configuration::get_interface_alternate(__u8 number,__u8 alternate) {
	if (number>=descriptor.bNumInterfaces || number<0) {return NULL;}
	if (!interfaceGroups[number]) {return NULL;}
	return interfaceGroups[number]->get_interface(alternate);
}

Interface* Configuration::get_interface(__u8 number) {
	if (number>=descriptor.bNumInterfaces || number<0) {return NULL;}
	if (!interfaceGroups[number]) {return NULL;}
	return interfaceGroups[number]->get_active_interface();
}

__u8 Configuration::get_interface_alternate_count(__u8 number) {
	if (!interfaceGroups[number]) {return 0;}
	return interfaceGroups[number]->get_alternate_count();
}


void Configuration::print(__u8 tabs,bool active) {
	unsigned int i;
	char* hex=hex_string(&descriptor,sizeof(descriptor));
	printf("%.*s%cConfig(%d): %s\n",tabs,TABPADDING,active?'*':' ',descriptor.bConfigurationValue,hex);
	free(hex);
	if (descriptor.iConfiguration) {
		USBString* s=get_config_string();
		if (s) {
			char* ascii=s->get_ascii();
			printf("%.*s   Name: %s\n",tabs,TABPADDING,ascii);
			free(ascii);
		}
	}
	for(i=0;i<descriptor.bNumInterfaces;i++) {
		if (interfaceGroups[i]) {interfaceGroups[i]->print(tabs+1);}
	}
}

USBString* Configuration::get_config_string(__u16 languageId) {
	if (!device||!descriptor.iConfiguration) {return NULL;}
	return device->get_string(descriptor.iConfiguration,languageId);
}

bool Configuration::is_highspeed() {
	return descriptor.bDescriptorType==USB_DT_OTHER_SPEED_CONFIG?true:false;
}

const definition_error Configuration::is_defined(bool highSpeed) {
	if (descriptor.bLength!=9) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x01, (highSpeed?DE_OBJ_OS_CONFIG:DE_OBJ_CONFIG),(highSpeed?0x80:0x0)|descriptor.bConfigurationValue);}
	if (descriptor.bDescriptorType!=(highSpeed?USB_DT_OTHER_SPEED_CONFIG:USB_DT_CONFIG)) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x02, (highSpeed?DE_OBJ_OS_CONFIG:DE_OBJ_CONFIG),(highSpeed?0x80:0x0)|descriptor.bConfigurationValue);}
	if (descriptor.wTotalLength!=get_full_descriptor_length()) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x03, (highSpeed?DE_OBJ_OS_CONFIG:DE_OBJ_CONFIG),(highSpeed?0x80:0x0)|descriptor.bConfigurationValue);}
	if (!descriptor.bNumInterfaces) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x05, (highSpeed?DE_OBJ_OS_CONFIG:DE_OBJ_CONFIG),(highSpeed?0x80:0x0)|descriptor.bConfigurationValue);}
	//__u8  bConfigurationValue;
	//__u8  iConfiguration;
	if ((descriptor.bmAttributes&0x9f)!=0x80) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x08, (highSpeed?DE_OBJ_OS_CONFIG:DE_OBJ_CONFIG),(highSpeed?0x80:0x0)|descriptor.bConfigurationValue);}
	//__u8  bMaxPower;

	int i;
	for (i=0;i<descriptor.bNumInterfaces;i++) {
		if (!interfaceGroups[i]) {return definition_error(DE_ERR_NULL_OBJECT,0x0, DE_OBJ_INTERFACEGROUP,(highSpeed?0x80:0x0)|descriptor.bConfigurationValue,i);}
		if (interfaceGroups[i]->get_number()!=i) {return definition_error(DE_ERR_MISPLACED_OBJECT,interfaceGroups[i]->get_number(), DE_OBJ_INTERFACEGROUP,(highSpeed?0x80:0x0)|descriptor.bConfigurationValue,i);}
		definition_error rc=interfaceGroups[i]->is_defined((highSpeed?0x80:0x0)|descriptor.bConfigurationValue);
		if (rc.error) {return rc;}
	}
	return definition_error();
}

Device* Configuration::get_device() {return device;}
