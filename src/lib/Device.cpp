/*
 * This file is part of USBProxy.
 */

#include <iostream>

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "DefinitionErrors.h"
#include "TRACE.h"
#include "HexString.h"

#include "Device.h"
#include "DeviceQualifier.h"
#include "Configuration.h"
#include "Interface.h"

#include "USBString.h"
#include "DeviceProxy.h"

//CLEANUP what happends if device is HS but host is not, in terms of correct config to use,etc.

Device::Device(DeviceProxy* _proxy) {
	hostConfigurationIndex=-1;
	hostAddress=-1;
	hostState=USB_STATE_NOTATTACHED;
    maxStringIdx=0;

    proxy=_proxy;
	__u8 buf[18];
	usb_ctrlrequest setup_packet;
	setup_packet.bRequestType=USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
	setup_packet.bRequest=USB_REQ_GET_DESCRIPTOR;
	setup_packet.wValue=USB_DT_DEVICE<<8;
	setup_packet.wIndex=0;
	setup_packet.wLength=18;
	int len=0;
	if (proxy->control_request(&setup_packet,&len,buf) < 0) {
		std::cerr << "Error sending control request!\n";
		exit(1);
	}
	memcpy(&descriptor,buf,len);
	int i;
	configurations=(Configuration **)calloc(descriptor.bNumConfigurations,sizeof(*configurations));

	maxStringIdx=(descriptor.iManufacturer>maxStringIdx)?descriptor.iManufacturer:maxStringIdx;
	maxStringIdx=(descriptor.iProduct>maxStringIdx)?descriptor.iProduct:maxStringIdx;
	maxStringIdx=(descriptor.iSerialNumber>maxStringIdx)?descriptor.iSerialNumber:maxStringIdx;
	strings=(USBString ***)calloc(maxStringIdx+1,sizeof(*strings));

	add_string(0,0);

	if (descriptor.iManufacturer) {add_string(descriptor.iManufacturer);}
	if (descriptor.iProduct) {add_string(descriptor.iProduct);}
	if (descriptor.iSerialNumber) {add_string(descriptor.iSerialNumber);}

	for(i=0;i<descriptor.bNumConfigurations;i++) {
		configurations[i]=new Configuration(this,proxy,i);
		__u8 iConfiguration=configurations[i]->get_descriptor()->iConfiguration;
		if (iConfiguration) {add_string(iConfiguration);}
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
							add_string(iInterface);
						}
					}
				}
				// End
			}
		}
	}

	qualifier=new DeviceQualifier(this,proxy);

	//not a high speed device
	if (!(qualifier->get_descriptor()->bLength)) {
		delete(qualifier);
		qualifier=NULL;
	}

	deviceState=USB_STATE_DEFAULT;
	deviceAddress=proxy->get_address();
	highspeed=proxy->is_highspeed();
	if (deviceAddress) {
		setup_packet.bRequestType=USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
		setup_packet.bRequest=USB_REQ_GET_CONFIGURATION;
		setup_packet.wValue=0;
		setup_packet.wIndex=0;
		setup_packet.wLength=1;
		__u8 result;
		if (proxy->control_request(&setup_packet,&len,&result) < 0) {
			std::cerr << "Error sending control request!\n";
			exit(1);
		}
		deviceConfigurationIndex=result;
		deviceState=deviceConfigurationIndex?USB_STATE_CONFIGURED:USB_STATE_ADDRESS;
	} else {
		deviceState=USB_STATE_ADDRESS;
		deviceConfigurationIndex=0;
	}
}

Device::Device(const usb_device_descriptor* _descriptor) {
	hostConfigurationIndex=-1;
	hostAddress=-1;
	hostState=USB_STATE_NOTATTACHED;
    maxStringIdx=0;
    highspeed=false;

    proxy=NULL;
	qualifier=NULL;
	deviceAddress=0;
	deviceConfigurationIndex=-1;
	deviceState=USB_STATE_NOTATTACHED;

	descriptor=*_descriptor;
	configurations=(Configuration **)calloc(descriptor.bNumConfigurations,sizeof(*configurations));
	strings=(USBString ***)calloc(1,sizeof(*strings));
	strings[0]=(USBString **)malloc(sizeof(**strings)*2);
	__u16 zero[]={0x0409, 0};
	strings[0][0]=new USBString(zero,0,0);
	strings[0][1]=NULL;
}

