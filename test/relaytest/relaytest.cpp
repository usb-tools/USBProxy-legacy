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
 * relay.c
 *
 * Created on: Dec 5, 2013
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/times.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <poll.h>
#include <boost/atomic.hpp>
#include <mqueue.h>
#include <sys/stat.h>
#include <errno.h>

int writecount=0;
int fd_in,fd_out;
long int sleepsize;

int slowread_fd[2],slowwrite_fd[2];
pthread_t slowread_thread,slowwrite_thread;
volatile bool slowread_halt,slowwrite_halt;

#define NUMWRITES 1000

void test_block() {
	int val;
	while (writecount) {
		read(fd_in,&val,4);
		write(fd_out,&val,4);
		writecount--;
	}
}

void test_ro() {
	int val;
	while (writecount) {
		read(fd_in,&val,4);
		writecount--;
	}
}

void test_wo() {
	int val=0;
	while (writecount) {
		write(fd_out,&val,4);
		writecount--;
	}
}

void test_empty() {
	while (writecount) {
		writecount--;
	}
}

volatile bool test_thread_halt;
pthread_t test_thread_tread,test_thread_twrite;
volatile int test_thread_val;

void* test_thread_read(void* p) {
	int val;
	while(!test_thread_halt) {
		read(fd_in,(void*)&test_thread_val,4);
	}
	return NULL;
}

void* test_thread_write(void* p) {
	while(writecount) {
		write(fd_out,(void*)&test_thread_val,4);
		writecount--;
	}
	test_thread_halt=true;
	return NULL;
}

void test_thread() {
	test_thread_halt=false;
	pthread_create(&test_thread_twrite,NULL,&test_thread_write,NULL);
	pthread_create(&test_thread_tread,NULL,&test_thread_read,NULL);
	pthread_join(test_thread_twrite,NULL);
	pthread_join(test_thread_tread,NULL);

}

volatile bool test_flag_halt,test_flag_ready;
volatile int test_flag_val;
pthread_t test_flag_tread,test_flag_twrite;


void* test_flag_read(void* p) {
	while(!test_flag_halt) {
		if (!test_flag_ready) {
			read(fd_in,(void *)&test_flag_val,4);
			test_flag_ready=true;
		} else {
			if (sleepsize<0) sched_yield(); else usleep(sleepsize);
		}
	}
	return NULL;
}

void* test_flag_write(void* p) {
	while(writecount) {
		if (test_flag_ready) {
			write(fd_out,(void *)&test_flag_val,4);
			writecount--;
			test_flag_ready=false;
		} else {
			if (sleepsize<0) sched_yield(); else usleep(sleepsize);
		}
	}
	test_flag_halt=true;
	return NULL;
}

void test_flag() {
	test_flag_halt=false;
	test_flag_ready=false;
	pthread_create(&test_flag_twrite,NULL,&test_flag_write,NULL);
	pthread_create(&test_flag_tread,NULL,&test_flag_read,NULL);
	pthread_join(test_flag_twrite,NULL);
	pthread_join(test_flag_tread,NULL);

}

volatile bool test_atomic_halt;
volatile int test_atomic_val;
boost::atomic<bool> test_atomic_ready;
pthread_t test_atomic_tread,test_atomic_twrite;

void* test_atomic_read(void* p) {
	int val;
	while(!test_atomic_halt) {
		if (!test_atomic_ready) {
			read(fd_in,(void *)&test_atomic_val,4);
			test_atomic_ready=true;
		} else {
			if (sleepsize<0) sched_yield(); else usleep(sleepsize);
		}
	}
	return NULL;
}

void* test_atomic_write(void* p) {
	while(writecount) {
		if (test_atomic_ready) {
			write(fd_out,(void *)&test_atomic_val,4);
			writecount--;
			test_atomic_ready=false;
		} else {
			if (sleepsize<0) sched_yield(); else usleep(sleepsize);
		}
	}
	test_atomic_halt=true;
	return NULL;
}

