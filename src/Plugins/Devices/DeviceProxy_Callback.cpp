/*
 * This file is part of USBProxy.
 */

#include "DeviceProxy_Callback.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "Packet.h"
#include "HexString.h"
#include "TRACE.h"
#include "USBString.h"

// Find the right place to pull this in from
#define cpu_to_le16(x) (x)

#define BUF_LEN 100

int DeviceProxy_Callback::debugLevel = 1;

#define STRING_MANUFACTURER 1
#define STRING_PRODUCT      2
#define STRING_SERIAL       3
#define STRING_LOOPBACK     4

static USBString** callback_strings;
static int callback_stringMaxIndex;

DeviceProxy_Callback::DeviceProxy_Callback(int vendorId,int productId) {
	p_is_connected = false;
	
	callback_device_descriptor.bLength = USB_DT_DEVICE_SIZE;
	callback_device_descriptor.bDescriptorType = USB_DT_DEVICE;
	callback_device_descriptor.bcdUSB = cpu_to_le16(0x0100);
	callback_device_descriptor.bDeviceClass = USB_CLASS_VENDOR_SPEC;
	callback_device_descriptor.bDeviceSubClass = 0;
	callback_device_descriptor.bDeviceProtocol = 0;
	callback_device_descriptor.bMaxPacketSize0=64;
	callback_device_descriptor.idVendor = cpu_to_le16(vendorId & 0xffff);
	callback_device_descriptor.idProduct = cpu_to_le16(productId & 0xffff);
	fprintf(stderr,"V: %04x P: %04x\n",callback_device_descriptor.idVendor,callback_device_descriptor.idProduct);
	callback_device_descriptor.bcdDevice = 0;
	callback_device_descriptor.iManufacturer = STRING_MANUFACTURER;
	callback_device_descriptor.iProduct = STRING_PRODUCT;
	callback_device_descriptor.iSerialNumber = STRING_SERIAL;
	callback_device_descriptor.bNumConfigurations = 1;
	
	callback_config_descriptor.bLength = USB_DT_CONFIG_SIZE;
	callback_config_descriptor.bDescriptorType = USB_DT_CONFIG;
	callback_config_descriptor.bNumInterfaces = 1;
	callback_config_descriptor.bConfigurationValue = 1;
	callback_config_descriptor.iConfiguration = STRING_LOOPBACK;
	callback_config_descriptor.bmAttributes = USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER;
	callback_config_descriptor.bMaxPower = 1;		/* self-powered */
	
	callback_interface_descriptor.bLength = USB_DT_INTERFACE_SIZE;
	callback_interface_descriptor.bDescriptorType = USB_DT_INTERFACE;
	callback_interface_descriptor.bInterfaceNumber=0;
	callback_interface_descriptor.bAlternateSetting=0;
	callback_interface_descriptor.bNumEndpoints = 2;
	callback_interface_descriptor.bInterfaceClass = USB_CLASS_VENDOR_SPEC;
	callback_interface_descriptor.bInterfaceSubClass=0;
	callback_interface_descriptor.bInterfaceProtocol=0;
	callback_interface_descriptor.iInterface = STRING_LOOPBACK;

	struct usb_endpoint_descriptor *ep;
	ep = &callback_eps[0];
	ep->bLength = USB_DT_ENDPOINT_SIZE;
	ep->bDescriptorType = USB_DT_ENDPOINT;
	ep->bEndpointAddress = USB_ENDPOINT_DIR_MASK | 1;
	ep->bmAttributes = USB_ENDPOINT_XFER_INT;
	ep->wMaxPacketSize = 64;
	ep->bInterval = 10;
	
	ep = &callback_eps[1];
	ep->bLength = USB_DT_ENDPOINT_SIZE;
	ep->bDescriptorType = USB_DT_ENDPOINT;
	ep->bEndpointAddress = 1;
	ep->bmAttributes = USB_ENDPOINT_XFER_INT;
	ep->wMaxPacketSize = 64;
	ep->bInterval = 10;

	callback_config_descriptor.wTotalLength=callback_config_descriptor.bLength+callback_interface_descriptor.bLength+callback_eps[0].bLength+callback_eps[1].bLength;

	__u16 string0[2]={0x0409,0x0000};
	callback_strings=(USBString**)calloc(5,sizeof(USBString*));

	callback_strings[0]=new USBString(string0,0,0);

	callback_strings[STRING_MANUFACTURER]=new USBString("Manufacturer",STRING_MANUFACTURER,0x409);
	callback_strings[STRING_PRODUCT]=new USBString("Product",STRING_PRODUCT,0x409);
	callback_strings[STRING_SERIAL]=new USBString("Serial",STRING_SERIAL,0x409);
	callback_strings[STRING_LOOPBACK]=new USBString("Loopback",STRING_LOOPBACK,0x409);
	callback_stringMaxIndex=STRING_LOOPBACK;

	buffer=NULL;
	full=false;
	head=tail=0;
}

