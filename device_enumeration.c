#include "device_enumeration.h"

libusb_device_handle* get_device_handle(__u16 vendorId,__u16 productId) {
	libusb_device_handle* devh=NULL;
	
	if (productId && vendorId) {
		devh=libusb_open_device_with_vid_pid(NULL,vendorId,productId);
		if (devh==NULL) {
			fprintf(stderr,"Device not found for Vendor ID [%04x] and Product ID [%04x].\n",vendorId,productId);
			return NULL;
		}
	} 
	if (!productId && !vendorId) {
		int found_device_count=open_single_nonhub_device(&devh);
		if (devh==NULL) {
			fprintf(stderr,"Device auto-detection failed, requires exactly one non-hub device, %d were found.\n",found_device_count);
			return NULL;
		}
	}
	libusb_set_auto_detach_kernel_driver(devh,1);
	return devh;	
}

int open_single_nonhub_device(libusb_device_handle** devh) {
	libusb_device **list=NULL;
	libusb_device *found=NULL;

	ssize_t cnt=libusb_get_device_list(NULL,&list);
	if (cnt<0) {fprintf(stderr,"Error %d retrieving device list.",cnt);return cnt;}
	
	ssize_t i=0;
	
	struct libusb_device_descriptor desc;
	int device_count=0;
	int rc=0;
	
	for(i = 0; i < cnt; i++){
		libusb_device *device = list[i];
		rc = libusb_get_device_descriptor(device,&desc);
		if (rc) {fprintf(stderr,"Error %d retrieving device descriptor.",rc);return rc;}
		if (desc.bDeviceClass!=LIBUSB_CLASS_HUB) {
			device_count++;
			found=device;
		}
	}
	if (device_count==1) {	
		rc=libusb_open(found,devh);
		if (rc) {fprintf(stderr,"Error %d opening device handle.",rc);return rc;}
	}
	libusb_free_device_list(list,1);
	return device_count;
}

void print_device_info(libusb_device_handle* devh) {
	uint8_t str_mfr[200];
	uint8_t str_prd[200];
	struct libusb_device_descriptor desc;
	libusb_device* dev=libusb_get_device(devh);
	int rc=libusb_get_device_descriptor (dev,&desc);
	if (rc) {fprintf(stderr,"Error %d retrieving device descriptor.",rc);return;}
	rc=libusb_get_string_descriptor_ascii(devh,desc.iManufacturer,str_mfr,200);
	if (rc<0) {fprintf(stderr,"Error %d retrieving string descriptor.",rc);return;}
	rc=libusb_get_string_descriptor_ascii(devh,desc.iProduct,str_prd,200);
	if (rc<0) {fprintf(stderr,"Error %d retrieving string descriptor.",rc);return;}
	fprintf(stdout,"%04x:%04x %s - %s\n",desc.idVendor,desc.idProduct,str_mfr,str_prd);
}