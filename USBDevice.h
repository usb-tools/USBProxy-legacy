#ifndef _USBDevice_
#define _USBDevice_

#include <linux/usb/ch9.h>
#include "USBDeviceProxy.h"

extern "C" class USBDevice {
	private:
		usb_device_descriptor descriptor;
		__u8 activeConfigurationIndex=0;
    __u8 address;

		//USBConfiguration activeConfiguration;
    //USBVendor deviceVendor;
		//vector(of USBConfiguration) configurations;
		//vector(of endpoints) endpoints;

	public:
		USBDevice(USBDeviceProxy* proxy);
		USBDevice(usb_device_descriptor _descriptor);
		USBDevice(__le16 bcdUSB,	__u8  bDeviceClass,	__u8  bDeviceSubClass,	__u8  bDeviceProtocol,	__u8  bMaxPacketSize0,	__le16 idVendor,	__le16 idProduct,	__le16 bcdDevice,	__u8  iManufacturer,	__u8  iProduct,	__u8  iSerialNumber,	__u8  bNumConfigurations);
		usb_device_descriptor getDescriptor();
};

#endif