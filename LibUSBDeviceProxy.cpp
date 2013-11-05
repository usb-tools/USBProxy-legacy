#include <linux/types.h>
#include <stdio.h>
#include "LibUSBDeviceProxy.h"

void LibUSBDeviceProxy::control_request(SETUP_PACKET *setup_packet, __u16 *nbytes, __u8* dataptr) {
	printf("sending\n");
}