void test_atomic() {
	test_atomic_halt=false;
	test_atomic_ready=false;
	pthread_create(&test_atomic_twrite,NULL,&test_atomic_write,NULL);
	pthread_create(&test_atomic_tread,NULL,&test_atomic_read,NULL);
	pthread_join(test_atomic_twrite,NULL);
	pthread_join(test_atomic_tread,NULL);

}

volatile bool test_mutex_halt;
struct timespec test_mutex_tval;
pthread_t test_mutex_tread,test_mutex_twrite;
pthread_mutex_t test_mutex_rmutex,test_mutex_wmutex;
volatile int test_mutex_val;

void* test_mutex_read(void* p) {
	while(!test_mutex_halt) {
		pthread_mutex_lock(&test_mutex_rmutex);
		read(fd_in,(void*)&test_mutex_val,4);
		pthread_mutex_unlock(&test_mutex_wmutex);
	}
	return NULL;
}

void* test_mutex_write(void* p) {
	while(writecount) {
		pthread_mutex_lock(&test_mutex_wmutex);
		write(fd_out,(void*)&test_mutex_val,4);
		writecount--;
		pthread_mutex_unlock(&test_mutex_rmutex);
	}
	test_mutex_halt=true;
	return NULL;
}

void test_mutex() {
	test_mutex_halt=false;
	test_mutex_tval.tv_nsec=10000000;
	pthread_mutex_init(&test_mutex_rmutex,NULL);
	pthread_mutex_init(&test_mutex_wmutex,NULL);
	pthread_mutex_lock(&test_mutex_wmutex);
	pthread_create(&test_mutex_tread,NULL,&test_mutex_read,NULL);
	pthread_create(&test_mutex_twrite,NULL,&test_mutex_write,NULL);
	pthread_join(test_mutex_twrite,NULL);
	pthread_join(test_mutex_tread,NULL);
}

volatile bool test_semaphore_halt;
struct timespec test_semaphore_tval;
pthread_t test_semaphore_tread,test_semaphore_twrite;
sem_t test_semaphore_rsemaphore,test_semaphore_wsemaphore;
volatile int test_semaphore_val;

void* test_semaphore_read(void* p) {
	while(!test_semaphore_halt) {
		sem_wait(&test_semaphore_rsemaphore);
		read(fd_in,(void*)&test_semaphore_val,4);
		sem_post(&test_semaphore_wsemaphore);
	}
	return NULL;
}

void* test_semaphore_write(void* p) {
	while(writecount) {
		sem_wait(&test_semaphore_wsemaphore);
		write(fd_out,(void*)&test_semaphore_val,4);
		writecount--;
		sem_post(&test_semaphore_rsemaphore);
	}
	test_semaphore_halt=true;
	sem_post(&test_semaphore_rsemaphore);
	return NULL;
}

void test_semaphore() {
	test_semaphore_halt=false;
	test_semaphore_tval.tv_nsec=10000000;
	sem_init(&test_semaphore_rsemaphore,0,1);
	sem_init(&test_semaphore_wsemaphore,0,0);
	pthread_create(&test_semaphore_tread,NULL,&test_semaphore_read,NULL);
	pthread_create(&test_semaphore_twrite,NULL,&test_semaphore_write,NULL);
	pthread_join(test_semaphore_twrite,NULL);
	pthread_join(test_semaphore_tread,NULL);
}

volatile bool test_mutexto_halt;
pthread_t test_mutexto_tread,test_mutexto_twrite;
pthread_mutex_t test_mutexto_rmutex,test_mutexto_wmutex;
volatile int test_mutexto_val;

void* test_mutexto_read(void* p) {
	struct timespec ts;
	while(!test_mutexto_halt) {
		clock_gettime(CLOCK_REALTIME,&ts);
		if (sleepsize>=1000) {
			ts.tv_sec+=sleepsize/1000;
		} else {
			ts.tv_nsec+=(1000000*sleepsize);
		}
		if (!pthread_mutex_timedlock(&test_mutexto_rmutex,&ts)) {
			read(fd_in,(void*)&test_mutexto_val,4);
			pthread_mutex_unlock(&test_mutexto_wmutex);
		}
	}
	return NULL;
}

