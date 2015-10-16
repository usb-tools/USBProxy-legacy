/*
 * This file is part of USBProxy.
 */

#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <arpa/inet.h>
#include <errno.h>
#include <poll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/time.h>
#include "TCP_Helper.h"
#include "TRACE.h"

#define BASE_PORT 10400
#define TCP_BUFFER_SIZE 1456

TCP_Helper::TCP_Helper(const char* address)
	: debugLevel(0)
{
	p_server = address==NULL;
	if (address) p_address = strdup(address);
	p_is_connected = false;
	int i;
	for (i=0;i<32;i++) {
		ep_listener[i]=0;
		ep_socket[i]=0;
		ep_buf[i]=NULL;
		ep_connect[i]=false;
	}
}

TCP_Helper::~TCP_Helper() {
	disconnect();
	if (p_address) free(p_address);
}

int TCP_Helper::connect(int timeout) {
	int rc;
	if(p_server) {
		server_listen(0);
		if(!ep_listener[0]) {
			usleep(timeout*1000);
			rc=ETIMEDOUT;
		} else {
			rc=server_connect(0,timeout);
		}
	} else {
		rc=client_connect(0,timeout);
	}
	
	p_is_connected=rc==0;
	//sized to handle ETHERNET less IP(20 byte)/TCP(max 24 byte) headers
	if (p_is_connected) ep_buf[0] = (__u8*)calloc(TCP_BUFFER_SIZE,sizeof(__u8));
	return rc;
}

int TCP_Helper::client_connect(int port,int timeout) {
	int sck;
	if (ep_connect[port] && ep_socket[port]) return 0;
	if (!ep_socket[port]) { //create socket
		struct sockaddr_in serv_addr;

		if((sck = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC, 0))< 0)
		{
			printf("\n Error : Could not create socket \n");
			return -1;
		}

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(BASE_PORT + port);
		serv_addr.sin_addr.s_addr = inet_addr(p_address);

		int rc=::connect(sck, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
		fprintf(stderr,"Connect on port %d: %d\n",BASE_PORT + port,rc);

		if (rc==0) { //immediate connect
			ep_socket[port]=sck;
			ep_connect[port]=true;
			return 0;
		}
		if(rc<0 && errno==EINPROGRESS) { //async connect
			ep_socket[port]=sck;
			ep_connect[port]=false;
		}
		if (rc<0 && errno!=EINPROGRESS) {
			fprintf(stderr, "Error : Connect Failed \n");
			return -1;
		}
	}
	if (ep_socket[port]) { //wait for an async connect to complete
		struct pollfd poll_connect;
		poll_connect.fd=ep_socket[port];
		poll_connect.events=POLLOUT;
		struct timeval tvBegin,tvEnd;
		gettimeofday(&tvBegin,NULL);
		if (poll(&poll_connect,1,timeout)) {
			//sleep for the remaining portion of timeout when socket itself has timed out
			if (poll_connect.revents&POLLHUP) {
				close(ep_socket[port]);
				ep_socket[port]=0;
				gettimeofday(&tvEnd,NULL);
				long int usecs=(tvEnd.tv_usec+tvEnd.tv_sec*1000000)-(tvBegin.tv_usec+tvBegin.tv_sec*1000000);
				long int utimeout=(long int)timeout*1000-usecs;
				if (utimeout>0) usleep(utimeout);
				return ETIMEDOUT;
			}
			if (poll_connect.revents==POLLOUT) {
				ep_connect[port]=true;
				return 0;
			}
			fprintf(stderr,"Unexpected client connect poll results: %d\n",poll_connect.revents);
		} else {
			return ETIMEDOUT;
		}
	}
	return -1; //should never really be reached
}

