/*
 * This file is part of USBProxy.
 */

#include <stdlib.h>
#include <stdio.h>

#include "DefinitionErrors.h"
#include "HexString.h"

#include "DeviceQualifier.h"

#include "Device.h"
#include "Configuration.h"
#include "Interface.h"

#include "DeviceProxy.h"

DeviceQualifier::DeviceQualifier(Device* _device,DeviceProxy* proxy) {
	device=_device;

	usb_ctrlrequest setup_packet;
	setup_packet.bRequestType=USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
	setup_packet.bRequest=USB_REQ_GET_DESCRIPTOR;
	setup_packet.wValue=USB_DT_DEVICE_QUALIFIER<<8;
	setup_packet.wIndex=0;
	setup_packet.wLength=10;
	int len=0;
	if (proxy->control_request(&setup_packet,&len,(__u8 *)&descriptor)) {
		descriptor.bLength=0;
		configurations=NULL;
		return;
	}
	int i;
	configurations=(Configuration **)calloc(descriptor.bNumConfigurations,sizeof(*configurations));

	for(i=0;i<descriptor.bNumConfigurations;i++) {
		configurations[i]=new Configuration(device,proxy,i,true);
		__u8 iConfiguration=configurations[i]->get_descriptor()->iConfiguration;
		if (iConfiguration) {device->add_string(iConfiguration);}
		int j;
		for (j=0;j<configurations[i]->get_descriptor()->bNumInterfaces;j++) {
			int k;
			for (k=0;k<configurations[i]->get_interface_alternate_count(j);k++) {

				// modified 20140903 atsumi@aizulab.com
				// begin
				if ( configurations[i]->get_interface_alternate(j,k)) {
					if ( configurations[i]->get_interface_alternate(j,k)->get_descriptor()) {
						__u8 iInterface=configurations[i]->get_interface_alternate(j,k)->get_descriptor()->iInterface;
						if (iInterface) {
							device->add_string(iInterface);
						}
					}
				}
				// End
			}
		}
	}
}

DeviceQualifier::DeviceQualifier(Device* _device,const usb_qualifier_descriptor* _descriptor) {
	device=_device;
	descriptor=*_descriptor;
	configurations=(Configuration **)calloc(descriptor.bNumConfigurations,sizeof(*configurations));
}

DeviceQualifier::DeviceQualifier(Device* _device,__le16 bcdUSB,	__u8  bDeviceClass,	__u8  bDeviceSubClass,	__u8  bDeviceProtocol,	__u8  bMaxPacketSize0, __u8 bNumConfigurations) {
	device=_device;
	descriptor.bLength=10;
	descriptor.bDescriptorType=USB_DT_DEVICE_QUALIFIER;
	descriptor.bcdUSB=bcdUSB;
	descriptor.bDeviceClass=bDeviceClass;
	descriptor.bDeviceSubClass=bDeviceSubClass;
	descriptor.bDeviceProtocol=bDeviceProtocol;
	descriptor.bMaxPacketSize0=bMaxPacketSize0;
	descriptor.bNumConfigurations=bNumConfigurations;
	configurations=(Configuration **)calloc(descriptor.bNumConfigurations,sizeof(*configurations));
}

DeviceQualifier::~DeviceQualifier() {
	int i;
	if (configurations) {
		for(i=0;i<descriptor.bNumConfigurations;i++) {
			if (configurations[i]) {
				delete(configurations[i]);
				configurations[i]=NULL;
			}
		}
		free(configurations);
		configurations=NULL;
	}
}

const usb_qualifier_descriptor* DeviceQualifier::get_descriptor() {
	return &descriptor;
}

void DeviceQualifier::add_configuration(Configuration* config) {
	int value=config->get_descriptor()->bConfigurationValue;
	if (value>descriptor.bNumConfigurations) {return;} else {value--;}
	if (configurations[value]) {delete(configurations[value]);/* not needed configurations[value]=NULL; */}
	configurations[value]=config;
}

Configuration* DeviceQualifier::get_configuration(__u8 index) {
	if (index>descriptor.bNumConfigurations || index<1) {return NULL;}
	return configurations[index-1];
}

void DeviceQualifier::print(__u8 tabs) {
	int i;
	char* hex=hex_string(&descriptor,sizeof(descriptor));
	printf("%.*sHS Qualifier: %s\n",tabs,TABPADDING,hex);
	free(hex);
	for(i=0;i<descriptor.bNumConfigurations;i++) {
		if (configurations[i]) {configurations[i]->print(tabs+1,(configurations[i]==device->get_active_configuration())?true:false);}
	}
}

void DeviceQualifier::set_device(Device* _device) {
	device=_device;
}

const definition_error DeviceQualifier::is_defined() {
	if (descriptor.bLength!=10) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x01, DE_OBJ_QUALIFIER);}
	if (descriptor.bDescriptorType!=USB_DT_DEVICE_QUALIFIER) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x02, DE_OBJ_QUALIFIER);}
	//__le16 bcdUSB;
	//__u8  bDeviceClass;
	//__u8  bDeviceSubClass;
	//__u8  bDeviceProtocol;
	if (descriptor.bMaxPacketSize0!=8&&descriptor.bMaxPacketSize0!=16&&descriptor.bMaxPacketSize0!=32&&descriptor.bMaxPacketSize0!=64) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x08, DE_OBJ_QUALIFIER);}
	if (!descriptor.bNumConfigurations) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x09, DE_OBJ_QUALIFIER);}
	//__u8  bRESERVED;

	int i;
	for (i=0;i<descriptor.bNumConfigurations;i++) {
		if (!configurations[i]) {return definition_error(DE_ERR_NULL_OBJECT,0x0, DE_OBJ_OS_CONFIG,i+1);}
		if (configurations[i]->get_descriptor()->bConfigurationValue!=(i+1)) {return definition_error(DE_ERR_MISPLACED_OBJECT,configurations[i]->get_descriptor()->bConfigurationValue, DE_OBJ_OS_CONFIG,i+1);}
		definition_error rc=configurations[i]->is_defined(true);
		if (rc.error) {return rc;}
	}

	return definition_error();
}
