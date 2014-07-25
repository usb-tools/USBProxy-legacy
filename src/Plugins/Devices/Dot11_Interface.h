/*
 * Copyright 2014 Dominic Spill
 * Copyright 2014 Mike Kershaw / dragorn
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
 */

#ifndef USBPROXY_DOT11_INTERFACE_H
#define USBPROXY_DOT11_INTERFACE_H

enum dot11_usb_commands {
    DOT11_OPEN_INJECT      = 0,
    DOT11_OPEN_MONITOR     = 1,
    DOT11_OPEN_INJMON      = 2,
    DOT11_SET_TIMEOUT      = 3,
    DOT11_GET_TIMEOUT      = 4,
    DOT11_GET_CAPIFACE     = 5,
    
};




/* Set a capture timeout (equivalent to the timeout value in pcap_open,
 * but with implications for non-pcap sources as well).  Timeout value 
 * is in ms */
void lorcon_set_timeout(lorcon_t *context, int in_timeout);
int lorcon_get_timeout(lorcon_t *context);

/* Open an interface for inject */
int lorcon_open_inject(lorcon_t *context);

/* Open an interface in monitor mode (may also enable injection) */
int lorcon_open_monitor(lorcon_t *context);

/* Open an interface in inject+monitor mode */
int lorcon_open_injmon(lorcon_t *context);

/* Get the interface we're capturing from */
const char *lorcon_get_capiface(lorcon_t *context);

/* Get the driver */
const char *lorcon_get_driver_name(lorcon_t *context);

/* Close interface */
void lorcon_close(lorcon_t *context);

/* Datalink layer info */
int lorcon_get_datalink(lorcon_t *context);
int lorcon_set_datalink(lorcon_t *context, int dlt);

/* Get/set channel/frequency */
int lorcon_set_channel(lorcon_t *context, int channel);
int lorcon_get_channel(lorcon_t *context);

/* Get/set MAC address, returns length of MAC and allocates in **mac,
 * caller is responsible for freeing this memory.  Different PHY types
 * may have different MAC lengths. 
 *
 * For 802.11, MAC is always 6 bytes.
 *
 * A length of 0 indicates no set MAC on this PHY.  Negative numbers
 * indicate error fetching MAC from hardware. */
int lorcon_get_hwmac(lorcon_t *context, uint8_t **mac);
/* Set a MAC, if the PHY supports it.  Negative on failure.  For 802.11,
 * MAC must always be 6 bytes. */
int lorcon_set_hwmac(lorcon_t *context, int mac_len, uint8_t *mac);

/* Get a pcap_t */
pcap_t *lorcon_get_pcap(lorcon_t *context);

/* Return pcap selectable FD */
int lorcon_get_selectable_fd(lorcon_t *context);

/* Fetch the next packet.  This is available on all sources, including 
 * those which do not present a pcap interface */
int lorcon_next_ex(lorcon_t *context, lorcon_packet_t **packet);

/* Add a capture filter (if possible) using pcap bpf */
int lorcon_set_filter(lorcon_t *context, const char *filter);

/* Add a precompiled filter (again, using bpf) */
int lorcon_set_compiled_filter(lorcon_t *context, struct bpf_program *filter);

/* Pass through to pcap instance */
int lorcon_loop(lorcon_t *context, int count, lorcon_handler callback, u_char *user);
int lorcon_dispatch(lorcon_t *context, int count, lorcon_handler callback, u_char *user);
void lorcon_breakloop(lorcon_t *context);

/* Inject a packet */
int lorcon_inject(lorcon_t *context, lorcon_packet_t *packet);

/* Inject raw bytes */
int lorcon_send_bytes(lorcon_t *context, int length, u_char *bytes);

unsigned long int lorcon_get_version();

/* Add a wep key to a context */
int lorcon_add_wepkey(lorcon_t *context, u_char *bssid, u_char *key, int length);

#endif /* USBPROXY_DOT11_INTERFACE_H */