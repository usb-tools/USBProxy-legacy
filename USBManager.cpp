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
 * USBManager.cpp
 *
 * Created on: Nov 12, 2013
 */
#include "USBManager.h"
#include "pthread.h"

USBManager::USBManager(USBDeviceProxy* _deviceProxy,USBHostProxy* _hostProxy) {
	status=USBM_IDLE;
	deviceProxy=_deviceProxy;
	hostProxy=_hostProxy;
	device=NULL;
	filters=NULL;
	filterCount=0;
	injectors=NULL;
	injectorCount=0;
	injectorThreads=NULL;
	in_queue_ep0=NULL;
	//out_queue_ep0=NULL;
	int i;
	for(i=0;i<16;i++) {
		in_endpoints[i]=NULL;
		in_relayers[i]=NULL;
		in_relayerThreads[i]=0;
		in_queue[i]=NULL;
		out_endpoints[i]=NULL;
		out_relayers[i]=NULL;
		out_relayerThreads[i]=0;
		out_queue[i]=NULL;
	}
}

USBManager::~USBManager() {
	if (device) {delete(device);}
	if (filters) {free(filters);}
	if (injectors) {free(injectors);}
	int i;
	for (i=0;i<16;i++) {
		if (in_relayers[i]) {delete(in_relayers[i]);}
		if (out_relayers[i]) {delete(out_relayers[i]);}
		USBPacket* p;
		while(in_queue[i]->pop(p)) {delete(p);}
		if (in_queue[i]) {delete(in_queue[i]);}
		while(out_queue[i]->pop(p)) {delete(p);}
		if (out_queue[i]) {delete(out_queue[i]);}
		if (in_relayerThreads[i]) {pthread_cancel(in_relayerThreads[i]);}
		if (out_relayerThreads[i]) {pthread_cancel(out_relayerThreads[i]);}
	}
	for (i=0;i<injectorCount;i++) {
		if (injectorThreads[i]) {pthread_cancel(injectorThreads[i]);}
	}
}

void USBManager::inject_packet(USBPacket *packet) {
	if (status!=USBM_RELAYING) {fprintf(stderr,"Can't inject packets unless manager is relaying.\n");}
	__u8 epAddress=packet->bEndpoint;
	if (epAddress&0x80) { //device->host
		in_queue[epAddress&0x0f]->push(packet);
	} else { //host->device
		out_queue[epAddress&0x0f]->push(packet);
	}
}

void USBManager::inject_setup_in(usb_ctrlrequest request,__u8** data,__u16 *transferred, bool filter) {
	if (status!=USBM_RELAYING) {fprintf(stderr,"Can't inject packets unless manager is relaying.\n");}
	USBSetupPacket* p=new USBSetupPacket(request,NULL,filter);
	in_queue_ep0->push(p);
	//TODO handle returned data...somehow..can use 2nd queue for replies, but would need to poll it or something
}

void USBManager::inject_setup_out(usb_ctrlrequest request,__u8* data,bool filter) {
	if (status!=USBM_RELAYING) {fprintf(stderr,"Can't inject packets unless manager is relaying.\n");}
	USBSetupPacket* p=new USBSetupPacket(request,data,filter);
	in_queue_ep0->push(p);
}


void USBManager::add_injector(USBInjector* _injector){
	if (status!=USBM_IDLE) {fprintf(stderr,"Can't add injectors unless manager is idle.\n");}
	if (injectors) {
		injectors=(USBInjector**)realloc(injectors,++injectorCount*sizeof(USBInjector*));
	} else {
		injectorCount=1;
		injectors=(USBInjector**)malloc(sizeof(USBInjector*));
	}
	injectors[injectorCount-1]=_injector;
}

void USBManager::remove_injector(__u8 index,bool freeMemory){
	if (status!=USBM_IDLE) {fprintf(stderr,"Can't remove injectors unless manager is idle.\n");}
	if (!injectors || index>=injectorCount) {fprintf(stderr,"Injector index out of bounds.\n");}
	if (freeMemory && injectors[index]) {delete(injectors[index]);}
	if (injectorCount==1) {
		injectorCount=0;
		free(injectors);
	} else {
		int i;
		for(i=index+1;i<injectorCount;i++) {
			injectors[i-1]=injectors[i];
		}
		injectors=(USBInjector**)realloc(injectors,--injectorCount*sizeof(USBInjector*));
	}
}

USBInjector* USBManager::get_injector(__u8 index){
	if (!injectors || index>=injectorCount) {return NULL;}
	return injectors[index];
}

__u8 USBManager::get_injector_count(){
	return injectorCount;
}

void USBManager::add_filter(USBPacketFilter* _filter){
	if (status!=USBM_IDLE) {fprintf(stderr,"Can't add filters unless manager is idle.\n");}
	if (filters) {
		filters=(USBPacketFilter**)realloc(filters,++filterCount*sizeof(USBPacketFilter*));
	} else {
		filterCount=1;
		filters=(USBPacketFilter**)malloc(sizeof(USBPacketFilter*));
	}
	filters[filterCount-1]=_filter;
}

void USBManager::remove_filter(__u8 index,bool freeMemory){
	if (status!=USBM_IDLE) {fprintf(stderr,"Can't remove filters unless manager is idle.\n");}
	if (!filters || index>=filterCount) {fprintf(stderr,"Filter index out of bounds.\n");}
	if (freeMemory && filters[index]) {delete(filters[index]);}
	if (filterCount==1) {
		filterCount=0;
		free(filters);
	} else {
		int i;
		for(i=index+1;i<filterCount;i++) {
			filters[i-1]=filters[i];
		}
		filters=(USBPacketFilter**)realloc(filters,--filterCount*sizeof(USBPacketFilter*));
	}
}

USBPacketFilter* USBManager::get_filter(__u8 index){
	if (!filters || index>=filterCount) {return NULL;}
	return filters[index];
}

__u8 USBManager::get_filter_count(){
	return filterCount;
}


void USBManager::start_relaying(){
	//connect device proxy, populate device model, enumerate endpoints
	//set up relayers for each endpoint, apply filters to each relayer
	//don't forget to include EP0
	//connect to host proxy
	//start relayer threads, start injector threads
	//FINISH
}

void USBManager::stop_relaying(){
	//stop & join injector/relayer threads
	//disconnect from host
	//clean up relayers
	//disconnect device proxy, clean up device model & endpoints
	//don't forget to include EP0
	//FINISH
}