void TCP_Helper::server_listen(int port) {
	int sck;
	if (ep_listener[port]) return;

	struct sockaddr_in serv_addr = {};

	sck = socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC, 0);
	int optval=1;
	setsockopt(sck,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));
	fprintf(stderr, "socket retrieve success\n");
	
	serv_addr.sin_family = AF_INET;    
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	serv_addr.sin_port = htons(BASE_PORT + port);    

	int rc=bind(sck, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if (rc==-1) {
		fprintf(stderr, "Failed to bind\n");
		return;
	}
	
	rc=listen(sck, 10);
	fprintf(stderr,"Listen on port %d: %d\n",BASE_PORT + port,rc);

	if(rc == -1){
		fprintf(stderr, "Failed to listen\n");
		return;
	}
	ep_connect[port]=false;
	ep_listener[port]=sck;
}

int TCP_Helper::server_connect(int port,int timeout) {
	struct pollfd poll_accept;
	poll_accept.fd=ep_listener[port];
	poll_accept.events=POLLIN;
	if (poll(&poll_accept,1,timeout) && poll_accept.revents==POLLIN) {
		struct sockaddr client_addr;
		unsigned int len=sizeof(client_addr);
		int rc=accept(ep_listener[port],&client_addr,&len);
		if (rc<0) {
			fprintf(stderr,"Accept failed.\n");
			return -1;
		} else {
			fcntl(rc,F_SETFD,FD_CLOEXEC);
			ep_socket[port]=rc;
			ep_connect[port]=true;
			return 0;
		}
	} else {
		return ETIMEDOUT;
	}
	return -1;
}

void TCP_Helper::disconnect() {
	int i;
	for(i=0;i<32;i++) {
		if (ep_listener[i]) {close(ep_listener[i]);ep_listener[i]=0;}
		if (ep_socket[i]) {close(ep_socket[i]);ep_socket[i]=0;}
		if (ep_buf[i]) {free(ep_buf[i]);ep_buf[i]=NULL;}
	}
}


int TCP_Helper::client_open_endpoints(__u8* eps,__u8 num_eps,int timeout) {
	int i;
	struct pollfd* poll_connections=NULL;
	__u8* poll_idxs=NULL;
	int poll_count=0;
	int poll_idx=0;

	for (i=0;i<num_eps;i++) {
		int idx=(eps[i]&0x80)?(eps[i]&0xf)|0x10:eps[i];
		if (!ep_connect[idx]) {
			if (!ep_socket[idx]) {
				fprintf(stderr,"Connecting EP%02x\n",eps[i]);
				client_connect(idx,0);
			}
			if (!ep_connect[idx]) poll_count++;
		}
	}
	if (!poll_count) return 0;

	poll_connections=(struct pollfd*)calloc(poll_count,sizeof(struct pollfd));
	poll_idxs=(__u8*)calloc(poll_count,sizeof(__u8));
	for (i=0;i<num_eps;i++) {
		int idx=(eps[i]&0x80)?(eps[i]&0xf)|0x10:eps[i];
		if (!ep_connect[idx]) {
			poll_connections[poll_idx].fd=ep_socket[idx];
			poll_connections[poll_idx].events=POLLOUT;
			poll_idxs[poll_idx]=idx;
			poll_idx++;
		}
	}

	int ep_count=poll_count;
	int rc=-1;
	struct timeval tvBegin,tvEnd;
	gettimeofday(&tvBegin,NULL);
	if (poll(poll_connections,poll_count,timeout)) {
		for (poll_idx=0;poll_idx<poll_count;poll_idx++) {
			if (poll_connections[poll_idx].revents&POLLHUP) {
				int idx=poll_idxs[poll_idx];
				close(ep_socket[idx]);
				ep_socket[idx]=0;
			}
			if (poll_connections[poll_idx].revents==POLLOUT) {
				int idx=poll_idxs[poll_idx];
				if (client_connect(idx,0)==0) {
					fprintf(stderr,"Connected EP%02x\n",(idx&0xf) | ((idx&0x10)<<3));
					ep_buf[idx] = (__u8*)malloc(TCP_BUFFER_SIZE);
					ep_count--;
				}
			}
		}
		gettimeofday(&tvEnd,NULL);
		long int usecs=(tvEnd.tv_usec+tvEnd.tv_sec*1000000)-(tvBegin.tv_usec+tvBegin.tv_sec*1000000);
		long int utimeout=(long int)timeout*1000-usecs;
		if (utimeout>0) usleep(utimeout);
		rc=ep_count;
	} else {
		rc=ETIMEDOUT;
	}
	free(poll_connections);
	poll_connections=NULL;
	free(poll_idxs);
	poll_idxs=NULL;
	return rc;
}