Device::Device(__le16 bcdUSB,	__u8  bDeviceClass,	__u8  bDeviceSubClass,	__u8  bDeviceProtocol,	__u8  bMaxPacketSize0,	__le16 idVendor,	__le16 idProduct,	__le16 bcdDevice,	__u8  iManufacturer,	__u8  iProduct,	__u8  iSerialNumber,	__u8  bNumConfigurations) {
	hostConfigurationIndex=-1;
	hostAddress=-1;
	hostState=USB_STATE_NOTATTACHED;
    maxStringIdx=0;
    highspeed=false;

	proxy=NULL;
	qualifier=NULL;
	deviceAddress=0;
	deviceConfigurationIndex=-1;
	deviceState=USB_STATE_NOTATTACHED;

	descriptor.bLength=18;
	descriptor.bDescriptorType=USB_DT_DEVICE;
	descriptor.bcdUSB=bcdUSB;
	descriptor.bDeviceClass=bDeviceClass;
	descriptor.bDeviceSubClass=bDeviceSubClass;
	descriptor.bDeviceProtocol=bDeviceProtocol;
	descriptor.bMaxPacketSize0=bMaxPacketSize0;
	descriptor.idVendor=idVendor;
	descriptor.idProduct=idProduct;
	descriptor.bcdDevice=bcdDevice;
	descriptor.iManufacturer=iManufacturer;
	descriptor.iProduct=iProduct;
	descriptor.iSerialNumber=iSerialNumber;
	descriptor.bNumConfigurations=bNumConfigurations;
	configurations=(Configuration **)calloc(descriptor.bNumConfigurations,sizeof(*configurations));
	strings=(USBString ***)calloc(1,sizeof(*strings));
	strings[0]=(USBString **)malloc(sizeof(**strings)*2);
	__u16 zero[]={0x0409, 0};
	strings[0][0]=new USBString(zero,0,0);
	strings[0][1]=NULL;
}

Device::~Device() {
	int i;
	if (qualifier) {
		delete(qualifier);
		qualifier=NULL;
	}
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
	if (strings) {
		for(i=0;i<=maxStringIdx;i++) {
			if (strings[i]) {
				int j=0;
				while (strings[i][j]) {
					delete(strings[i][j]);
					strings[i][j]=NULL;
					j++;
				}
				free(strings[i]);
				strings[i]=NULL;
			}
		}
		free(strings);
		strings=NULL;
	}
}

const usb_device_descriptor* Device::get_descriptor() {
	return &descriptor;
}

void Device::add_configuration(Configuration* config) {
	int value=config->get_descriptor()->bConfigurationValue;
	if (value>descriptor.bNumConfigurations) {return;} else {value--;}
	if (configurations[value]) {delete(configurations[value]);/* not needed configurations[value]=NULL; */}
	configurations[value]=config;
}

Configuration* Device::get_configuration(__u8 index) {
	if (index>descriptor.bNumConfigurations || index<1) {return NULL;}
	return configurations[index-1];
}

void Device::print(__u8 tabs) {
	int i;
	char* hex=hex_string(&descriptor,sizeof(descriptor));
	printf("%.*sDevice: %s\n",tabs,TABPADDING,hex);
	free(hex);

	USBString* s;
	if (descriptor.iManufacturer) {
		s=get_manufacturer_string();
		if (s) {
			char* ascii=s->get_ascii();
			printf("%.*s  Manufacturer: %s\n",tabs,TABPADDING,ascii);
			free(ascii);
		}
	}
	if (descriptor.iProduct) {
		s=get_product_string();
		if (s) {
			char* ascii=s->get_ascii();
			printf("%.*s  Product:      %s\n",tabs,TABPADDING,ascii);
			free(ascii);
		}
	}
	if (descriptor.iSerialNumber) {
		s=get_serial_string();
		if (s) {
			char* ascii=s->get_ascii();
			printf("%.*s  Serial:       %s\n",tabs,TABPADDING,ascii);
			free(ascii);
		}
	}
	for(i=0;i<descriptor.bNumConfigurations;i++) {
		if (configurations[i]) {
			configurations[i]->print(tabs+1,configurations[i]==get_active_configuration()?true:false);
		}
	}
	if (qualifier) {
		qualifier->print(tabs);
	}
}

