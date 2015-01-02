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

int DeviceProxy_Callback::debugLevel = 2;

#define STRING_MANUFACTURER 1
#define STRING_PRODUCT      2
#define STRING_SERIAL       3
#define STRING_LOOPBACK     4

static USBString** callback_strings;
static int callback_stringMaxIndex;

DeviceProxy_Callback::DeviceProxy_Callback(ConfigParser *cfg) {
	 connect_cb = (f_connect) cfg->get_pointer("DeviceProxy_Callback::connect");
	 disconnect_cb = (f_disconnect) cfg->get_pointer("DeviceProxy_Callback::disconnect");
	 reset_cb = (f_reset) cfg->get_pointer("DeviceProxy_Callback::reset");
	 control_request_cb = (f_control_request) cfg->get_pointer("DeviceProxy_Callback::control_request");
	 send_data_cb = (f_send_data) cfg->get_pointer("DeviceProxy_Callback::send_data");
	 receive_data_cb = (f_receive_data) cfg->get_pointer("DeviceProxy_Callback::receive_data");
	 toString_cb = (f_toString) cfg->get_pointer("DeviceProxy_Callback::toString");
	 
	/* Copy descriptors */
	void *device_desc = cfg->get_pointer("DeviceProxy_Callback::device_descriptor");
	fprintf(stderr, "device ptr: 0x%p\n", device_desc);
	fprintf(stderr, "_callback_device_descriptor: %d\n", (*(struct usb_device_descriptor *) device_desc).bDeviceClass);
	struct usb_config_descriptor *_callback_config_descriptor = (struct usb_config_descriptor *) cfg->get_pointer("DeviceProxy_Callback::config_descriptor");
	struct usb_interface_descriptor *_callback_interface_descriptor = (struct usb_interface_descriptor *) cfg->get_pointer("DeviceProxy_Callback::interface_descriptor");
	struct usb_endpoint_descriptor *_callback_eps = (struct usb_endpoint_descriptor *) cfg->get_pointer("DeviceProxy_Callback::endpoint_descriptor");

	if(device_desc)
		memcpy(&callback_device_descriptor, device_desc, sizeof(struct usb_device_descriptor));
	if(_callback_config_descriptor)
		memcpy(&callback_config_descriptor, _callback_config_descriptor, sizeof(struct usb_config_descriptor));
	if(_callback_interface_descriptor)
		memcpy(&callback_interface_descriptor, _callback_interface_descriptor, sizeof(struct usb_interface_descriptor));
	if(_callback_eps) {
		memcpy(&callback_eps, _callback_eps, sizeof(struct usb_endpoint_descriptor));
		memcpy((&callback_eps)+sizeof(struct usb_endpoint_descriptor), _callback_eps, sizeof(struct usb_endpoint_descriptor));
	}
	__u16 string0[2]={0x0409,0x0000};
	callback_strings=(USBString**)calloc(5,sizeof(USBString*));

	callback_strings[0]=new USBString(string0,0,0);

	callback_strings[STRING_MANUFACTURER]=new USBString("Manufacturer",STRING_MANUFACTURER,0x409);
	callback_strings[STRING_PRODUCT]=new USBString("Product",STRING_PRODUCT,0x409);
	callback_strings[STRING_SERIAL]=new USBString("Serial",STRING_SERIAL,0x409);
	callback_strings[STRING_LOOPBACK]=new USBString("Callback",STRING_LOOPBACK,0x409);
	callback_stringMaxIndex=STRING_LOOPBACK;

	p_is_connected = false;
}

DeviceProxy_Callback::~DeviceProxy_Callback() {
	disconnect();
	delete[] callback_strings;
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
	//if(control_request_cb)
	//	return control_request_cb(setup_packet, nbytes, dataptr, timeout);
	int rv = 0;
	if (debugLevel>1) {
		char* hex=hex_string((void*)setup_packet,sizeof(*setup_packet));
		fprintf(stderr, "802.11< %s\n",hex);
		free(hex);
	}
	if((setup_packet->bRequestType & USB_DIR_IN) && setup_packet->bRequest == USB_REQ_GET_DESCRIPTOR) {
		__u8* buf;
		__u8* p;
		__u8 idx;
		const usb_string_descriptor* string_desc;
	
		switch ((setup_packet->wValue)>>8) {
			case USB_DT_DEVICE:
				TRACE
				memcpy(dataptr, &callback_device_descriptor, callback_device_descriptor.bLength);
				*nbytes = callback_device_descriptor.bLength;
				break;
			case USB_DT_CONFIG:
				TRACE
				idx=setup_packet->wValue & 0xff;
				if (idx>=callback_device_descriptor.bNumConfigurations) return -1;
				fprintf(stderr, "callback_config_descriptor> 0x%x\n",callback_config_descriptor.wTotalLength);
				buf=(__u8*)malloc(callback_config_descriptor.wTotalLength);
				p=buf;
				memcpy(p, &callback_config_descriptor, callback_config_descriptor.bLength);
				p+=callback_config_descriptor.bLength;
				memcpy(p, &callback_interface_descriptor, callback_interface_descriptor.bLength);
				p+=callback_interface_descriptor.bLength;
				memcpy(p, &callback_eps[0], callback_eps[0].bLength);
				p+=callback_eps[0].bLength;
				memcpy(p, &callback_eps[1], callback_eps[1].bLength);
				*nbytes = callback_config_descriptor.wTotalLength>setup_packet->wLength?setup_packet->wLength:callback_config_descriptor.wTotalLength;
				TRACE1(*nbytes)
				memcpy(dataptr, buf, *nbytes);
				free(buf);
				break;
			case USB_DT_STRING:
				TRACE
				idx=setup_packet->wValue & 0xff;
				if (idx>0 && setup_packet->wIndex!=0x409) return -1;
				if (idx>callback_stringMaxIndex) return -1;
				string_desc=callback_strings[idx]->get_descriptor();
				*nbytes=string_desc->bLength>setup_packet->wLength?setup_packet->wLength:string_desc->bLength;
				memcpy(dataptr,string_desc,*nbytes);
				break;
			case USB_DT_DEVICE_QUALIFIER:
				TRACE
				return -1;
				break;
			case USB_DT_OTHER_SPEED_CONFIG:
				TRACE
				return -1;
				break;
		}
	} else if (setup_packet->bRequest==USB_REQ_GET_CONFIGURATION){
		dataptr[0]=1;
		*nbytes=1;
	} else if (setup_packet->bRequest==USB_REQ_SET_CONFIGURATION){
		fprintf(stderr, "Setting config %d (As if that does anything)\n",
				setup_packet->wValue);
	} else if (setup_packet->bRequest==USB_REQ_GET_INTERFACE){
		dataptr[0]=1;
		*nbytes=1;
	//} else if (setup_packet->bRequestType & USB_TYPE_VENDOR) {
		/* These are our custom commands */
		//rv = vendor_request(setup_packet, nbytes, dataptr, timeout);
	} else {
		fprintf(stderr,"Unhandled control request: 0x%02x, 0x%02x, %d, %d\n",
				setup_packet->bRequestType, setup_packet->bRequest,
				setup_packet->wValue, setup_packet->wIndex);
	}
	if (debugLevel>1 && nbytes) {
		char* hex=hex_string(dataptr, *nbytes);
		fprintf(stderr, "Callback> %s\n",hex);
		free(hex);
	}
	return 0;
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