void* test_mutexto_write(void* p) {
	struct timespec ts;
	while(writecount) {
		clock_gettime(CLOCK_REALTIME,&ts);
		if (sleepsize>=1000) {
			ts.tv_sec+=sleepsize/1000;
		} else {
			ts.tv_nsec+=(1000000*sleepsize);
		}
		if (!pthread_mutex_timedlock(&test_mutexto_wmutex,&ts)) {
			write(fd_out,(void*)&test_mutexto_val,4);
			writecount--;
			pthread_mutex_unlock(&test_mutexto_rmutex);
		}
	}
	test_mutexto_halt=true;
	return NULL;
}

void test_mutexto() {
	test_mutexto_halt=false;
	pthread_mutex_init(&test_mutexto_rmutex,NULL);
	pthread_mutex_init(&test_mutexto_wmutex,NULL);
	pthread_mutex_lock(&test_mutexto_wmutex);
	pthread_create(&test_mutexto_tread,NULL,&test_mutexto_read,NULL);
	pthread_create(&test_mutexto_twrite,NULL,&test_mutexto_write,NULL);
	pthread_join(test_mutexto_twrite,NULL);
	pthread_join(test_mutexto_tread,NULL);
}

volatile bool test_mutextry_halt;
struct timespec test_mutextry_tval;
pthread_t test_mutextry_tread,test_mutextry_twrite;
pthread_mutex_t test_mutextry_rmutex,test_mutextry_wmutex;
volatile int test_mutextry_val;

void* test_mutextry_read(void* p) {
	while(!test_mutextry_halt) {
		if (!pthread_mutex_trylock(&test_mutextry_rmutex)) {
			read(fd_in,(void*)&test_mutextry_val,4);
			pthread_mutex_unlock(&test_mutextry_wmutex);
		} else {
			if (sleepsize<0) sched_yield(); else usleep(sleepsize);
		}

	}
	return NULL;
}

void* test_mutextry_write(void* p) {
	while(writecount) {
		if (!pthread_mutex_trylock(&test_mutextry_wmutex)) {
			write(fd_out,(void*)&test_mutextry_val,4);
			writecount--;
			pthread_mutex_unlock(&test_mutextry_rmutex);
		} else {
			if (sleepsize<0) sched_yield(); else usleep(sleepsize);
		}
	}
	test_mutextry_halt=true;
	return NULL;
}

void test_mutextry() {
	test_mutextry_halt=false;
	test_mutextry_tval.tv_nsec=10000000;
	pthread_mutex_init(&test_mutextry_rmutex,NULL);
	pthread_mutex_init(&test_mutextry_wmutex,NULL);
	pthread_mutex_lock(&test_mutextry_wmutex);
	pthread_create(&test_mutextry_tread,NULL,&test_mutextry_read,NULL);
	pthread_create(&test_mutextry_twrite,NULL,&test_mutextry_write,NULL);
	pthread_join(test_mutextry_twrite,NULL);
	pthread_join(test_mutextry_tread,NULL);
}

volatile bool test_semaphoreto_halt;
struct timespec test_semaphoreto_tval;
pthread_t test_semaphoreto_tread,test_semaphoreto_twrite;
sem_t test_semaphoreto_rsemaphore,test_semaphoreto_wsemaphore;
volatile int test_semaphoreto_val;

void* test_semaphoreto_read(void* p) {
	struct timespec ts;
	while(!test_semaphoreto_halt) {
		clock_gettime(CLOCK_REALTIME,&ts);
		ts.tv_sec+=1;
		if (!sem_timedwait(&test_semaphoreto_rsemaphore,&ts)) {
			read(fd_in,(void*)&test_semaphoreto_val,4);
			sem_post(&test_semaphoreto_wsemaphore);
		} else {
			if (sleepsize<0) sched_yield(); else usleep(sleepsize);
		}

	}
	return NULL;
}

