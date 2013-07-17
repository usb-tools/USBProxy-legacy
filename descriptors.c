/*
 * Copyright 2013 Dominic Spill
 *
 * This file is part of USB Proxy.
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
 */

#include "descriptors.h"

void show_libusb_error(int error_code)
{
	char *error_hint = "";
	const char *error_name;

	/* Available only in libusb > 1.0.3 */
	// error_name = libusb_error_name(error_code);

	switch (error_code) {
		case LIBUSB_ERROR_TIMEOUT:
			error_name="Timeout";
			break;
		case LIBUSB_ERROR_NO_DEVICE:
			error_name="No Device";
			error_hint="Check device is connected to host";
			break;
		case LIBUSB_ERROR_ACCESS:
			error_name="Insufficient Permissions";
			break;
		default:
			error_name="Command Error";
			break;
	}

	fprintf(stderr,"libUSB Error: %s: %s (%d)\n", error_name, error_hint, error_code);
}

static inline int max(__u8 a, __u8 b)
{
	return (a > b) ? a : b;
}

static __u8 max_index;

int fetch_strings(libusb_device_handle *dev) {
	int idx, len;
	struct usb_string *str;
	unsigned char data[255];
	char *strtab;

	len = libusb_get_string_descriptor(dev, 0, 0, data, 255);
	if(len >= 4) {
		strings.language = data[2]<<8 | data[3];

		strings.strings = malloc(max_index * sizeof(struct usb_string));
		if(strings.strings == NULL) {
			fprintf(stderr, "Out of memory - unable to allocate %d bytes", max_index * sizeof(struct usb_string));
			return -1;
		}

		str = (struct usb_string *) strings.strings;
		for(idx=1; idx <= max_index; idx++) {
			len = libusb_get_string_descriptor(dev, idx, strings.language,
											   data, 255);
			if(len > 0) {
				strtab = malloc(len);
				if(strtab == NULL) {
					fprintf(stderr, "Out of memory - unable to allocate %d bytes", len);
					return -1;
				}
				memcpy(&data, strtab, len);
				str->s = strtab;
				str->id = idx;
			}
			str++;
		}
	}
	return max_index;
}

static char *
clone_config(char *cp, struct libusb_config_descriptor *config)
{
	const struct libusb_interface_descriptor *iface;
	const struct libusb_endpoint_descriptor *libusb_ep;
	struct usb_config_descriptor *c;
	struct usb_endpoint_descriptor *ep;
	int i, j, k;

	c = (struct usb_config_descriptor *) cp;

	/* Don't worry about endianness of wTotalLength, will overwrite it below */
	memcpy(cp, config, USB_DT_CONFIG_SIZE);
	cp += USB_DT_CONFIG_SIZE;

	for(i=0; i < config->bNumInterfaces; i++) {
		/* No endianness issues here, all fields are u8s */
		for(j=0; j < config->interface->num_altsetting; j++) {
			iface = &config->interface->altsetting[j];
			memcpy(cp, iface, USB_DT_INTERFACE_SIZE);
			cp += USB_DT_INTERFACE_SIZE;

			for (k = 0; k < iface->bNumEndpoints; k++) {
				libusb_ep = &iface->endpoint[k];
				memcpy (cp, libusb_ep, USB_DT_ENDPOINT_SIZE);
				ep = (struct usb_endpoint_descriptor *) cp;
				ep->wMaxPacketSize = __cpu_to_le16(libusb_ep->wMaxPacketSize);
				cp += USB_DT_ENDPOINT_SIZE;
			}
		}
	}
	c->wTotalLength = __cpu_to_le16(cp - (char *) c);
	return cp;
}

/* Copy config from given vId/pId device */
int clone_descriptors(__u16 vendorId, __u16 productId, char *buf) {
	char *cp;
	libusb_device* dev;
	libusb_device_handle* slave_devh;
	struct libusb_device_descriptor slave_desc;
	struct usb_device_descriptor *device_desc;
	struct libusb_config_descriptor *config;
	int i, r, silly;
	
	cp = &buf[0];
	/* tag for this descriptor format */
	*(__u32 *)cp = 0;
	cp += 4;

	libusb_init(NULL);
	libusb_set_debug(NULL, debug);
	if((slave_devh = libusb_open_device_with_vid_pid(NULL, vendorId, productId))
	   != NULL) {
		/* Copy configuration in to gadget structs */
		dev = libusb_get_device(slave_devh);
		r = libusb_get_device_descriptor(dev, &slave_desc);
		if(r<0) {
			show_libusb_error(r);
			return r;
		}

		/* This is silly! Copy endpoints a second time if we're using a
		 * high speed controller - which we are */
		for(silly=0; silly < 2; silly++) {
			for(i=0; i<slave_desc.bNumConfigurations; ++i) {
				r = libusb_get_config_descriptor(dev, i, &config);
				if(r<0) {
					show_libusb_error(r);
					return r;
				}
				cp = clone_config(cp, config);
			}
		}

		libusb_free_config_descriptor(config);

		/* and device descriptor at the end */
		device_desc = (struct usb_device_descriptor *) cp;
		memcpy (cp, &slave_desc, USB_DT_DEVICE_SIZE);
		cp += USB_DT_DEVICE_SIZE;

		/* Fix endianness on u16 fields */
		device_desc->bcdUSB = __cpu_to_le16(slave_desc.bcdUSB);
		device_desc->idVendor = __cpu_to_le16(slave_desc.idVendor);
		device_desc->idProduct = __cpu_to_le16(slave_desc.idProduct);
		device_desc->bcdDevice = __cpu_to_le16(slave_desc.bcdDevice);

		return cp - &buf[0];
	}
	return -1;

}