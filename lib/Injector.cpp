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
 * Injector.cpp
 *
 * Created on: Nov 12, 2013
 */

#include <poll.h>
#include <stdio.h>
#include "get_tid.h"
#include "HaltSignal.h"

#include "Injector.h"

#define SLEEP_US 1000

Injector::Injector() {
	haltSignal=0;
}

void Injector::set_queue(__u8 epAddress,mqd_t queue) {
	if (epAddress&0x80) {
		inQueues[epAddress&0x0f]=queue;
	} else {
		outQueues[epAddress&0x0f]=queue;
	}
}

void Injector::set_haltsignal(__u8 _haltSignal) {
	haltSignal=_haltSignal;
}

void Injector::listen() {
	bool halt=false;
	struct pollfd haltpoll;
	int haltfd;
	if (haltsignal_setup(haltSignal,&haltpoll,&haltfd)!=0) return;
	fprintf(stderr,"Starting injector thread (%ld) for [%s].\n",gettid(),this->toString());
	start_injector();
	while (!halt) {
		//TODO we also need to handle setup packets and getting the response back
		Packet* p=get_packets();
		if (p) {
			__u8 epAddress=p->bEndpoint;
			mqd_t queue =(epAddress&0x80)?inQueues[epAddress&0x0f]:outQueues[epAddress&0x0f];
			//TODO we need to poll so we don't send blocking packets.
			if (queue) mq_send(queue,(char*)&p,sizeof(Packet*),0);
		}
		halt=haltsignal_check(haltSignal,&haltpoll,&haltfd);
	}
	stop_injector();
	fprintf(stderr,"Finished injector thread (%ld) for [%s].\n",gettid(),this->toString());
}

void* Injector::listen_helper(void* context) {
	((Injector*)context)->listen();
	return 0;
}