void* test_semaphoreto_write(void* p) {
	struct timespec ts;
	while(writecount) {
		clock_gettime(CLOCK_REALTIME,&ts);
		ts.tv_sec+=1;
		if (!sem_timedwait(&test_semaphoreto_wsemaphore,&ts)) {
			write(fd_out,(void*)&test_semaphoreto_val,4);
			writecount--;
			sem_post(&test_semaphoreto_rsemaphore);
		}
	}
	test_semaphoreto_halt=true;
	return NULL;
}

void test_semaphoreto() {
	test_semaphoreto_halt=false;
	test_semaphoreto_tval.tv_nsec=10000000;
	sem_init(&test_semaphoreto_rsemaphore,0,1);
	sem_init(&test_semaphoreto_wsemaphore,0,0);
	pthread_create(&test_semaphoreto_tread,NULL,&test_semaphoreto_read,NULL);
	pthread_create(&test_semaphoreto_twrite,NULL,&test_semaphoreto_write,NULL);
	pthread_join(test_semaphoreto_twrite,NULL);
	pthread_join(test_semaphoreto_tread,NULL);
}

volatile bool test_semaphoreone_halt;
struct timespec test_semaphoreone_tval;
pthread_t test_semaphoreone_tread,test_semaphoreone_twrite;
sem_t test_semaphoreone_semaphore;
volatile int test_semaphoreone_val;

void* test_semaphoreone_read(void* p) {
	while(!test_semaphoreone_halt) {
		read(fd_in,(void*)&test_semaphoreone_val,4);
		sem_post(&test_semaphoreone_semaphore);
	}
	return NULL;
}

void* test_semaphoreone_write(void* p) {
	while(writecount) {
		if (!sem_timedwait(&test_semaphoreone_semaphore,&test_semaphoreone_tval)) {
			write(fd_out,(void*)&test_semaphoreone_val,4);
			writecount--;
		}
	}
	test_semaphoreone_halt=true;
	return NULL;
}

void test_semaphoreone() {
	test_semaphoreone_halt=false;
	test_semaphoreone_tval.tv_nsec=10000000;
	sem_init(&test_semaphoreone_semaphore,0,0);
	pthread_create(&test_semaphoreone_tread,NULL,&test_semaphoreone_read,NULL);
	pthread_create(&test_semaphoreone_twrite,NULL,&test_semaphoreone_write,NULL);
	pthread_join(test_semaphoreone_twrite,NULL);
	pthread_join(test_semaphoreone_tread,NULL);
}

volatile bool test_pipe_halt;
int test_pipe_pipe[2];
pthread_t test_pipe_tread,test_pipe_twrite;

void* test_pipe_read(void* p) {
	int val;
	while(!test_pipe_halt) {
		read(fd_in,&val,4);
		write(test_pipe_pipe[1],&val,4);
	}
	return NULL;
}

void* test_pipe_write(void* p) {
	int val;
	while(writecount) {
		read(test_pipe_pipe[0],&val,4);
		write(fd_out,&val,4);
		writecount--;
	}
	test_pipe_halt=true;
	return NULL;
}

void test_pipe() {
	test_pipe_halt=false;
	pipe(test_pipe_pipe);
	pthread_create(&test_pipe_tread,NULL,&test_pipe_read,NULL);
	pthread_create(&test_pipe_twrite,NULL,&test_pipe_write,NULL);
	pthread_join(test_pipe_twrite,NULL);
	pthread_join(test_pipe_tread,NULL);
	close(test_pipe_pipe[0]);
	close(test_pipe_pipe[1]);
}

volatile bool test_queue_halt;
int test_queue_count,test_queue_size;
pthread_t test_queue_tread,test_queue_twrite;
mqd_t test_queue_rqueue,test_queue_wqueue;
struct pollfd test_queue_rpoll,test_queue_wpoll;

void* test_queue_read(void* p) {
	int val;
	while(!test_queue_halt) {
		if (poll(&test_queue_wpoll, 1, 100) && (test_queue_wpoll.revents&POLLOUT)) {
			read(fd_in,&val,4);
			mq_send(test_queue_wqueue,(char*)&val,4,0);
		}
	}
	return NULL;
}

