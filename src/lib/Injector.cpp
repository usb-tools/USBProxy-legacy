/*
 * This file is part of USBProxy.
 */

#include <poll.h>
#include <stdio.h>
#include <sched.h>
#include "get_tid.h"
#include "TRACE.h"

#include "Injector.h"

#define SLEEP_US 1000

Injector::Injector()
	: _please_stop(false)
{
	int i;
	for (i=0;i<16;i++) {
		inQueues[i]=0;
		outQueues[i]=0;
		inPollIndex[i]=0;
		outPollIndex[i]=0;
	}
}

void Injector::set_queue(__u8 epAddress,mqd_t queue) {
	if (epAddress&0x80) {
		inQueues[epAddress&0x0f]=queue;
	} else {
		outQueues[epAddress&0x0f]=queue;
	}
}

void Injector::listen() {
	bool idle;
	bool setup_wait=false;
	fprintf(stderr,"Starting injector thread (%ld) for [%s].\n",gettid(),this->toString());
	start_injector();
	struct Packet* packet;
	struct SetupPacket* setup;
	struct pollfd poll_setup,poll_send;
	poll_setup.fd=inQueues[0];
	poll_setup.events=POLLIN;
	poll_send.events=POLLOUT;

	int* fdlist=get_pollable_fds();
	int pollreadcount=0;
	int i=0;
	while(fdlist[i++]) {pollreadcount++;}
	struct pollfd* poll_list=(struct pollfd*)calloc(pollreadcount,sizeof(pollfd));
	Packet** poll_buffer=(Packet**)calloc(pollreadcount,sizeof(Packet*));
	for (i=0;i<pollreadcount;i++) {
		fprintf(stderr,"Injector In FD[%d/%d]: %d\n",i+1,pollreadcount,fdlist[i]);
		poll_list[i].fd=fdlist[i];
		poll_buffer[i]=NULL;
		poll_list[i++].events=POLLIN;
	}
	free(fdlist);
	int polllistsize=pollreadcount;
	while (!_please_stop) {
		idle=true;
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
		} else if (poll(poll_list,pollreadcount,500)) {
			for(i=pollreadcount;i<polllistsize;i++) {
				if (poll_list[i].revents & POLLOUT) {
					poll_list[i].revents=0;
					if (poll_buffer[i]) {
						mq_send(poll_list[i].fd,(char*)poll_buffer[i],sizeof(Packet*),0);
						poll_buffer[i]=NULL;
					} else {
						poll_list[i].events=0;
					}
				}
			}
			for(i=0;i<pollreadcount;i++) {
				if (poll_list[i].revents & POLLIN) {
					get_packets(&packet,&setup,500);
					if (setup) {
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
							full_pipe(setup);
							setup_wait=true;
							idle=false;
						}
					} else if (packet) {
						//if we are unable to write a packet immediately (0 poll on the out pipe), we add the out pipe to our polling list (POLLOUT)
						//if we get a hit on an out pipe we send that pending packet then remove the out pipe from the polling list
						//if we get a new packet while there is one pending on the same out pipe we send back to the injector via full_pipe(Packet *p)
						__u8 epAddress=packet->bEndpoint;
						mqd_t queue =(epAddress&0x80)?inQueues[epAddress&0x0f]:outQueues[epAddress&0x0f];
						if (queue) { //if queue defined for this EP, attempt to send
							poll_send.fd=queue;
							if (poll(&poll_send,1,0) && (poll_send.revents&POLLOUT)) { //send on the queue if available
								mq_send(queue,(char*)&packet,sizeof(Packet*),0);
							} else {
								fprintf(stderr,"Buffering packet for EP%02x\n",epAddress);
								int pollIndex=(epAddress&0x80)?inPollIndex[epAddress&0x0f]:outPollIndex[epAddress&0x0f];
								if (!pollIndex) { //set up a new poll entry for this queue and store the packet in the buffer
									pollIndex=polllistsize++;
									poll_list=(struct pollfd*)realloc(poll_list,polllistsize*sizeof(pollfd));
									poll_buffer=(Packet**)realloc(poll_buffer,polllistsize*sizeof(Packet*));
									if (epAddress&0x80) {inPollIndex[epAddress&0x0f]=pollIndex;} else {outPollIndex[epAddress&0x0f]=pollIndex;}
									poll_list[pollIndex].fd=queue;
									poll_buffer[pollIndex]=packet;
									poll_list[pollIndex].events=POLLOUT;
								} else { // check whether the buffer is in use
									if (poll_buffer[pollIndex]) { //kick packet back to injector
										full_pipe(packet);
									} else { //store packet in buffer.
										poll_buffer[pollIndex]=packet;
										poll_list[pollIndex].events=POLLOUT;
									}
								}
							}
						} else { //kick packet back to injector
							full_pipe(packet);
						}
						packet=NULL;
						idle=false;
					}
					continue;
				}
			}
		}
		if (idle) sched_yield();
	}
	free(poll_list);
	for (i=0;i<polllistsize;i++) {
		if (poll_buffer[i]) {delete(poll_buffer[i]);poll_buffer[i]=NULL;}
	}
	free(poll_buffer);
	stop_injector();
	fprintf(stderr,"Finished injector thread (%ld) for [%s].\n",gettid(),this->toString());
	_please_stop = false;

	/*
Injector (this will need to use a common event loop)

    while this flag is set it will need to poll the ep0 mq as well as it's normal data source, if we want to optimize this at some point we could potentially have it poll them both simultaneously, but i'm comfortable having it poll them both (at the cost of increased latency or CPU usage) or simply stop reading in new packets until it gets a setup response. i'm leaning heavily to "no new packets" because otherwise if we read in another packet for ep0 we need to buffer that up somehow. I figure you shouldn't have setup traffic going on and expect other traffic to be injected as quickly as normal. your thoughts?
    when it gets a response, it "does something" with it, i think we could just add in an ack/stall/data received calls to the base class and people can then implement if they need it, and the event loop will call the appropriate function
    it the clears the flag and goes back to business as usual.
	 */
}
