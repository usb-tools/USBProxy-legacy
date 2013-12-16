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
 * HexPrint.h
 *
 * Created on: Dec 3, 2013
 */
#ifndef USBPROXY_HEXSTRING_H
#define USBPROXY_HEXSTRING_H

#define TABPADDING "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
char* hex_string_wide(void* buf,int length,int width=32);
char* hex_string(void* buf,int length);

#endif /* USBPROXY_HEXSTRING_H */
