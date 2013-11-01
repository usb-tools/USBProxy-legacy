/*
 * Copyright 2013 Dominic Spill
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
 */

/*
 * This file is based on http://www.linux-usb.org/gadget/usb.c
 * That file lacks any copyright information - but it belongs to someone
 * probably David Brownell - so thank you very much to him too!
 */

/* $(CROSS_COMPILE)cc -Wall -g -o proxy proxy.c usbstring.c -lpthread */

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include "descriptors.h"

static int debug;

/*-------------------------------------------------------------------------*/

#define	CONFIG_VALUE		3

static int	HIGHSPEED;
static char	*DEVNAME;

/* gadgetfs currently has no chunking (or O_DIRECT/zerocopy) support
 * to turn big requests into lots of smaller ones; so this is "small".
 */
#define	USB_BUFSIZE	(7 * 1024)

static enum usb_device_speed	current_speed;

static int autoconfig()
{
	struct stat	statb;

	/* Something based on Mentor USB Highspeed Dual-Role Controller */
	if (stat (DEVNAME = "musb-hdrc", &statb) == 0) {
		HIGHSPEED = 1;
	} else {
		DEVNAME = 0;
		return -ENODEV;
	}
	return 0;
}

/*-------------------------------------------------------------------------*/

struct thread_handle {
	pthread_t thread;
	struct pollfd poll;
	char desc[7];
};

static struct thread_handle ep0;
static struct thread_handle *eps;

static libusb_context *ctx;
static libusb_device_handle* devh;

// FIXME no status i/o yet

static void close_fd (void *fd_ptr)
{
	int	status, fd;

	fd = *(int *)fd_ptr;
	*(int *)fd_ptr = -1;

	/* test the FIFO ioctls (non-ep0 code paths) */
	if (pthread_self () != ep0.thread) {
		status = ioctl (fd, GADGETFS_FIFO_STATUS);
		if (status < 0) {
			/* ENODEV reported after disconnect */
			if (errno != ENODEV && errno != -EOPNOTSUPP)
				perror ("get fifo status");
		} else {
			fprintf (stderr, "fd %d, unclaimed = %d\n",
				fd, status);
			if (status) {
				status = ioctl (fd, GADGETFS_FIFO_FLUSH);
				if (status < 0)
					perror ("fifo flush");
			}
		}
	}

	if (close (fd) < 0)
		perror ("close");
}


/* you should be able to open and configure endpoints
 * whether or not the host is connected
 */
static int
ep_config (char *name, const char *label, char *ep)
{
	int		fd, status;
	char		buf [USB_BUFSIZE];

	/* open and initialize with endpoint descriptor(s) */
	fd = open (name, O_RDWR);
	if (fd < 0) {
		status = -errno;
		fprintf (stderr, "%s open %s error %d (%s)\n",
			label, name, errno, strerror (errno));
		return status;
	}

	/* one (fs or ls) or two (fs + hs) sets of config descriptors */
	*(__u32 *)buf = 1;	/* tag for this format */
	memcpy(buf + 4,  ep, USB_DT_ENDPOINT_SIZE);
	memcpy(buf + 4 + USB_DT_ENDPOINT_SIZE, ep, USB_DT_ENDPOINT_SIZE);
	status = write (fd, buf, 4 + USB_DT_ENDPOINT_SIZE + USB_DT_ENDPOINT_SIZE);
	if (status < 0) {
		status = -errno;
		fprintf (stderr, "%s config %s error %d (%s)\n",
			label, name, errno, strerror (errno));
		close (fd);
		return status;
	} else if (debug) {
		unsigned long	id;

		id = pthread_self ();
		fprintf (stderr, "%s start %ld fd %d\n", label, id, fd);
	}
	return fd;
}

static void *in_thread (void *param)
{
	struct thread_handle *ep = (struct thread_handle *) param;
	char name[8];
	int	 status;

	sprintf(name, "ep%d%s", (ep->desc[2] && 0x7f), "in");
	status = ep_config(name, "source_open", ep->desc);
	if (status < 0)
		return 0;
	ep->poll.fd = status;

	pthread_cleanup_push (close_fd, &ep->poll.fd);
	do {
		/* original LinuxThreads cancelation didn't work right
		 * so test for it explicitly.
		 */
		pthread_testcancel ();

		/* FIXME: actually do something useful */
		status = write(ep->poll.fd, name, 1);

	} while (status > 0);
	if (status == 0) {
		if (debug)
			fprintf (stderr, "done %s\n", __FUNCTION__);
	} else if (debug > 2 || errno != ESHUTDOWN) /* normal disconnect */
		perror ("write");
	fflush (stdout);
	fflush (stderr);
	pthread_cleanup_pop (1);

	return 0;
}

