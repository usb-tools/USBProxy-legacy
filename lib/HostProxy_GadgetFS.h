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
 * HostProxyGadgetFS.h
 *
 * Created on: Nov 21, 2013
 */
#ifndef USBPROXY_HOSTPROXY_GADGETFS_H
#define USBPROXY_HOSTPROXY_GADGETFS_H

extern "C" {
#include <linux/usb/gadgetfs.h>
}
#include "HostProxy.h"
#include <pthread.h>
#include <unistd.h>
#include <boost/atomic.hpp>
#include "TRACE.h"
#include "errno.h"
#include "aio.h"

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
	int awd_errno;
	int bufSize;
	__u8* buf;

	async_write_data(int _fd) : thread(0), fd(_fd), ready(true), rc(0), awd_errno(0), bufSize(0), buf(NULL) {}

	~async_write_data() {
		if (thread) {pthread_cancel(thread);thread=0;}
		if (buf) {free(buf);buf=NULL;}
		if (fd) {close(fd);fd=0;}
	}

	void start_write(__u8* inBuf,int inBufSize) {
		ready=false;
		bufSize=inBufSize;
		buf=(__u8*)malloc(bufSize);
		memcpy(buf,inBuf,bufSize);
		pthread_create(&thread,NULL,&async_write_data::do_write,this);
	}

	int finish_write() {
		int old_rc=rc;
		rc=0;
		return old_rc;
	}

	static void* do_write(void* context) {
		async_write_data* awd=(async_write_data*)context;
		awd->rc=write(awd->fd,awd->buf,awd->bufSize);
		if (awd->rc<0) awd->awd_errno=errno;
		awd->ready=true;
		return 0;
	}
};


class HostProxy_GadgetFS: public HostProxy {
private:
	bool p_is_connected;
	int p_device_file;
	struct aiocb* p_epin_async[16];
	struct aiocb* p_epout_async[16];
	bool p_epin_active[16];

	int debugLevel;

	char* descriptor;
	int descriptorLength;

	int reconnect();
	int generate_descriptor(Device* device);

	usb_ctrlrequest lastControl;

public:
	HostProxy_GadgetFS(int _debugLevel=0);
	virtual ~HostProxy_GadgetFS();

	int connect(Device* device);
	void disconnect();
	void reset();
	bool is_connected();

	//return 0 in usb_ctrlrequest->brequest if there is no request
	int control_request(usb_ctrlrequest *setup_packet, int *nbytes, __u8** dataptr);
	void send_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8* dataptr,int length);
	bool send_complete(__u8 endpoint);
	void receive_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8** dataptr, int* length);
	void control_ack();
	void stall_ep(__u8 endpoint);
	void setConfig(Configuration* fs_cfg,Configuration* hs_cfg,bool hs);
};

#endif /* USBPROXY_HOSTPROXY_GADGETFS_H */