DeviceProxy_Callback::DeviceProxy_Callback(ConfigParser *cfg) {
	/* FIXME pull these values from the config object */
	int vendorId = 0xffff;
	int productId = 0xffff;

	p_is_connected = false;
	
	callback_device_descriptor.bLength = USB_DT_DEVICE_SIZE;
	callback_device_descriptor.bDescriptorType = USB_DT_DEVICE;
	callback_device_descriptor.bcdUSB = cpu_to_le16(0x0100);
	callback_device_descriptor.bDeviceClass = USB_CLASS_VENDOR_SPEC;
	callback_device_descriptor.bDeviceSubClass = 0;
	callback_device_descriptor.bDeviceProtocol = 0;
	callback_device_descriptor.bMaxPacketSize0=64;
	callback_device_descriptor.idVendor = cpu_to_le16(vendorId & 0xffff);
	callback_device_descriptor.idProduct = cpu_to_le16(productId & 0xffff);
	fprintf(stderr,"V: %04x P: %04x\n",callback_device_descriptor.idVendor,callback_device_descriptor.idProduct);
	callback_device_descriptor.bcdDevice = 0;
	callback_device_descriptor.iManufacturer = STRING_MANUFACTURER;
	callback_device_descriptor.iProduct = STRING_PRODUCT;
	callback_device_descriptor.iSerialNumber = STRING_SERIAL;
	callback_device_descriptor.bNumConfigurations = 1;
	
	callback_config_descriptor.bLength = USB_DT_CONFIG_SIZE;
	callback_config_descriptor.bDescriptorType = USB_DT_CONFIG;
	callback_config_descriptor.bNumInterfaces = 1;
	callback_config_descriptor.bConfigurationValue = 1;
	callback_config_descriptor.iConfiguration = STRING_LOOPBACK;
	callback_config_descriptor.bmAttributes = USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER;
	callback_config_descriptor.bMaxPower = 1;		/* self-powered */
	
	callback_interface_descriptor.bLength = USB_DT_INTERFACE_SIZE;
	callback_interface_descriptor.bDescriptorType = USB_DT_INTERFACE;
	callback_interface_descriptor.bInterfaceNumber=0;
	callback_interface_descriptor.bAlternateSetting=0;
	callback_interface_descriptor.bNumEndpoints = 2;
	callback_interface_descriptor.bInterfaceClass = USB_CLASS_VENDOR_SPEC;
	callback_interface_descriptor.bInterfaceSubClass=0;
	callback_interface_descriptor.bInterfaceProtocol=0;
	callback_interface_descriptor.iInterface = STRING_LOOPBACK;

	struct usb_endpoint_descriptor *ep;
	ep = &callback_eps[0];
	ep->bLength = USB_DT_ENDPOINT_SIZE;
	ep->bDescriptorType = USB_DT_ENDPOINT;
	ep->bEndpointAddress = USB_ENDPOINT_DIR_MASK | 1;
	ep->bmAttributes = USB_ENDPOINT_XFER_INT;
	ep->wMaxPacketSize = 64;
	ep->bInterval = 10;
	
	ep = &callback_eps[1];
	ep->bLength = USB_DT_ENDPOINT_SIZE;
	ep->bDescriptorType = USB_DT_ENDPOINT;
	ep->bEndpointAddress = 1;
	ep->bmAttributes = USB_ENDPOINT_XFER_INT;
	ep->wMaxPacketSize = 64;
	ep->bInterval = 10;

	callback_config_descriptor.wTotalLength=callback_config_descriptor.bLength+callback_interface_descriptor.bLength+callback_eps[0].bLength+callback_eps[1].bLength;

	__u16 string0[2]={0x0409,0x0000};
	callback_strings=(USBString**)calloc(5,sizeof(USBString*));

	callback_strings[0]=new USBString(string0,0,0);

	callback_strings[STRING_MANUFACTURER]=new USBString("Manufacturer",STRING_MANUFACTURER,0x409);
	callback_strings[STRING_PRODUCT]=new USBString("Product",STRING_PRODUCT,0x409);
	callback_strings[STRING_SERIAL]=new USBString("Serial",STRING_SERIAL,0x409);
	callback_strings[STRING_LOOPBACK]=new USBString("Loopback",STRING_LOOPBACK,0x409);
	callback_stringMaxIndex=STRING_LOOPBACK;

	buffer=NULL;
	full=false;
	head=tail=0;
}

