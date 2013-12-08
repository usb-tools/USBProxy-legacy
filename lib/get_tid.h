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
 * get_tid.h
 *
 * Created on: Dec 4, 2013
 */
#ifndef USBPROXY_GET_TID_H
#define USBPROXY_GET_TID_H

#include <unistd.h>
#include <sys/syscall.h>
#include <asm/unistd.h>

static long int gettid() {
	return (pid_t)syscall(__NR_gettid);
}

#endif /* USBPROXY_GET_TID_H */