static void *out_thread (void *param)
{
	struct thread_handle *ep = (struct thread_handle *) param;
	char name[8];
	int		status;
	char		buf [1];

	sprintf(name, "ep%d%s", (ep->desc[2] && 0x7f), "in");
	status = ep_config(name, "sink_open", ep->desc);
	if (status < 0)
		return 0;
	ep->poll.fd = status;

	/* synchronous reads of endless streams of data */
	pthread_cleanup_push (close_fd, &ep->poll.fd);
	do {
		/* original LinuxThreads cancelation didn't work right
		 * so test for it explicitly.
		 */
		pthread_testcancel ();
		errno = 0;
		status = read (ep->poll.fd, buf, 1);
		if (status < 0)
			break;
	} while (status > 0);
	if (status == 0) {
		if (debug)
			fprintf (stderr, "done %s\n", __FUNCTION__);
	} else if (debug > 2 || errno != ESHUTDOWN) /* normal disconnect */
		perror ("read");
	fflush (stdout);
	fflush (stderr);
	pthread_cleanup_pop (1);

	return 0;
}

static void start_io ()
{
	sigset_t	allsig, oldsig;
	struct endpoint_item *ep_item = ep_head;
	int i;
	char name[8];

	sigfillset (&allsig);
	errno = pthread_sigmask (SIG_SETMASK, &allsig, &oldsig);
	if (errno < 0) {
		perror("set thread signal mask");
		return;
	}

	eps = malloc(ep_list_len * sizeof(struct thread_handle));
	if(eps == NULL)
		fprintf(stderr, "Unable to allocate %d bytes for threads\n", ep_list_len * sizeof(struct thread_handle));
	
	for(i=0; i<ep_list_len; i++) {
		memcpy(ep_item->desc, &eps[i].desc, 7);
		// DGS FIXME: Only supports buk in/out EPs so far
		if(ep_item->desc[2] && 0x80) {
			
			if (pthread_create(&eps[i].thread, 0, in_thread,
							   (void *) &eps[i]) != 0) {
				perror("can't create thread");
				goto cleanup;
			}
		} else {
			sprintf(name, "ep%d%s", (ep_item->desc[2] && 0x7f), "out");
			if (pthread_create(&eps[i].thread, 0, out_thread,
							   (void *) &eps[i]) != 0) {
				perror("can't create thread");
				// DGS FIXME Clean up existing threads
				//pthread_cancel (source);
				//source = ep0.thread;
				goto cleanup;
			}
		}
		ep_item = ep_item->next;
	}

	/* give the other threads a chance to run before we report
	 * success to the host.
	 * FIXME better yet, use pthread_cond_timedwait() and
	 * synchronize on ep config success.
	 */
	sched_yield ();

cleanup:
	errno = pthread_sigmask (SIG_SETMASK, &oldsig, 0);
	if (errno != 0) {
		perror ("restore sigmask");
		exit (-1);
	}
}

static void stop_io()
{
	int i;
	// DGS FIXME Loop through EP threads, kill all
	for(i=0; i<ep_list_len; i++) {
		if (!pthread_equal (eps[i].thread, ep0.thread)) {
			pthread_cancel (eps[i].thread);
			if (pthread_join (eps[i].thread, 0) != 0)
				perror ("can't join source thread");
			eps[i].thread = ep0.thread;
		}
	}
}

/*-------------------------------------------------------------------------*/

static int init_device()
{
	int len, status, fd, i;
	char buf[4096];

	len = clone_descriptors(devh, buf);
	printf("devh cloned\n");

	if(len < 0) {
		fprintf (stderr, "Unable to clone device config (%d)\n", len);
		return len;
	}
	printf("config length:%d\n", len);
	for(i=0; i<len; i++) {
		printf("%02x ", buf[i]);
		if(i%8==7)
			printf("\n");
	}
	printf("\n");

	status = autoconfig();
	if (status < 0) {
		fprintf (stderr, "?? don't recognize /gadget bulk device\n");
		return status;
	}

	fd = open (DEVNAME, O_RDWR);
	if (fd < 0) {
		perror (DEVNAME);
		return -errno;
	}

	status = write(fd, buf, len);
	if (status < 0) {
		perror ("write dev descriptors");
		close (fd);
		return status;
	} else if (status != len) {
		fprintf (stderr, "dev init, wrote %d expected %d\n",
				status, len);
		close (fd);
		return -EIO;
	}
	return fd;
}

