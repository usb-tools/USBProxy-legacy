#ifndef _Packets_
#define _Packets_

// USB Setup Packet
typedef struct __attribute__((packed)) {
  __u8 bmRequestType;
	__u8 bRequest;
  __le16 wValue;
	__le16 wIndex;
	__le16 wLength;
} SETUP_PACKET;


#endif