void* test_queue_write(void* p) {
	int val;
	while(writecount) {
		if (poll(&test_queue_rpoll, 1, 100) && (test_queue_rpoll.revents&POLLIN)) {
			mq_receive(test_queue_rqueue,(char*)&val,4,NULL);
			write(fd_out,&val,4);
			writecount--;
		}
	}
	test_queue_halt=true;
	return NULL;
}

void test_queue() {
	test_queue_halt=false;
	struct mq_attr mqa;
	mqa.mq_maxmsg=test_queue_size;
	mqa.mq_msgsize=4;

	if (test_queue_count==1) {
		test_queue_rqueue=mq_open("/relaytest",O_RDWR | O_CREAT,S_IRWXU,&mqa);
		test_queue_wqueue=test_queue_rqueue;
	} else {
		test_queue_rqueue=mq_open("/relaytest",O_RDONLY | O_CREAT,S_IRWXU,&mqa);
		test_queue_wqueue=mq_open("/relaytest",O_WRONLY);
	}

	if (test_queue_rqueue<0) printf("%d %s\n",errno,strerror(errno));
	test_queue_rpoll.fd=test_queue_rqueue;
	test_queue_rpoll.events=POLLIN;
	test_queue_wpoll.fd=test_queue_wqueue;
	test_queue_wpoll.events=POLLOUT;
	pthread_create(&test_queue_tread,NULL,&test_queue_read,NULL);
	pthread_create(&test_queue_twrite,NULL,&test_queue_write,NULL);
	pthread_join(test_queue_twrite,NULL);
	pthread_join(test_queue_tread,NULL);
	if (test_queue_wqueue!=test_queue_rqueue) mq_close(test_queue_wqueue);
	mq_close(test_queue_rqueue);
	mq_unlink("relaytest");
}

volatile bool test_pipepoll_halt;
int test_pipepoll_pipe[2];
pthread_t test_pipepoll_tread,test_pipepoll_twrite;
struct pollfd test_pipepoll_poll,test_pipepoll_poll1;

void* test_pipepoll_read(void* p) {
	int val;
	while(!test_pipepoll_halt) {
		read(fd_in,&val,4);
		if (poll(&test_pipepoll_poll1, 1, 100) && (test_pipepoll_poll1.revents&POLLOUT)) {
			write(test_pipepoll_pipe[1],&val,4);
		}
	}
	return NULL;
}

void* test_pipepoll_write(void* p) {
	int val;
	while(writecount) {
		if (poll(&test_pipepoll_poll, 1, 100) && (test_pipepoll_poll.revents&POLLIN)) {
			test_pipepoll_poll.revents=0;
			read(test_pipepoll_pipe[0],&val,4);
			write(fd_out,&val,4);
			writecount--;
		}
	}
	test_pipepoll_halt=true;
	if (poll(&test_pipepoll_poll, 1, 100) && (test_pipepoll_poll.revents&POLLIN)) {
		test_pipepoll_poll.revents=0;
		read(test_pipepoll_pipe[0],&val,4);
		write(fd_out,&val,4);
		writecount--;
	}
	return NULL;
}

void test_pipepoll() {
	test_pipepoll_halt=false;
	pipe(test_pipepoll_pipe);
	test_pipepoll_poll.fd=test_pipepoll_pipe[0];
	test_pipepoll_poll.events=POLLIN;
	test_pipepoll_poll1.fd=test_pipepoll_pipe[1];
	test_pipepoll_poll1.events=POLLOUT;
	pthread_create(&test_pipepoll_tread,NULL,&test_pipepoll_read,NULL);
	pthread_create(&test_pipepoll_twrite,NULL,&test_pipepoll_write,NULL);
	pthread_join(test_pipepoll_twrite,NULL);
	pthread_join(test_pipepoll_tread,NULL);
	close(test_pipepoll_pipe[0]);
	close(test_pipepoll_pipe[1]);
}

