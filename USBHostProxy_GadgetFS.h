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
 * USBHostProxyGadgetFS.h
 *
 * Created on: Nov 21, 2013
 */
#ifndef USBHOSTPROXYGADGETFS_H_
#define USBHOSTPROXYGADGETFS_H_

extern "C" {
#include <linux/usb/gadgetfs.h>
}
#include "USBHostProxy.h"
#include <pthread.h>
#include <unistd.h>
#include <boost/atomic.hpp>
#include "TRACE.h"

struct async_read_data {
	pthread_t thread;
	int fd;
	boost::atomic_bool ready;
	int rc;
	int bufSize;
	__u8* buf;

	async_read_data(int _fd,int packetSize) : thread(0), fd(_fd), ready(false), rc(0), bufSize(packetSize), buf((__u8*)malloc(packetSize)) {}

	~async_read_data() {
		if (thread) {pthread_cancel(thread);thread=0;}
		if (buf) {free(buf);buf=NULL;}
		if (fd) {close(fd);fd=0;}
	}

	void start_read() {
		TRACE1(fd)
		ready=false;
		pthread_create(&thread,NULL,&async_read_data::do_read,this);
	}

	int finish_read(__u8** dataptr) {
		if (rc>0) {
			*dataptr=(__u8*)malloc(rc);
			memcpy(*dataptr,buf,rc);
		} else {
			*dataptr=NULL;
		}
		start_read();
		return rc;
	}

	static void* do_read(void* context) {
		async_read_data* ard=(async_read_data*)context;
		ard->rc=read(ard->fd,ard->buf,ard->bufSize);
		ard->ready=true;
		return 0;
	}
};

struct async_write_data {
	pthread_t thread;
	int fd;
	boost::atomic_bool ready;
	int rc;
	int bufSize;
	__u8* buf;

	async_write_data(int _fd) : thread(0), fd(_fd), ready(true), rc(0), bufSize(0), buf(NULL) {}

	~async_write_data() {
		if (thread) {pthread_cancel(thread);thread=0;}
		if (buf) {free(buf);buf=NULL;}
		if (fd) {close(fd);fd=0;}
	}

	void start_write(__u8* buf,int bufSize) {
		TRACE1(fd)
		ready=false;
		pthread_create(&thread,NULL,&async_write_data::do_write,this);
	}

	static void* do_write(void* context) {
		async_write_data* awd=(async_write_data*)context;
		awd->rc=write(awd->fd,awd->buf,awd->bufSize);
		awd->ready=true;
		return 0;
	}
};


class USBHostProxy_GadgetFS: public USBHostProxy {
private:
	bool p_is_connected;
	int p_device_file;
	int p_epin_file[16];
	struct async_read_data* p_epout_async[16];


	int debugLevel;

	char* descriptor;
	int descriptorLength;

	int reconnect();
	int generate_descriptor(USBDevice* device);

	usb_ctrlrequest lastControl;

public:
	USBHostProxy_GadgetFS(int _debugLevel=0);
	virtual ~USBHostProxy_GadgetFS();

	int connect(USBDevice* device);
	void disconnect();
	void reset();
	bool is_connected();

	//return 0 in usb_ctrlrequest->brequest if there is no request
	int control_request(usb_ctrlrequest *setup_packet, int *nbytes, __u8** dataptr);
	void send_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8* dataptr,int length);
	void receive_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8** dataptr, int* length);
	void control_ack();
	void stall_ep(__u8 endpoint);
	void setConfig(USBConfiguration* fs_cfg,USBConfiguration* hs_cfg,bool hs);
};

#endif /* USBHOSTPROXYGADGETFS_H_ */
