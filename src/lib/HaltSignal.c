/*
 * This file is part of USBProxy.
 */

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
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



