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
	deviceProxy=_deviceProxy;
	hostProxy=_hostProxy;
	device=NULL;
	filters=NULL;
	filterCount=0;
	injectors=NULL;
	injectorCount=0;
	injectorThreads=NULL;
	int i;
	for(i=0;i<16;i++) {
		in_endpoints[i]=NULL;
		in_relayers[i]=NULL;
		in_relayerThreads[i]=NULL;
		in_queue[i]=NULL;
		out_endpoints[i]=NULL;
		out_relayers[i]=NULL;
		out_relayerThreads[i]=NULL;
		out_queue[i]=NULL;
	}
}

USBManager::~USBManager() {
	//TODO: 1
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

int USBManager::inject_packet(USBPacket *packet) {
	//TODO 1 stub
}

int USBManager::inject_setup_in(usb_ctrlrequest request,__u8** data,__u16 *transferred, bool filter) {
	//TODO 1 stub
}

int USBManager::inject_setup_out(usb_ctrlrequest request,__u8* data,bool filter) {
	//TODO 1 stub
}


void USBManager::add_injector(USBInjector* _injector){
	//TODO: 1
}

void USBManager::remove_injector(__u8 index){
	//TODO: 1
}

USBInjector* USBManager::get_injector(__u8 index){
	//TODO: 1
}

__u8 USBManager::get_injector_count(){
	//TODO: 1
}

void USBManager::add_filter(USBPacketFilter* _filter){
	//TODO: 1
}

void USBManager::remove_filter(__u8 index){
	//TODO: 1
}

USBPacketFilter* USBManager::get_filter(__u8 index){
	//TODO: 1
}

__u8 USBManager::get_filter_count(){
	//TODO: 1
}


void USBManager::start_relaying(){
	//connect device proxy, populate device model, enumerate endpoints
	//set up relayers for each endpoint, apply filters to each relayer
	//don't forget to include EP0
	//connect to host proxy
	//start relayer threads, start injector threads
	//TODO: 1
}

void USBManager::stop_relaying(){
	//stop & join injector/relayer threads
	//disconnect from host
	//clean up relayers
	//disconnect device proxy, clean up device model & endpoints
	//don't forget to include EP0
	//TODO: 1
}