volatile bool test_pipebidi_halt;
int test_pipebidi_pipe[2];
int test_pipebidi_ackpipe[2];
pthread_t test_pipebidi_tread,test_pipebidi_twrite;
struct pollfd test_pipebidi_poll,test_pipebidi_ackpoll;

void* test_pipebidi_read(void* p) {
	int val;
	while(!test_pipebidi_halt) {
		if (poll(&test_pipebidi_ackpoll, 1, 100) && (test_pipebidi_ackpoll.revents&POLLIN)) {
			read(test_pipebidi_ackpipe[0],&val,1);
			read(fd_in,&val,4);
			write(test_pipebidi_pipe[1],&val,4);
		}
	}
	return NULL;
}

void* test_pipebidi_write(void* p) {
	int val;
	while(writecount) {
		if (poll(&test_pipebidi_poll, 1, 100) && (test_pipebidi_poll.revents&POLLIN)) {
			test_pipebidi_poll.revents=0;
			read(test_pipebidi_pipe[0],&val,4);
			write(fd_out,&val,4);
			write(test_pipebidi_ackpipe[1],&val,1);
			writecount--;
		}
	}
	test_pipebidi_halt=true;
	return NULL;
}

void test_pipebidi() {
	int dummy=0;
	test_pipebidi_halt=false;
	pipe(test_pipebidi_pipe);
	pipe(test_pipebidi_ackpipe);
	test_pipebidi_poll.fd=test_pipebidi_pipe[0];
	test_pipebidi_poll.events=POLLIN;
	test_pipebidi_ackpoll.fd=test_pipebidi_ackpipe[0];
	test_pipebidi_ackpoll.events=POLLIN;
	write(test_pipebidi_ackpipe[1],&dummy,1);
	pthread_create(&test_pipebidi_tread,NULL,&test_pipebidi_read,NULL);
	pthread_create(&test_pipebidi_twrite,NULL,&test_pipebidi_write,NULL);
	pthread_join(test_pipebidi_twrite,NULL);
	pthread_join(test_pipebidi_tread,NULL);
	close(test_pipebidi_pipe[0]);
	close(test_pipebidi_pipe[1]);
	close(test_pipebidi_ackpipe[0]);
	close(test_pipebidi_ackpipe[1]);
}

volatile bool test_pipesema_halt;
int test_pipesema_pipe[2];
pthread_t test_pipesema_tread,test_pipesema_twrite;
struct pollfd test_pipesema_poll;
sem_t test_pipesema_rsemaphore;

void* test_pipesema_read(void* p) {
	int val;
	struct timespec ts;
	while(!test_pipesema_halt) {
		clock_gettime(CLOCK_REALTIME,&ts);
		ts.tv_sec+=1;
		if (!sem_timedwait(&test_pipesema_rsemaphore,&ts)) {
			read(fd_in,&val,4);
			write(test_pipesema_pipe[1],&val,4);
		}
	}
	return NULL;
}

void* test_pipesema_write(void* p) {
	int val;
	while(writecount) {
		if (poll(&test_pipesema_poll, 1, 100) && (test_pipesema_poll.revents&POLLIN)) {
			test_pipesema_poll.revents=0;
			read(test_pipesema_pipe[0],&val,4);
			write(fd_out,&val,4);
			writecount--;
			sem_post(&test_pipesema_rsemaphore);
		}
	}
	test_pipesema_halt=true;
	return NULL;
}

void test_pipesema() {
	test_pipesema_halt=false;
	pipe(test_pipesema_pipe);
	test_pipesema_poll.fd=test_pipesema_pipe[0];
	test_pipesema_poll.events=POLLIN;
	sem_init(&test_pipesema_rsemaphore,0,1);
	pthread_create(&test_pipesema_tread,NULL,&test_pipesema_read,NULL);
	pthread_create(&test_pipesema_twrite,NULL,&test_pipesema_write,NULL);
	pthread_join(test_pipesema_twrite,NULL);
	pthread_join(test_pipesema_tread,NULL);
	close(test_pipesema_pipe[0]);
	close(test_pipesema_pipe[1]);
}

