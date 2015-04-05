/*
 * This file is part of USBProxy.
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "TRACE.h"
#include "HexString.h"
#include "DeviceProxy_TCP.h"

#include "Configuration.h"
#include "Interface.h"
#include "Endpoint.h"

DeviceProxy_TCP::DeviceProxy_TCP(const char* address) {
	p_is_connected = false;
}

/* FIXME pull settings from config parser */
DeviceProxy_TCP::DeviceProxy_TCP(ConfigParser *cfg)
	: DeviceProxy(*cfg)
{
	network = new TCP_Helper(NULL);
	if (network)
		network->debugLevel = debugLevel;
	p_is_connected = false;
}

DeviceProxy_TCP::~DeviceProxy_TCP() {
	if (network) {
		delete(network);
		network=NULL;
	}
}

/* Open a socket for EP0 - we don't know how many EPs we need yet */
int DeviceProxy_TCP::connect(int timeout) {
	int rc=network->connect(timeout);
	p_is_connected=rc==0;
	return rc;
}

void DeviceProxy_TCP::disconnect() {
	p_is_connected = false;
}

void DeviceProxy_TCP::reset() {
	//FINISH
}

bool DeviceProxy_TCP::is_connected() {
	return p_is_connected;
}

bool DeviceProxy_TCP::is_highspeed() {
	return false;
}

//return -1 to stall
int DeviceProxy_TCP::control_request(const usb_ctrlrequest *setup_packet, int *nbytes, __u8 *dataptr, int timeout) {
	//FINISH
	if (debugLevel>1) {
		char* hex=hex_string((void*)setup_packet,sizeof(*setup_packet));
		fprintf(stderr, "TCP> %s\n",hex);
		free(hex);
	}
	int length=8;
	length+=(setup_packet->bRequestType&0x80)?0:setup_packet->wLength;
	__u8* buf=(__u8*)malloc(length);
	memcpy(buf,setup_packet,8);
	if (!(setup_packet->bRequestType&0x80)) memcpy(buf+8,dataptr,setup_packet->wLength);
	network->send_data(0,buf,length);
	free(buf);
	buf=NULL;
	length=0;
	network->receive_data(0,&buf,&length,timeout);
	if (length==0 || buf[0]) {return -1;}
	__u16 usblen=buf[1]<<8 | buf[2];
	*nbytes=(usblen>setup_packet->wLength)?setup_packet->wLength:usblen;
	if (debugLevel>1 && *nbytes) {
		char* hex=hex_string((void*)(buf+3),*nbytes);
		fprintf(stderr, "TCP> %s\n",hex);
		free(hex);
	}
	memcpy(dataptr,buf+3,*nbytes);
	free(buf);
	return 0;
}

void DeviceProxy_TCP::send_data(__u8 endpoint,__u8 attributes, __u16 maxPacketSize, __u8* dataptr, int length) {
	network->send_data(endpoint,dataptr,length);
}

void DeviceProxy_TCP::receive_data(__u8 endpoint,__u8 attributes, __u16 maxPacketSize, __u8** dataptr, int* length, int timeout) {
	network->receive_data(endpoint,dataptr,length,timeout);
}

void DeviceProxy_TCP::setConfig(Configuration* fs_cfg, Configuration* hs_cfg, bool hs) {
	fprintf(stderr,"TCPDP SetConfig\n");
	int ifc_idx;
	__u8 ep_total=0;
	__u8 ifc_count=fs_cfg->get_descriptor()->bNumInterfaces;
	for (ifc_idx=0;ifc_idx<ifc_count;ifc_idx++) {
		Interface* ifc=fs_cfg->get_interface(ifc_idx);
		ep_total+=ifc->get_endpoint_count();
	}

	__u8* eps=(__u8*)malloc(ep_total);
	__u8 ep_total_idx=0;
	for (ifc_idx=0;ifc_idx<ifc_count;ifc_idx++) {
		Interface* ifc=fs_cfg->get_interface(ifc_idx);
		__u8 ep_count=ifc->get_endpoint_count();
		int ep_idx;
		for (ep_idx=0;ep_idx<ep_count;ep_idx++) {
			const usb_endpoint_descriptor* ep=ifc->get_endpoint_by_idx(ep_idx)->get_descriptor();
			eps[ep_total_idx++]=ep->bEndpointAddress;
		}
	}
	int rc=network->open_endpoints(eps,ep_total,250);
	TRACE1(rc)
	while (rc>0) {rc=network->open_endpoints(eps,ep_total,250);putchar('.');fflush(stdout);TRACE1(rc)}
	free(eps);
}

void DeviceProxy_TCP::claim_interface(__u8 interface) {}

void DeviceProxy_TCP::release_interface(__u8 interface) {}

__u8 DeviceProxy_TCP::get_address() {
	return 1;
}

static DeviceProxy_TCP *proxy;

extern "C" {
	DeviceProxy * get_deviceproxy_plugin(ConfigParser *cfg) {
		proxy = new DeviceProxy_TCP(cfg);
		return (DeviceProxy *) proxy;
	}
	
	void destroy_plugin() {
		delete proxy;
	}
}