static void handle_control (int fd, struct usb_ctrlrequest *setup)
{
	int status, r;
	unsigned char buf[256];
	__u16 value, index, length;

	value = __le16_to_cpu(setup->wValue);
	index = __le16_to_cpu(setup->wIndex);
	length = __le16_to_cpu(setup->wLength);

	if (debug)
		fprintf (stderr, "SETUP %02x.%02x v%04x i%04x %d\n",
				 setup->bRequestType, setup->bRequest, value, index, length);

	r = libusb_control_transfer(devh, setup->bRequestType,
									setup->bRequest, value, index, &buf[0],
									length, 0);
	if(r < 0) {
		fprintf(stderr, "Error forwarding control request (%d)\n", r);
	} else {
		status = write(fd, &buf[0], r);

		if (status != r)
			fprintf (stderr, "Error writing to ep0 (%d of %d written)\n",
					 status, r);
	}
}

static void signothing (int sig, siginfo_t *info, void *ptr)
{
	/* NOP */
	if (debug > 2)
		fprintf (stderr, "%s %d\n", __FUNCTION__, sig);
}

static const char *speed (enum usb_device_speed s)
{
	switch (s) {
	case USB_SPEED_LOW:	return "low speed";
	case USB_SPEED_FULL:	return "full speed";
	case USB_SPEED_HIGH:	return "high speed";
	default:		return "UNKNOWN speed";
	}
}

/*-------------------------------------------------------------------------*/

/* control thread, handles main event loop  */

#define	NEVENT		5
#define	LOGDELAY	(15 * 60)	/* seconds before stdout timestamp */

static void *ep0_thread (void *param)
{
	int fd = *(int*) param;
	struct sigaction	action;
	time_t			now, last;

	ep0.thread = pthread_self();
	pthread_cleanup_push (close_fd, param);

	/* REVISIT signal handling ... normally one pthread should
	 * be doing sigwait() to handle all async signals.
	 */
	action.sa_sigaction = signothing;
	sigfillset (&action.sa_mask);
	action.sa_flags = SA_SIGINFO;
	if (sigaction (SIGINT, &action, NULL) < 0) {
		perror ("SIGINT");
		return 0;
	}
	if (sigaction (SIGQUIT, &action, NULL) < 0) {
		perror ("SIGQUIT");
		return 0;
	}

	ep0.poll.fd = fd;
	ep0.poll.events = POLLIN | POLLOUT | POLLHUP;

	start_io();
	/* event loop */
	last = 0;
	while(1) {
		struct usb_gadgetfs_event	event [NEVENT];
		int tmp, i, nevent, connected = 0;

		/* Use poll() to test that mechanism, to generate
		 * activity timestamps, and to make it easier to
		 * tweak this code to work without pthreads.  When
		 * AIO is needed without pthreads, ep0 can be driven
		 * instead using SIGIO.
		 */
		tmp = poll(&ep0.poll, 1, -1);
		if (debug) {
			time (&now);
			if ((now - last) > LOGDELAY) {
				char		timebuf[26];

				last = now;
				ctime_r (&now, timebuf);
				printf ("\n** %s", timebuf);
			}
		}
		if (tmp < 0) {
			/* exit path includes EINTR exits */
			perror("poll");
			break;
		}

		tmp = read (fd, &event, sizeof event);
		if (tmp < 0) {
			if (errno == EAGAIN) {
				sleep (1);
				continue;
			}
			perror ("ep0 read after poll");
			goto done;
		}
		nevent = tmp / sizeof event [0];
		if (nevent != 1 && debug)
			fprintf (stderr, "read %d ep0 events\n",
				nevent);

		for (i = 0; i < nevent; i++) {
			switch (event [i].type) {
			case GADGETFS_NOP:
				if (debug)
					fprintf (stderr, "NOP\n");
				break;
			case GADGETFS_CONNECT:
				connected = 1;
				current_speed = event [i].u.speed;
				if (debug)
					fprintf (stderr,
						"CONNECT %s\n",
					    speed (event [i].u.speed));
				break;
			case GADGETFS_SETUP:
				connected = 1;
				handle_control(fd, &event [i].u.setup);
				break;
			case GADGETFS_DISCONNECT:
				connected = 0;
				current_speed = USB_SPEED_UNKNOWN;
				if (debug)
					fprintf(stderr, "DISCONNECT\n");
				stop_io();
				break;
			case GADGETFS_SUSPEND:
				// connected = 1;
				if (debug)
					fprintf (stderr, "SUSPEND\n");
				break;
			default:
				fprintf (stderr,
					"* unhandled event %d\n",
					event [i].type);
			}
		}
		continue;
done:
		fflush (stdout);
		if (connected)
			stop_io();
		break;
	}
	if (debug)
		fprintf (stderr, "done\n");
	fflush (stdout);

	pthread_cleanup_pop (1);
	return 0;
}

