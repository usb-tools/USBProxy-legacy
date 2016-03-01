/*
 * This file is part of USBProxy.
 */

#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <poll.h>
#include "errno.h"
#include "netinet/in.h"

#include "Injector_UDP.h"
#include "Packet.h"
#include "HexString.h"

Injector_UDP::Injector_UDP(ConfigParser *cfg) {
	std::string port_str = cfg->get("Injector_UDP::Port");
	if(port_str == "") {
		fprintf(stderr, "Error: no port found for Injector_UDP\n");
		return;
	}
	else
		port = std::stoi(port_str, nullptr, 10);
	
	sck=0;
	buf=NULL;
	spoll.events=POLLIN;
}

Injector_UDP::~Injector_UDP() {
	if (sck) {close(sck);sck=0;}
	if (buf) {free(buf);buf=NULL;}
}

void Injector_UDP::start_injector() {
	fprintf(stderr,"Opening injection UDP socket on port %d.\n",port);
	sck=socket(AF_INET,SOCK_DGRAM | SOCK_CLOEXEC,IPPROTO_UDP);
	spoll.fd=sck;
	if (sck<0) {
		fprintf(stderr,"Error creating socket.\n");
		sck=0;
	}
	struct sockaddr_in addr;
	memset((char *)&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(port);
	if (bind(sck,(struct sockaddr*)&addr,sizeof(addr))<0) {
		fprintf(stderr,"Error binding to port %d.\n",port);
		sck=0;
	}
	//sized to handle ETHERNET less IP(20 byte)/UDP(8 byte) headers
	buf=(__u8*)malloc(UDP_BUFFER_SIZE);
}

int* Injector_UDP::get_pollable_fds() {
	int* tmp=(int*)calloc(2,sizeof(int));
	tmp[0]=sck;
	return tmp;
}

void Injector_UDP::stop_injector() {
	if (sck) {close(sck);sck=0;}
	if (buf) {free(buf);buf=NULL;}
}
void Injector_UDP::get_packets(Packet** packet,SetupPacket** setup,int timeout) {
	*packet=NULL;
	*setup=NULL;

	if (!poll(&spoll, 1, timeout) || !(spoll.revents&POLLIN)) {
		return;
	}

	ssize_t len=recv(sck,buf,UDP_BUFFER_SIZE,0);
	if (len>0) {
		if (buf[0]) {
			__u16 usblen=buf[2]<<8 | buf[3];
			__u8* usbbuf=(__u8*)malloc(usblen);
			memcpy(usbbuf,buf+4,usblen);
			*packet=new Packet(buf[0],usbbuf,usblen,buf[1]&0x01?false:true);
			(*packet)->transmit=buf[1]&0x02?false:true;
			return;
		} else {
			struct usb_ctrlrequest ctrl_req;
			memcpy(&ctrl_req,buf+3,8);
			if (ctrl_req.bRequestType&0x80) {
				*setup=new SetupPacket(ctrl_req,NULL,true);
			} else {
				__u8* usbbuf=(__u8*)malloc(ctrl_req.wLength);
				memcpy(usbbuf,buf+11,ctrl_req.wLength);
				*setup=new SetupPacket(ctrl_req,usbbuf,true);
			}
			(*setup)->filter_out=~(buf[1]&0x01);
			(*setup)->transmit_out=~(buf[1]&0x02);
			(*setup)->filter_in=~(buf[1]&0x04);
			(*setup)->transmit_in=~(buf[1]&0x08);
			return;
		}
	}
	if (len<0) {
		fprintf(stderr,"Socket read error [%s].\n",strerror(errno));
	}
	return;
}

void Injector_UDP::full_pipe(Packet* p) {fprintf(stderr,"Packet returned due to full pipe & buffer\n");}

static Injector_UDP *injector;

extern "C" {
	int plugin_type = PLUGIN_INJECTOR;
	
	Injector * get_plugin(ConfigParser *cfg) {
		injector = new Injector_UDP(cfg);
		return (Injector *) injector;
	}
	
	void destroy_plugin() {
		delete injector;
	}
}
