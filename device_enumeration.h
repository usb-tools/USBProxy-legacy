#ifndef _MITM_Device_Enumeration_
#define _MITM_Device_Enumeration_

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <libusb-1.0/libusb.h>

#include <linux/types.h>
#include <linux/usb/gadgetfs.h>
#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <memory.h>
#include <asm/byteorder.h>
#include <stdlib.h>

int open_single_nonhub_device(libusb_device_handle** devh);
void print_device_info(libusb_device_handle* devh);
libusb_device_handle* get_device_handle(__u16 vendorID,__u16 productID);

#endif