/*-------------------------------------------------------------------------*/

void usage(char *arg) {
	fprintf(stderr, "usage: %s [-v vendorId] [-p productId] [-d (debug)]\n",
			arg);
	
}

int open_single_nonhub_device() {
	libusb_device **list;
	libusb_device *found = NULL;

	ssize_t cnt=libusb_get_device_list(ctx,&list);
	if (cnt<0) {fprintf(stderr,"Error %d retrieving device list.",cnt);return cnt;}
	
	ssize_t i;
	
	struct libusb_device_descriptor desc;
	int device_count=0;
	int rc;
	
	for(i = 0; i < cnt; i++){
		libusb_device *device = list[i];
		rc = libusb_get_device_descriptor(device,&desc);
		if (rc<0) {fprintf(stderr,"Error %d retrieving device descriptor.",rc);return rc;}
		if (desc.bDeviceClass!=LIBUSB_CLASS_HUB) {
			device_count++;
			found=device;
		}
	}
	if (device_count==1) {	
		rc=libusb_open(found,&devh);
		if (rc<0) {fprintf(stderr,"Error %d opening device handle.",rc);return rc;}
	}
	libusb_free_device_list(list,1);
	return device_count;
}

void print_device_info() {
	uint8_t str_mfr[200];
	uint8_t str_prd[200];
	struct libusb_device_descriptor desc;
	libusb_device* dev=libusb_get_device(devh);
	int rc=libusb_get_device_descriptor (dev,&desc);
	if (rc<0) {fprintf(stderr,"Error %d retrieving device descriptor.",rc);return rc;}
	rc=libusb_get_string_descriptor_ascii(devh,desc.iManufacturer,str_mfr,200);
	if (rc<0) {fprintf(stderr,"Error %d retrieving string descriptor.",rc);return rc;}
	rc=libusb_get_string_descriptor_ascii(devh,desc.iProduct,str_prd,200);
	if (rc<0) {fprintf(stderr,"Error %d retrieving string descriptor.",rc);return rc;}
	fprintf(stdout,"%04x:%04x %s - %s\n",desc.idVendor,desc.idProduct,str_mfr,str_prd);
}

int main(int argc, char **argv)
{
	int c;
	char* end;
	__u16 vendorId=0, productId=0;

	while ((c = getopt (argc, argv, "p:v:d")) != EOF) {
		switch (c) {
		case 'p':
			productId = strtol(optarg, &end, 16);
			continue;
		case 'v':
			vendorId = strtol(optarg, &end, 16);
			continue;
		case 'd':		/* verbose */
			debug++;
			continue;
		}
		usage(argv[0]);
		return 1;
	}

	if ((!productId && vendorId) || (productId && !vendorId)) {
		fprintf(stderr, "If you supply Vendor ID or Product ID then you must supply both of them.\n");
		usage(argv[0]);
		return 1;
	}

	//we have enough info to search now

	int rc=libusb_init(&ctx);
	if (rc<0) {fprintf(stderr,"Error %d initializing libusb.",rc);return rc;}
	libusb_set_debug(ctx, debug);

	if (productId && vendorId) {
		devh=libusb_open_device_with_vid_pid(ctx,vendorId,productId);
		if (devh==NULL) {
			fprintf(stderr,"Device not found for Vendor ID [%04x] and Product ID [%04x].\n",vendorId,productId);
			return 1;
		}
	} 
	if (!productId && !vendorId) {
		int found_device_count=open_single_nonhub_device();
		if (devh==NULL) {
			fprintf(stderr,"Device auto-detection failed, requires exactly one non-hub device, %d were found.",found_device_count);
			return 1;
		}
		//libusb_set_auto_detach_kernel_driver(devh,1);
	}

	if (debug) {print_device_info();}
	
	int fd;

	if (chdir("/gadget") < 0) {
		perror ("can't chdir /gadget");
		return 1;
	}

	printf("Calling init_device\n");
	fd = init_device(vendorId, productId);
	if (fd < 0)
		return 1;

	fprintf (stderr, "/gadget/%s ep0 configured\n", DEVNAME);
	fflush (stderr);

	ep0_thread(&fd);
	return 0;
	libusb_exit(ctx);
}
