/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
 *
 * This file is part of USBProxy.
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
 * HaltSignal.c
 *
 * Created on: Dec 10, 2013
 */

#include <stddef.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>
#include <sys/signalfd.h>
#include "HaltSignal.h"

int haltsignal_setup(int haltsignal,struct pollfd* haltpoll,int* haltfd) {
	if (!haltsignal) {
		fprintf(stderr,"No halt signal set.\n");
		return 1;
	}
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask,haltsignal);
	//block signals so they will go to the FD
	if (sigprocmask(SIG_BLOCK,&mask,NULL)<0) {
		fprintf(stderr,"Error applying sigprocmask.\n");
		return 1;
	}
	*haltfd=signalfd(-1,&mask,SFD_CLOEXEC);
	if (*haltfd<0) {
		fprintf(stderr,"Error creating signalfd\n");
		*haltfd=0;
		return 1;
	}
	haltpoll->fd=*haltfd;
	haltpoll->events=POLLIN;
	return 0;
}

int haltsignal_check(int haltsignal,struct pollfd* haltpoll,int* haltfd) {
	if (poll(haltpoll,1,0) && (haltpoll->revents&POLLIN)) {
		struct signalfd_siginfo si;
		int rc=read(*haltfd,&si,sizeof(si));
		if (rc<0) {
			fprintf(stderr,"Error reading from signalfd\n");
		} else {
			if (si.ssi_signo==haltsignal) {
				close(*haltfd);
				return 1;
			}
		}
	}
	return 0;
}