DeviceProxy_Callback::~DeviceProxy_Callback() {
	disconnect();

	int i;
	if (callback_strings) {
		for (i=0;i<callback_stringMaxIndex;i++) {
			if (callback_strings[i]) {
				delete(callback_strings[i]);
				callback_strings[i]=NULL;
			}
		}
		delete callback_strings;
	}
}

int DeviceProxy_Callback::connect(int timeout) {
	if(connect_cb)
		return connect_cb(timeout);
}

void DeviceProxy_Callback::disconnect() {
	if(disconnect_cb)
		disconnect_cb();
}

void DeviceProxy_Callback::reset() {
	if(reset_cb)
		reset_cb();
}

bool DeviceProxy_Callback::is_connected() {
	return p_is_connected;
}

bool DeviceProxy_Callback::is_highspeed() {
	return false;
}

int DeviceProxy_Callback::control_request(const usb_ctrlrequest* setup_packet, int* nbytes, __u8* dataptr, int timeout) {
	if(control_request_cb)
		return control_request_cb(setup_packet, nbytes, dataptr, timeout);
}

void DeviceProxy_Callback::send_data(__u8 endpoint,__u8 attributes, __u16 maxPacketSize, __u8* dataptr, int length) {
	if(send_data_cb)
		send_data_cb(endpoint, attributes, maxPacketSize, dataptr, length);
}

void DeviceProxy_Callback::receive_data(__u8 endpoint,__u8 attributes, __u16 maxPacketSize, __u8** dataptr, int* length, int timeout) {
	if(receive_data_cb)
		receive_data_cb(endpoint, attributes, maxPacketSize, dataptr, length, timeout);

}

void DeviceProxy_Callback::setConfig(Configuration* fs_cfg, Configuration* hs_cfg, bool hs) {
	;
}

void DeviceProxy_Callback::claim_interface(__u8 interface) {
	;
}

void DeviceProxy_Callback::release_interface(__u8 interface) {
	;
}

__u8 DeviceProxy_Callback::get_address() {
	return 1;
}

char* DeviceProxy_Callback::toString() {
	if(toString_cb)
		return toString_cb();
	else
		return (char *) "Callback device";
}

static DeviceProxy_Callback *proxy;

extern "C" {
	DeviceProxy * get_deviceproxy_plugin(ConfigParser *cfg) {
		proxy = new DeviceProxy_Callback(cfg);
		return (DeviceProxy *) proxy;
	}
	
	void destroy_plugin() {
		delete proxy;
	}
}
