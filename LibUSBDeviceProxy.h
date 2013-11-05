#ifndef _LibUSBDeviceProxy_
#define _LibUSBDeviceProxy_

#include "USBDeviceProxy.h"

class LibUSBDeviceProxy:public USBDeviceProxy {
	public:
		void control_request(SETUP_PACKET *setup_packet, __u16 *nbytes, __u8* dataptr);
};

#endif