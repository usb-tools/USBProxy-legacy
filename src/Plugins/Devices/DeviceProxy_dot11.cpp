/*
 * This file is part of USBProxy.
 */

#include "DeviceProxy_dot11.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>

#include "HexString.h"
#include "Packet.h"
#include "TRACE.h"
#include "USBString.h"

// Find the right place to pull this in from
#define cpu_to_le16(x) (x)

#define STRING_MANUFACTURER 1
#define STRING_PRODUCT      2
#define STRING_SERIAL       3
#define STRING_DOT11	    4

extern "C" {
// lorcon.h defines
//   unsigned int from_ds, to_ds, protected, fragmented, retry;
// which ends up with an error because protected is a keyword.
// Use a hack to avoid that.
#define protected protected_flag
#include <lorcon2/lorcon.h>
#undef protected

static lorcon_driver_t *drvlist, *driver; // Needed to set up interface/context
static lorcon_t *context; // LORCON context
static USBString** dot11_strings;
static int dot11_stringMaxIndex;

	DeviceProxy_dot11::DeviceProxy_dot11(ConfigParser *cfg) {
		/* FIXME pull these values from the config object */
		int vendorId = 0xffff;
		int productId = 0x0005;
		interface = cfg->get("802.11_interface");
		
		p_is_connected = false;
		
		dot11_device_descriptor.bLength = USB_DT_DEVICE_SIZE;
		dot11_device_descriptor.bDescriptorType = USB_DT_DEVICE;
		dot11_device_descriptor.bcdUSB = cpu_to_le16(0x0100);
		dot11_device_descriptor.bDeviceClass = USB_CLASS_HID;
		dot11_device_descriptor.bDeviceSubClass = 0;
		dot11_device_descriptor.bDeviceProtocol = 0;
		dot11_device_descriptor.bMaxPacketSize0=64;
		dot11_device_descriptor.idVendor = cpu_to_le16(vendorId & 0xffff);
		dot11_device_descriptor.idProduct = cpu_to_le16(productId & 0xffff);
		fprintf(stderr,"V: %04x P: %04x\n",dot11_device_descriptor.idVendor,dot11_device_descriptor.idProduct);
		dot11_device_descriptor.bcdDevice = 0;
		dot11_device_descriptor.iManufacturer = STRING_MANUFACTURER;
		dot11_device_descriptor.iProduct = STRING_PRODUCT;
		dot11_device_descriptor.iSerialNumber = STRING_SERIAL;
		dot11_device_descriptor.bNumConfigurations = 1;
		
		dot11_config_descriptor.bLength = USB_DT_CONFIG_SIZE;
		dot11_config_descriptor.bDescriptorType = USB_DT_CONFIG;
		dot11_config_descriptor.bNumInterfaces = 1;
		dot11_config_descriptor.bConfigurationValue = 1;
		dot11_config_descriptor.iConfiguration = STRING_DOT11;
		dot11_config_descriptor.bmAttributes = USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER;
		dot11_config_descriptor.bMaxPower = 1;		/* self-powered */
		
		dot11_interface_descriptor.bLength = USB_DT_INTERFACE_SIZE;
		dot11_interface_descriptor.bDescriptorType = USB_DT_INTERFACE;
		dot11_interface_descriptor.bInterfaceNumber=0;
		dot11_interface_descriptor.bAlternateSetting=0;
		dot11_interface_descriptor.bNumEndpoints = 2;
		dot11_interface_descriptor.bInterfaceClass = USB_CLASS_HID;
		dot11_interface_descriptor.bInterfaceSubClass=0;
		dot11_interface_descriptor.bInterfaceProtocol=0;
		dot11_interface_descriptor.iInterface = STRING_DOT11;
	
		struct usb_endpoint_descriptor *ep;
		ep = &dot11_eps[0];
		ep->bLength = USB_DT_ENDPOINT_SIZE;
		ep->bDescriptorType = USB_DT_ENDPOINT;
		ep->bEndpointAddress = USB_ENDPOINT_DIR_MASK | 1;
		ep->bmAttributes = USB_ENDPOINT_XFER_BULK;
		ep->wMaxPacketSize = 64;
		ep->bInterval = 10;
		
		ep = &dot11_eps[1];
		ep->bLength = USB_DT_ENDPOINT_SIZE;
		ep->bDescriptorType = USB_DT_ENDPOINT;
		ep->bEndpointAddress = 2;
		ep->bmAttributes = USB_ENDPOINT_XFER_BULK;
		ep->wMaxPacketSize = 64;
		ep->bInterval = 10;
	
		dot11_config_descriptor.wTotalLength=dot11_config_descriptor.bLength+dot11_interface_descriptor.bLength+dot11_eps[0].bLength+dot11_eps[1].bLength;
	
		__u16 string0[2]={0x0409,0x0000};
		dot11_strings=(USBString**)calloc(5,sizeof(USBString*));
	
		dot11_strings[0]=new USBString(string0,0,0);
	
		dot11_strings[STRING_MANUFACTURER]=new USBString("USBProxy",STRING_MANUFACTURER,0x409);
		dot11_strings[STRING_PRODUCT]=new USBString("802.11 HID Device",STRING_PRODUCT,0x409);
		dot11_strings[STRING_SERIAL]=new USBString("0001",STRING_SERIAL,0x409);
		dot11_strings[STRING_DOT11]=new USBString("802.11",STRING_DOT11,0x409);
		dot11_stringMaxIndex=STRING_DOT11;
		
		monitor = false;
	}
	
	DeviceProxy_dot11::~DeviceProxy_dot11() {
		disconnect();
		free(dot11_strings);
	}
	
	int DeviceProxy_dot11::connect(int timeout) {
		if ( interface == ""  ) { 
			fprintf(stderr, "ERROR: 802.11 interface not set\n");
			return 1;
		}
		fprintf(stderr, "802.11: Using interface %s\n", interface.c_str());
		if ( (driver = lorcon_auto_driver(interface.c_str())) == NULL) {
			fprintf(stderr, "ERROR: 802.11: Could not determine the driver for %s\n",interface.c_str());
			return 1;
		} else {
			fprintf(stderr, "802.11:  Driver: %s\n",driver->name);
		}
	
		// Create LORCON context
		if ((context = lorcon_create(interface.c_str(), driver)) == nullptr) {
				fprintf(stderr, "Error: 802.11:  Failed to create context");
				return 1; 
		}
		int rv = lorcon_open_monitor(context);
		if (rv < 0) {
			fprintf(stderr, "Error: 802.11: Could not create Monitor Mode interface!\n");
			return 1;
		} else {
			fprintf(stderr, "802.11: Monitor Mode VAP: %s, (%d)\n",
					lorcon_get_vap(context), rv);
			monitor = true;
		}
		lorcon_free_driver_list(driver);
		
		p_is_connected = true;
		return 0;
	}
	
	void DeviceProxy_dot11::disconnect() {
		// Free the LORCON Context
		lorcon_free(context);
		p_is_connected = false;
	}
	
	void DeviceProxy_dot11::reset() {
	}
	
	bool DeviceProxy_dot11::is_connected() {
		return p_is_connected;
	}
	
	bool DeviceProxy_dot11::is_highspeed() {
		return false;
	}
	
	//return -1 to stall
	int DeviceProxy_dot11::control_request(const usb_ctrlrequest* setup_packet, int* nbytes, __u8* dataptr, int timeout) {
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
					memcpy(dataptr, &dot11_device_descriptor, dot11_device_descriptor.bLength);
					*nbytes = dot11_device_descriptor.bLength;
					break;
				case USB_DT_CONFIG:
					idx=setup_packet->wValue & 0xff;
					if (idx>=dot11_device_descriptor.bNumConfigurations) return -1;
					buf=(__u8*)malloc(dot11_config_descriptor.wTotalLength);
					p=buf;
					memcpy(p, &dot11_config_descriptor, dot11_config_descriptor.bLength);
					p+=dot11_config_descriptor.bLength;
					memcpy(p, &dot11_interface_descriptor, dot11_interface_descriptor.bLength);
					p+=dot11_interface_descriptor.bLength;
					memcpy(p, &dot11_eps[0], dot11_eps[0].bLength);
					p+=dot11_eps[0].bLength;
					memcpy(p, &dot11_eps[1], dot11_eps[1].bLength);
					*nbytes = dot11_config_descriptor.wTotalLength>setup_packet->wLength?setup_packet->wLength:dot11_config_descriptor.wTotalLength;
					memcpy(dataptr, buf, *nbytes);
					free(buf);
					break;
				case USB_DT_STRING:
					idx=setup_packet->wValue & 0xff;
					if (idx>0 && setup_packet->wIndex!=0x409) return -1;
					if (idx>dot11_stringMaxIndex) return -1;
					string_desc=dot11_strings[idx]->get_descriptor();
					*nbytes=string_desc->bLength>setup_packet->wLength?setup_packet->wLength:string_desc->bLength;
					memcpy(dataptr,string_desc,*nbytes);
					break;
				case USB_DT_DEVICE_QUALIFIER:
					return -1;
					break;
				case USB_DT_OTHER_SPEED_CONFIG:
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
		} else if (setup_packet->bRequestType & USB_TYPE_VENDOR) {
			/* These are our custom commands */
			rv = vendor_request(setup_packet, nbytes, dataptr, timeout);
		} else {
			fprintf(stderr,"Unhandled control request: 0x%02x, 0x%02x, %d, %d\n",
					setup_packet->bRequestType, setup_packet->bRequest,
					setup_packet->wValue, setup_packet->wIndex);
		}
		if (debugLevel>1 && nbytes) {
			char* hex=hex_string(dataptr, *nbytes);
			fprintf(stderr, "802.11> %s\n",hex);
			free(hex);
		}
		return 0;
	}
	
	int DeviceProxy_dot11::vendor_request(const usb_ctrlrequest* setup_packet, int* nbytes, __u8* dataptr, int timeout) {
		int rv, value;
		char *chr_res;
		uint8_t **mac;
		fprintf(stderr, "802.11> Handling vendor request (%d)\n", setup_packet->bRequest);
		switch (setup_packet->bRequest) {
			case DOT11_OPEN_INJECT:
				rv = lorcon_open_inject(context);
				if (rv < 0) {
					fprintf(stderr, "Error: 802.11: Could not create Injector interface!\n");
					return 1;
				} else {
					fprintf(stderr, "802.11: Injector VAP: %s\n",
							lorcon_get_vap(context));
					lorcon_free_driver_list(driver);
				}
				*(int *)(dataptr) = rv;
				*nbytes = 4;
				break;
			case DOT11_OPEN_MONITOR:
				rv = lorcon_open_monitor(context);
				if (rv < 0) {
					fprintf(stderr, "Error: 802.11: Could not create Monitor Mode interface!\n");
					return 1;
				} else {
					fprintf(stderr, "802.11: Monitor Mode VAP: %s, (%d)\n",
							lorcon_get_vap(context), rv);
					lorcon_free_driver_list(driver);
					monitor = true;
				}
				*(int *)(dataptr) = rv;
				*nbytes = 4;
				break;
			case DOT11_OPEN_INJMON:
				rv = lorcon_open_injmon(context);
				if (rv < 0) {
					fprintf(stderr, "Error: 802.11: Could not create Injector / Monitor Mode interface!\n");
					return 1;
				} else {
					fprintf(stderr, "802.11: Injector / Monitor Mode VAP: %s\n",
							lorcon_get_vap(context));
					lorcon_free_driver_list(driver);
					monitor = true;
				}
				*(int *)(dataptr) = rv;
				*nbytes = 4;
				break;
			case DOT11_SET_TIMEOUT:
				value = *(int *)(dataptr);
				lorcon_set_timeout(context, value);
				*nbytes = 0;
				break;
			case DOT11_GET_TIMEOUT:
				/* I have no idea if this is right... */
				rv = lorcon_get_timeout(context);
				*(int *)(dataptr) = rv;
				*nbytes = 4;
				break;
			case DOT11_GET_CAPIFACE:
				chr_res = (char *) lorcon_get_capiface(context);
				*nbytes = strlen(chr_res) + 1;
				fprintf(stderr, "CapIface: %s\n", chr_res);
				memcpy(dataptr, chr_res, *nbytes);
				break;
			case DOT11_GET_DRIVER_NAME:
				chr_res = (char *) lorcon_get_driver_name(context);
				*nbytes = strlen(chr_res);
				memcpy(dataptr, chr_res, *nbytes);
				break;
			case DOT11_CLOSE_INTERFACE:
				lorcon_close(context);
				*nbytes = 0;
				break;
			case DOT11_GET_DATALINK:
				rv = lorcon_get_datalink(context);
				*(int *)(dataptr) = rv;
				*nbytes = 4;
				break;
			case DOT11_SET_DATALINK:
				value = *(int *)(dataptr);
				rv = lorcon_set_datalink(context, value);
				*(int *)(dataptr) = rv;
				*nbytes = 4;
				break;
			case DOT11_GET_CHANNEL:
				rv = lorcon_get_channel(context);
				*(int *)(dataptr) = rv;
				*nbytes = 4;
				break;
			case DOT11_SET_CHANNEL:
				value = *(int *)(dataptr);
				rv = lorcon_set_channel(context, value);
				*(int *)(dataptr) = rv;
				*nbytes = 4;
				break;
			case DOT11_GET_HWMAC:
				*nbytes = lorcon_get_hwmac(context, (uint8_t **) &chr_res);
				memcpy(dataptr, chr_res, *nbytes);
				break;
			case DOT11_SET_HWMAC:
				rv = lorcon_set_hwmac(context, *nbytes, dataptr);
				*(int *)(dataptr) = rv;
				*nbytes = 4;
				break;
			case DOT11_ADD_WEPKEY:
				rv = lorcon_add_wepkey(context, dataptr, dataptr+6, *nbytes-6);
				*(int *)(dataptr) = rv;
				*nbytes = 4;
				break;
		}
	}
	
	/* Send data to the device */
	void DeviceProxy_dot11::send_data(__u8 endpoint,__u8 attributes, __u16 maxPacketSize, __u8* dataptr, int length) {
		//fprintf(stderr, "send_data\n");
	}
	
	/* Receive data from the device */
	void DeviceProxy_dot11::receive_data(__u8 endpoint,__u8 attributes, __u16 maxPacketSize, __u8** dataptr, int* length, int timeout) {
		//fprintf(stderr, "receive_data\n");
		int rv;
		lorcon_packet_t *packet;
		if(monitor) {
			rv = lorcon_next_ex(context, &packet);
			if(rv > 0) {
				dot11_packet_header_t *pkthdr;
//				pkthdr = (dot11_packet_header_t *) malloc(sizeof(dot11_packet_header_t));
				*dataptr = (__u8 *) malloc(sizeof(dot11_packet_header_t) + packet->length_header + packet->length_data);
				if(*dataptr==NULL)
					fprintf(stderr, "Uh oh, malloc failed\n");
				pkthdr  = (dot11_packet_header_t *) dataptr;
				pkthdr->tv_sec = packet->ts.tv_sec;
				pkthdr->tv_usec = packet->ts.tv_usec;
				pkthdr->dlt = packet->dlt;
				pkthdr->channel = packet->channel;
				pkthdr->length_capheader = packet->length_header;
				pkthdr->length_data = packet->length_data;
				fprintf(stderr, "Allocating %d + %d bytes (length=%d)\n",
						packet->length_header,
						packet->length_data,
						packet->length);
				//memcpy(*dataptr+sizeof(dot11_packet_header_t), packet->packet_header, packet->length_header);
				//memcpy(*dataptr+sizeof(dot11_packet_header_t)+packet->length_header,
				//	packet->packet_data,
				//	packet->length_data);
				*length = sizeof(dot11_packet_header_t) + packet->length_header + packet->length_data;
				free(packet);
			}
		}
	}
	
	void DeviceProxy_dot11::setConfig(Configuration* fs_cfg, Configuration* hs_cfg, bool hs) {
		;
	}
	
	void DeviceProxy_dot11::claim_interface(__u8 interface) {
		;
	}
	
	void DeviceProxy_dot11::release_interface(__u8 interface) {
		;
	}
	
	__u8 DeviceProxy_dot11::get_address() {
		return 1;
	}
	
	static DeviceProxy_dot11 *proxy;

	DeviceProxy * get_deviceproxy_plugin(ConfigParser *cfg) {
		proxy = new DeviceProxy_dot11(cfg);
		return (DeviceProxy *) proxy;
	}
	
	void destroy_plugin() {
		delete proxy;
	}
}

