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
	bool idle;
	bool halt=false;
	bool setup_wait=false;
	struct pollfd haltpoll;
	int haltfd;
	if (haltsignal_setup(haltSignal,&haltpoll,&haltfd)!=0) return;
	fprintf(stderr,"Starting injector thread (%ld) for [%s].\n",gettid(),this->toString());
	start_injector();
	struct Packet* packet;
	struct SetupPacket* setup;
	struct pollfd poll_setup,poll_send;
	poll_setup.fd=inQueues[0];
	poll_setup.events=POLLIN;

	poll_send.events=POLLOUT;

	fprintf(stderr,"Injector polling %d\n",inQueues[0]);
	while (!halt) {
		idle=true;
		if (!setup && !packet) get_packets(&packet,&setup,500);
		if (setup) {
			if (setup_wait) {
				if (poll(&poll_setup,1,500) & poll_setup.revents&POLLIN) {
					fprintf(stderr,"Injector recv setup on %d\n",poll_setup.fd);
					mq_receive(poll_setup.fd,(char*)&setup,sizeof(SetupPacket*),NULL);
					poll_setup.revents=0;
					if (setup->transmit_in) {
						if (setup->ctrl_req.wLength) {
							setup_data(setup->data,setup->ctrl_req.wLength);
						} else {
							setup_ack();
						}
					} else {
						setup_stall();
					}
					idle=false;
				}
			} else {
				mqd_t queue=outQueues[0];
				if (queue) {
					fprintf(stderr,"Injector send setup on %d\n",queue);
					poll_send.fd=queue;
					if (poll(&poll_send,1,500) && (poll_send.revents&POLLOUT)) {
						poll_send.revents=0;
						mq_send(queue,(char*)&setup,sizeof(SetupPacket*),0);
						setup_wait=true;
						idle=false;
					}
				} else {
					setup_wait=true;
					idle=false;
				}
			}
		} else if (packet) {
			__u8 epAddress=packet->bEndpoint;
			mqd_t queue =(epAddress&0x80)?inQueues[epAddress&0x0f]:outQueues[epAddress&0x0f];
			//TODO we need to poll so we don't send blocking packets.
			if (queue) mq_send(queue,(char*)&packet,sizeof(Packet*),0);
			idle=false;
		}
		halt=haltsignal_check(haltSignal,&haltpoll,&haltfd);
		if (idle) sched_yield();
	}
	stop_injector();
	fprintf(stderr,"Finished injector thread (%ld) for [%s].\n",gettid(),this->toString());

	/*
Injector (this will need to use a common event loop)

    while this flag is set it will need to poll the ep0 mq as well as it's normal data source, if we want to optimize this at some point we could potentially have it poll them both simultaneously, but i'm comfortable having it poll them both (at the cost of increased latency or CPU usage) or simply stop reading in new packets until it gets a setup response. i'm leaning heavily to "no new packets" because otherwise if we read in another packet for ep0 we need to buffer that up somehow. I figure you shouldn't have setup traffic going on and expect other traffic to be injected as quickly as normal. your thoughts?
    when it gets a response, it "does something" with it, i think we could just add in an ack/stall/data received calls to the base class and people can then implement if they need it, and the event loop will call the appropriate function
    it the clears the flag and goes back to business as usual.
	 */
}

void* Injector::listen_helper(void* context) {
	((Injector*)context)->listen();
	return 0;
}
