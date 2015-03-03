/*
 * This file is part of USBProxy.
 */

#include <linux/usb/ch9.h>
#include "PacketFilter.h"
#include "TRACE.h"

void PacketFilter::set_packet_filter(__u8 header[8],__u8 mask[8]) {
	int i;
	packetHeaderMaskLength=1;
	for(i=0;i<8;i++) {
		if (mask[i]) {packetHeaderMaskLength=i;}
	}
}

bool PacketFilter::test_packet(const Packet* packet) {
	const __u8* data;
	if (packet->wLength<packetHeaderMaskLength) {return false;}
	data=packet->data;
	int i;
	for(i=0;i<packetHeaderMaskLength;i++) {
		if (packetHeaderMask[i] && ((packetHeaderMask[i]&data[i])!=(packetHeaderMask[i]&packetHeader[i]))) {return false;}
	}
	return true;
}

bool PacketFilter::test_setup_packet(const SetupPacket* packet,bool direction_out) {
	if (!((packetHeaderSetupOut || (!direction_out)) || (packetHeaderSetupIn || direction_out))) return false;
	const __u8* data;
	data=(__u8*)&(packet->ctrl_req);
	int i;
	for(i=0;i<packetHeaderMaskLength;i++) {
		if (packetHeaderMask[i] && ((packetHeaderMask[i]&data[i])!=(packetHeaderMask[i]&packetHeader[i]))) {return false;}
	}
	return true;
}



