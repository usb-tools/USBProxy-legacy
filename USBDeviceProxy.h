#ifndef _USBDeviceProxy_
#define _USBDeviceProxy_

#include "Packets.h"

class USBDeviceProxy{
	//send packet
	//recv packet
	//reset
	//recv setup
	public:
		virtual void control_request(SETUP_PACKET *setup_packet, __u16 *nbytes, __u8* dataptr)=0;
	
};

#endif