void Device::add_string(USBString* string) {
	__u8 index=string->get_index();
	__u16 languageId=string->get_languageId();
	if (index||languageId) add_language(languageId);
	if (index>maxStringIdx) {
		USBString*** newStrings=(USBString ***)calloc(index+1,sizeof(*newStrings));
		if (strings) {
			memcpy(newStrings,strings,sizeof(*newStrings)*(maxStringIdx+1));
			free(strings);
			/*not needed strings=NULL;*/
		}
		strings=newStrings;
		maxStringIdx=index;
	}
	if (strings[index]) {
		int i=0;
		while (true) {
			if (strings[index][i]) {
				if (strings[index][i]->get_languageId()==languageId) {
					// modified 20140902 atsumi@aizulab.com
          // There is a possibility of memory leaks
          // delete(strings[index][i]); 
					/* not needed strings[index][i]=NULL; */
					strings[index][i]=string;
				}
			} else {
				strings[index]=(USBString**)realloc(strings[index],sizeof(USBString*)*(i+2));
				strings[index][i]=string;
				strings[index][i+1]=NULL;
				break;
			}
			i++;
		}
	} else {
		strings[index]=(USBString**)malloc(sizeof(USBString*)*2);
		strings[index][0]=string;
		strings[index][1]=NULL;
	}
}

//adds via proxy
void Device::add_string(__u8 index,__u16 languageId) {
	if (!proxy) {fprintf(stderr,"Can't automatically add string, no device proxy defined.\n");return;}
	add_string(new USBString(proxy,index,languageId));
}

//adds for all languages
void Device::add_string(__u8 index) {
	if (!strings[0]) {return;}
	if (!strings[0][0]) {return;}
	const usb_string_descriptor* p=strings[0][0]->get_descriptor();
	__u8 length=(p->bLength)>>1;
	int i;
	for(i=0;i<(length-1);i++) {
		add_string(index,p->wData[i]);
	}
}

USBString* Device::get_string(__u8 index,__u16 languageId) {
	if (index>maxStringIdx || index<0)  {return NULL;}
	if (!strings[index]) {return NULL;}
	if (!languageId&&index) {languageId=strings[0][0]->get_descriptor()->wData[0];}
	int i=0;
	i=0;
	while (true) {
		if (strings[index][i]) {
			if (strings[index][i]->get_languageId()==languageId) {
				return strings[index][i];
			}
		} else {
			return NULL;
		}
		i++;
	}
	return NULL;
}

USBString* Device::get_manufacturer_string(__u16 languageId) {
	if (!descriptor.iManufacturer) {return NULL;}
	return get_string(descriptor.iManufacturer,languageId?languageId:strings[0][0]->get_descriptor()->wData[0]);
}

USBString* Device::get_product_string(__u16 languageId) {
	if (!descriptor.iProduct) {return NULL;}
	return get_string(descriptor.iProduct,languageId?languageId:strings[0][0]->get_descriptor()->wData[0]);
}

USBString* Device::get_serial_string(__u16 languageId) {
	if (!descriptor.iSerialNumber) {return NULL;}
	return get_string(descriptor.iSerialNumber,languageId?languageId:strings[0][0]->get_descriptor()->wData[0]);
}

void Device::add_language(__u16 languageId) {
	int count=get_language_count();
	int i;
	const usb_string_descriptor* list=strings[0][0]->get_descriptor();
	for (i=0;i<count;i++) {
		if (languageId==list->wData[i]) {return;}
	}
	strings[0][0]->append_char(languageId);
}

__u16 Device::get_language_by_index(__u8 index) {
	if (index>=get_language_count() || index<0) {return 0;}
	return strings[0][0]->get_descriptor()->wData[index];
}

int Device::get_language_count() {
	return strings[0][0]->get_char_count();
}

Configuration* Device::get_active_configuration() {
	if (deviceConfigurationIndex<0) {return NULL;}
	return get_configuration(deviceConfigurationIndex);
}

void Device::set_active_configuration(__u8 index) {
	if (index>descriptor.bNumConfigurations || index<1) {deviceConfigurationIndex=-1;return;}
	deviceConfigurationIndex=index;
}

