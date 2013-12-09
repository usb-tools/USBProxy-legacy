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
 * Criteria.cpp
 *
 * Created on: Dec 9, 2013
 */

#include "Criteria.h"

#include "Device.h"
#include "Configuration.h"
#include "Interface.h"
#include "Endpoint.h"

bool criteria_endpoint::test(Endpoint* endpoint) {
	const usb_endpoint_descriptor* desc=endpoint->get_descriptor();
	if ((addressMask&address)!=(addressMask&desc->bEndpointAddress)) {return false;}
	if ((attributesMask&attributes)!=(attributesMask&desc->bmAttributes)) {return false;}
	if (packetSizeMin>desc->wMaxPacketSize || packetSizeMax<desc->wMaxPacketSize) {return false;}
	if (intervalMin>desc->bInterval || intervalMax<desc->bInterval) {return false;}
	return true;
}

bool criteria_interface::test(Interface* interface) {
	const usb_interface_descriptor* desc=interface->get_descriptor();
	if (number!=-1 && number!=desc->bInterfaceNumber) {return false;}
	if (alternate!=-1 && alternate!=desc->bAlternateSetting) {return false;}
	if (deviceClass!=-1 && deviceClass!=desc->bInterfaceClass) {return false;}
	if (subClass!=-1 && subClass!=desc->bInterfaceSubClass) {return false;}
	if (protocol!=-1 && protocol!=desc->bInterfaceProtocol) {return false;}
	return true;
}
bool criteria_configuration::test(Configuration* configuration) {
	const usb_config_descriptor* desc=configuration->get_descriptor();
	if (number!=-1 && number!=desc->bConfigurationValue) {return false;}
	if ((attributesMask&attributes)!=(attributesMask&desc->bmAttributes)) {return false;}
	return true;
}
bool criteria_device::test(Device* device) {
	const usb_device_descriptor* desc=device->get_descriptor();
	if (deviceClass!=-1 && deviceClass!=desc->bDeviceClass) {return false;}
	if (subClass!=-1 && subClass!=desc->bDeviceSubClass) {return false;}
	if (protocol!=-1 && protocol!=desc->bDeviceProtocol) {return false;}
	if (ep0packetSizeMin>desc->bMaxPacketSize0 || ep0packetSizeMax<desc->bMaxPacketSize0) {return false;}
	if (vendor!=-1 && vendor!=desc->idVendor) {return false;}
	if (product!=-1 && product!=desc->idProduct) {return false;}
	if (release!=-1 && release!=desc->bcdDevice) {return false;}
	return true;
}





