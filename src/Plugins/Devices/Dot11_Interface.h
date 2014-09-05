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

#include <stdint.h>

enum dot11_usb_commands {
    DOT11_OPEN_INJECT      = 0,
    DOT11_OPEN_MONITOR     = 1,
    DOT11_OPEN_INJMON      = 2,
    DOT11_GET_TIMEOUT      = 3,
    DOT11_SET_TIMEOUT      = 4,
    DOT11_GET_CAPIFACE     = 5,
    DOT11_GET_DRIVER_NAME  = 6,
    DOT11_CLOSE_INTERFACE  = 7,
    DOT11_GET_DATALINK     = 8,
    DOT11_SET_DATALINK     = 9,
    DOT11_GET_CHANNEL      = 10,
    DOT11_SET_CHANNEL      = 11,
    DOT11_GET_HWMAC        = 12,
    DOT11_SET_HWMAC        = 13,
    DOT11_ADD_WEPKEY       = 14,
};

#define DOT11_VID 0xffff
#define DOT11_PID 0x0005

/* This struct will be sent at the start of a bulk trasnfer
 * followed by length bytes */
struct dot11_packet_header {
    int32_t tv_sec;
    int32_t tv_usec;
    int dlt;
    
    /* Channel we captured on or channel will tx on */
    int channel;
    
    /* Length of components */
    int length_capheader;
    int length_data;
};
typedef struct dot11_packet_header dot11_packet_header_t;

///* Get a pcap_t */
//pcap_t *lorcon_get_pcap(lorcon_t *context);
///* Return pcap selectable FD */
//int lorcon_get_selectable_fd(lorcon_t *context);
//
///* Fetch the next packet.  This is available on all sources, including 
// * those which do not present a pcap interface */
//int lorcon_next_ex(lorcon_t *context, lorcon_packet_t **packet);
//
///* Add a capture filter (if possible) using pcap bpf */
//int lorcon_set_filter(lorcon_t *context, const char *filter);
//
///* Add a precompiled filter (again, using bpf) */
//int lorcon_set_compiled_filter(lorcon_t *context, struct bpf_program *filter);
//
///* Pass through to pcap instance */
//int lorcon_loop(lorcon_t *context, int count, lorcon_handler callback, u_char *user);
//int lorcon_dispatch(lorcon_t *context, int count, lorcon_handler callback, u_char *user);
//void lorcon_breakloop(lorcon_t *context);
//
///* Inject a packet */
//int lorcon_inject(lorcon_t *context, lorcon_packet_t *packet);
//
///* Inject raw bytes */
//int lorcon_send_bytes(lorcon_t *context, int length, u_char *bytes);
//
//unsigned long int lorcon_get_version();

#endif /* USBPROXY_DOT11_INTERFACE_H */