DeviceQualifier* Device::get_device_qualifier() {
	return qualifier;
}
void Device::set_device_qualifier(DeviceQualifier* _qualifier) {
	if (qualifier) {delete(qualifier);/* not needed qualifier=NULL; */}
	qualifier=_qualifier;
}
bool Device::is_highspeed() {
	return highspeed;
}

const definition_error Device::is_string_defined(__u8 index) {
	if (!index) {return definition_error();}
	if (!strings[index]) {
		return definition_error(DE_ERR_NULL_OBJECT,0x0, DE_OBJ_STRING,index);
	} else {
		int lc=get_language_count();
		int lngidx;
		for(lngidx=0;lngidx<lc;lngidx++) {
			__u16 langId=get_language_by_index(lngidx);
			bool found=false;
			int slidx=0;
			while (strings[index][slidx] && !found) {
				if (strings[index][slidx]->get_languageId()==langId) {found=true;}
				slidx++;
			}
			if (!found) {return definition_error(DE_ERR_MISSING_LANGUAGE,langId, DE_OBJ_STRING,index);}
		}
	}
	return definition_error();
}

const definition_error Device::is_defined() {
	if (descriptor.bLength!=18) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x01, DE_OBJ_DEVICE);}
	if (descriptor.bDescriptorType!=USB_DT_DEVICE) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x02, DE_OBJ_DEVICE);}
	//__le16 bcdUSB;
	//__u8  bDeviceClass;
	//__u8  bDeviceSubClass;
	//__u8  bDeviceProtocol;
	if (descriptor.bMaxPacketSize0!=8&&descriptor.bMaxPacketSize0!=16&&descriptor.bMaxPacketSize0!=32&&descriptor.bMaxPacketSize0!=64) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x08, DE_OBJ_DEVICE);}
	//__le16 idVendor;
	//__le16 idProduct;
	//__le16 bcdDevice;
	//__u8  iManufacturer;
	//__u8  iProduct;
	//__u8  iSerialNumber;
	if (!descriptor.bNumConfigurations) {return definition_error(DE_ERR_INVALID_DESCRIPTOR,0x12, DE_OBJ_DEVICE);}

	definition_error rc=definition_error();
	int i;
	for (i=0;i<descriptor.bNumConfigurations;i++) {
		if (!configurations[i]) {return definition_error(DE_ERR_NULL_OBJECT,0x0, DE_OBJ_CONFIG,i+1);}
		if (configurations[i]->get_descriptor()->bConfigurationValue!=(i+1)) {return definition_error(DE_ERR_MISPLACED_OBJECT,configurations[i]->get_descriptor()->bConfigurationValue, DE_OBJ_CONFIG,i+1);}
		rc=configurations[i]->is_defined(false);
		if (rc.error) {return rc;}
	}

	if (!strings[0]||!strings[0][0]) {return definition_error(DE_ERR_NULL_OBJECT,0x0, DE_OBJ_STRING,0);}
	int lc=get_language_count();
	if (!lc) {return definition_error(DE_ERR_MISSING_LANGUAGE,0x0, DE_OBJ_STRING,0);}

	rc=is_string_defined(descriptor.iManufacturer);
	if (rc.error) {return rc;}

	rc=is_string_defined(descriptor.iProduct);
	if (rc.error) {return rc;}

	rc=is_string_defined(descriptor.iSerialNumber);
	if (rc.error) {return rc;}

	for(i=0;i<descriptor.bNumConfigurations;i++) {
		rc=is_string_defined(configurations[i]->get_descriptor()->iConfiguration);
		if (rc.error) {return rc;}
		int j;
		for (j=0;j<configurations[i]->get_descriptor()->bNumInterfaces;j++) {
			int k;
			for (k=0;k<configurations[i]->get_interface_alternate_count(j);k++) {
				rc=is_string_defined(configurations[i]->get_interface_alternate(j,k)->get_descriptor()->iInterface);
				if (rc.error) {return rc;}
			}
		}
	}

	if (qualifier) {
		rc=qualifier->is_defined();
		if (rc.error) {return rc;}
		for(i=0;i<qualifier->get_descriptor()->bNumConfigurations;i++) {
			rc=is_string_defined(qualifier->get_configuration(i)->get_descriptor()->iConfiguration);
			if (rc.error) {return rc;}
		}
	}

	return definition_error();
}