void run_test(char* name,int nw,void (*func)(void)) {
	struct tms tmsBegin,tmsEnd;
	clock_t tickBegin,tickEnd;
	struct timeval tvBegin,tvEnd;
	writecount=nw;
	printf("%s",name);
	tickBegin=times(&tmsBegin);
	gettimeofday(&tvBegin,NULL);
	func();
	gettimeofday(&tvEnd,NULL);
	tickEnd=times(&tmsEnd);
	long int usecs=(tvEnd.tv_usec+tvEnd.tv_sec*1000000)-(tvBegin.tv_usec+tvBegin.tv_sec*1000000);
	clock_t ticks=tickEnd-tickBegin;
	clock_t userTicks=tmsEnd.tms_utime-tmsBegin.tms_utime;
	clock_t sysTicks=tmsEnd.tms_stime-tmsBegin.tms_stime;
	double secs=(double)(usecs)/1000000;
	double userPct=((double)userTicks)/((double)ticks);
	double sysPct=((double)sysTicks)/((double)ticks);
	printf(" - Time %4ldms - writes/s %7.0f - User %5.1f%% - Sys %5.1f%% - Tot %5.1f%%\n",usecs/1000,(double)nw/secs,100*userPct,100*sysPct,100*(userPct+sysPct));
}

void* slowread_func(void* na) {
	int data=0;
	while (!slowread_halt) {
		write(slowread_fd[1],&data,4);
		sleep(1);
	}
	return NULL;
}

void* slowwrite_func(void* na) {
	char buf[512];
	printf("swpt %d %d\n",slowwrite_fd[0],slowwrite_fd[1]);
	putchar('w');
	while (!slowwrite_halt) {
		putchar('.');
		read(slowwrite_fd[0],&buf,sizeof(buf));
		sleep(1);
	}
	putchar('W');
	return NULL;
}

