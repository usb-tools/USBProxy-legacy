/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
 *
 * This file is part of USBProxy.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
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
 * PacketFilter_KeyLoggger.cpp
 *
 * Created on: Nov 23, 2013
 */
#include "PacketFilter_Power.h"
#include <linux/usb/ch9.h>

PacketFilter_Power::PacketFilter_Power() {
}

void PacketFilter_Power::filter_setup_packet(SetupPacket* packet, bool direction_in) {
	struct usb_config_descriptor *config;

	if (direction_in
		&& (packet->ctrl_req.bRequest == USB_REQ_GET_DESCRIPTOR)
		&& (packet->ctrl_req.wValue == USB_DT_CONFIG)) {
		config = (struct usb_config_descriptor *) packet->data;
		config->bmAttributes = USB_CONFIG_ATT_ONE;
		config->bMaxPower = 0x32; /* 100mA */
	}
}
