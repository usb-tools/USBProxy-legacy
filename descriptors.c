/*
 * Copyright 2013 Dominic Spill
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

static struct endpoint_item *ep_tail = NULL;

/* We're going to need the endpoints again, so store them in a list */
int append_endpoint(struct usb_endpoint_descriptor *ep) {
	struct endpoint_item *ep_item;
	
	if(ep_tail == NULL)
		ep_item = ep_head;
	else
		ep_item = ep_tail->next;
		
	ep_item = malloc(sizeof(struct endpoint_item));
	if(ep_item == NULL) {
		fprintf(stderr, "Out of memory - unable to allocate %d bytes", sizeof(struct endpoint_item));
		return -1;
	}
	memcpy(ep, &ep_item->desc, USB_DT_ENDPOINT_SIZE);
	ep_tail = ep_item;

	ep_list_len++;
	return 0;
}

/* Clone a config descriptor + associated interfaces and endpoints */
static char *
clone_config(char *cp, struct libusb_config_descriptor *config, int highspeed)
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

	/* interfaces */
	for(i=0; i < config->bNumInterfaces; i++) {
		/* No endianness issues here, all fields are u8s */
		for(j=0; j < config->interface->num_altsetting; j++) {
			iface = &config->interface->altsetting[j];
			memcpy(cp, iface, USB_DT_INTERFACE_SIZE);
			cp += USB_DT_INTERFACE_SIZE;

			/* endpoints */
			for (k = 0; k < iface->bNumEndpoints; k++) {
				libusb_ep = &iface->endpoint[k];
				memcpy (cp, libusb_ep, USB_DT_ENDPOINT_SIZE);
				ep = (struct usb_endpoint_descriptor *) cp;
				ep->wMaxPacketSize = __cpu_to_le16(libusb_ep->wMaxPacketSize);
				cp += USB_DT_ENDPOINT_SIZE;
				if(!highspeed)
					append_endpoint(ep);
			}
		}
	}
	c->wTotalLength = __cpu_to_le16(cp - (char *) c);
	return cp;
}

/* Copy config from given vId/pId device */
int clone_descriptors(__u16 vendorId, __u16 productId, libusb_device_handle* devh, char *buf) {
	char *cp;
	libusb_device* dev;
	struct libusb_device_descriptor slave_desc;
	struct usb_device_descriptor *device_desc;
	struct libusb_config_descriptor *config;
	int i, r, hs;

	ep_list_len = 0;
	cp = &buf[0];
	/* tag for this descriptor format */
	*(__u32 *)cp = 0;
	cp += 4;

	if(devh != NULL) {
		/* Copy configuration in to gadget structs */
		dev = libusb_get_device(devh);
		r = libusb_get_device_descriptor(dev, &slave_desc);
		if(r<0) {
			show_libusb_error(r);
			return r;
		}

		/* This is silly! Copy endpoints a second time if we're using a
		 * high speed controller - which we are */
		for(hs=0; hs < 2; hs++) {
			for(i=0; i<slave_desc.bNumConfigurations; ++i) {
				r = libusb_get_config_descriptor(dev, i, &config);
				if(r<0) {
					show_libusb_error(r);
					return r;
				}
				cp = clone_config(cp, config, hs);
			}
		}

		libusb_free_config_descriptor(config);

		/* device descriptor is the last descriptor */
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