extern "C" int main(int argc, char **argv) {
	fd_in=open("/dev/zero",O_RDONLY);
	fd_out=open("/dev/null",O_WRONLY);

	/*
	printf("~500 ms runs\n");

	printf("example only\n");
	run_test("Block    ",200000,test_block);
	run_test("Read     ",400000,test_ro);
	run_test("Write    ",400000,test_wo);
	run_test("Thread   ",130000,test_thread);
	run_test("SemaOne  ",110000,test_semaphoreone);
	run_test("Pipe     ",32000,test_pipe);
	run_test("PipePoll ",17000,test_pipepoll);

	printf("\nblocking\n");
	run_test("Mutex    ",12000,test_mutex);
	run_test("Sema     ",11500,test_semaphore);

	printf("\ntimeout\n");
	run_test("Flag     ",30000,test_flag);
	run_test("Atomic   ",30000,test_atomic);
	run_test("SemaTO   ",30000,test_semaphoreto);
	run_test("PipeBidi ",5700,test_pipebidi);
*/
	sleepsize=-1;
	printf("~500 ms runs\n");
	run_test("SemaTO   ",18000,test_semaphoreto);
	run_test("PipeBidi ",11500,test_pipebidi);
	run_test("PipeSema ",12500,test_pipesema);
	test_queue_count=1;
	test_queue_size=1;
	run_test("Queue1-1 ",10000,test_queue);
	test_queue_count=1;
	test_queue_size=2;
	run_test("Queue1-2 ",10000,test_queue);

/*
	sleepsize=-1;
	printf("\ntimeout\n");
	run_test("Flag     ",30000,test_flag);
	run_test("Atomic   ",30000,test_atomic);
	run_test("MutexTO  ",30000,test_mutexto);
	run_test("SemaTO   ",30000,test_semaphoreto);

	sleepsize=0;
	printf("\ntimeout\n");
	run_test("Flag     ",3000,test_flag);
	run_test("Atomic   ",3000,test_atomic);
	run_test("MutexTO  ",3000,test_mutexto);
	run_test("SemaTO   ",3000,test_semaphoreto);

	sleepsize=10;
	printf("\ntimeout\n");
	run_test("Flag     ",3000,test_flag);
	run_test("Atomic   ",3000,test_atomic);
	run_test("MutexTO  ",3000,test_mutexto);
	run_test("SemaTO   ",3000,test_semaphoreto);

	sleepsize=100;
	printf("\ntimeout\n");
	run_test("Flag     ",3000,test_flag);
	run_test("Atomic   ",3000,test_atomic);
	run_test("MutexTO  ",3000,test_mutexto);
	run_test("SemaTO   ",3000,test_semaphoreto);

	sleepsize=1000;
	printf("\ntimeout\n");
	run_test("Flag     ",3000,test_flag);
	run_test("Atomic   ",3000,test_atomic);
	run_test("MutexTO  ",3000,test_mutexto);
	run_test("SemaTO   ",3000,test_semaphoreto);
*/
	close(fd_in);
	close(fd_out);

	slowread_halt=false;
	pipe(slowread_fd);
	pthread_create(&slowread_thread,NULL,&slowread_func,NULL);

	fd_in=slowread_fd[0];
	fd_out=open("/dev/null",O_WRONLY);

	printf("\n\n5 reads with 1s/read\n");


	int tmp;
	read(fd_in,&tmp,4);
	run_test("SemaTO   ",5,test_semaphoreto);
	run_test("PipeBidi ",5,test_pipebidi);
	run_test("PipeSema ",5,test_pipesema);
	run_test("Queue    ",5,test_queue);

	/*
	printf("example only\n");
	run_test("Block    ",5,test_block);
	run_test("Read     ",5,test_ro);
	//run_test("Write    ",400000,test_wo);
	//run_test("Thread   ",130000,test_thread);
	run_test("SemaOne  ",5,test_semaphoreone);
	run_test("Pipe     ",5,test_pipe);
	run_test("PipePoll ",5,test_pipepoll);

	printf("\nblocking\n");
	run_test("Mutex    ",5,test_mutex);
	run_test("Sema     ",5,test_semaphore);

	printf("\ntimeout\n");
	run_test("Flag     ",5,test_flag);
	run_test("Atomic   ",5,test_atomic);
	run_test("MutexTO  ",5,test_mutexto);
	run_test("SemaTO   ",5,test_semaphoreto);
	run_test("PipeBidi ",5,test_pipebidi);
	 */

	/*
	sleepsize=-1;
	printf("\ntimeout sleep %d\n",sleepsize);
	run_test("Atomic   ",5,test_atomic);

	sleepsize=0;
	printf("\ntimeout sleep %d\n",sleepsize);
	run_test("Atomic   ",5,test_atomic);

	sleepsize=8;
	printf("\ntimeout sleep %d\n",sleepsize);
	run_test("Atomic   ",5,test_atomic);

	sleepsize=16;
	printf("\ntimeout sleep %d\n",sleepsize);
	run_test("Atomic   ",5,test_atomic);

	sleepsize=32;
	printf("\ntimeout sleep %d\n",sleepsize);
	run_test("Atomic   ",5,test_atomic);

	sleepsize=64;
	printf("\ntimeout sleep %d\n",sleepsize);
	run_test("Atomic   ",5,test_atomic);

	sleepsize=128;
	printf("\ntimeout sleep %d\n",sleepsize);
	run_test("Atomic   ",5,test_atomic);

	sleepsize=256;
	printf("\ntimeout sleep %d\n",sleepsize);
	run_test("Atomic   ",5,test_atomic);

	sleepsize=512;
	printf("\ntimeout sleep %d\n",sleepsize);
	run_test("Atomic   ",5,test_atomic);

	sleepsize=1024;
	printf("\ntimeout sleep %d\n",sleepsize);
	run_test("Atomic   ",5,test_atomic);
	*/

	slowread_halt=true;
	pthread_join(slowread_thread,NULL);
	close(fd_in);
	close(fd_out);
	close(slowread_fd[1]);

	return 0;
}