int TCP_Helper::server_open_endpoints(__u8* eps,__u8 num_eps,int timeout) {
	int i;
	struct pollfd* poll_connections=NULL;
	__u8* poll_idxs=NULL;
	int poll_count=0;
	int poll_idx=0;

	for (i=0;i<num_eps;i++) {
		int idx=(eps[i]&0x80)?(eps[i]&0xf)|0x10:eps[i];
		if (!ep_connect[idx]) {
			if (!ep_listener[idx]) {
				fprintf(stderr,"Listening EP%02x\n",eps[i]);
				server_listen(idx);
			}
			poll_count++;
		}
	}
	poll_connections=(struct pollfd*)calloc(poll_count,sizeof(struct pollfd));
	poll_idxs=(__u8*)calloc(poll_count,sizeof(__u8));
	for (i=0;i<num_eps;i++) {
		int idx=(eps[i]&0x80)?(eps[i]&0xf)|0x10:eps[i];
		if (!ep_connect[idx]) {
			TRACE3(i,idx,ep_listener[idx])
			poll_connections[poll_idx].fd=ep_listener[idx];
			poll_connections[poll_idx].events=POLLIN;
			poll_idxs[poll_idx]=idx;
			poll_idx++;
		}
	}

	int ep_count=poll_count;
	int rc=-1;
	TRACE1(poll_count)
	if (poll(poll_connections,poll_count,timeout)) {
		TRACE
		for (poll_idx=0;poll_idx<poll_count;poll_idx++) {
			TRACE1(poll_idx)
			if (poll_connections[poll_idx].revents==POLLIN) {
				int idx=poll_idxs[poll_idx];
				if (server_connect(idx,0)==0) {
					fprintf(stderr,"Connected EP%02x\n",(idx&0xf) | ((idx&0x10)<<3));
					ep_buf[idx] = (__u8*) malloc(TCP_BUFFER_SIZE);
					ep_count--;
				}
			}
		}
		rc=ep_count;
	} else {
		TRACE
		rc=ETIMEDOUT;
	}
	free(poll_connections);
	poll_connections=NULL;
	free(poll_idxs);
	poll_idxs=NULL;
	TRACE1(rc)
	return rc;
}

int TCP_Helper::open_endpoints(__u8* eps,__u8 num_eps,int timeout) {
if (p_server)
	return server_open_endpoints(eps,num_eps,timeout);
else
	return client_open_endpoints(eps,num_eps,timeout);
}

void TCP_Helper::reset() {
	
}

bool TCP_Helper::is_connected() {
	return p_is_connected;
}

void TCP_Helper::send_data(int ep,__u8* data,int length) {
	int port=(ep&0x80)?(ep&0xf)|0x10:ep;
	write(ep_socket[port],data,length);
	if(debugLevel)
		fprintf(stderr,"Wrote %d bytes to FD %d\n",length,ep_socket[port]);
}

void TCP_Helper::receive_data(int ep,__u8** data,int *length,int timeout) {
	int port=(ep&0x80)?(ep&0xf)|0x10:ep;
	struct pollfd poll_accept;
	poll_accept.fd=ep_socket[port];
	poll_accept.events=POLLIN;
	if (poll(&poll_accept,1,timeout) && poll_accept.revents==POLLIN) {
		*data=(__u8*)malloc(TCP_BUFFER_SIZE);
		*length=read(ep_socket[port],*data,TCP_BUFFER_SIZE);
		*data=(__u8*)realloc(*data,*length);
		if(debugLevel)
			if (*length) fprintf(stderr,"Read %d bytes from FD %d\n",*length,ep_socket[port]);
	}
}

