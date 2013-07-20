/*
 * Copyright 2013 Dominic Spill
 *
 * This file is part of USB Proxy.
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
 * That file lacks any copyright information - but it belongs to someone.
 */

/* $(CROSS_COMPILE)cc -Wall -g -o usb usb.c usbstring.c -lpthread */

/*
 * this is an example pthreaded USER MODE driver implementing a
 * USB Gadget/Device with simple bulk source/sink functionality.
 * you could implement pda sync software this way, or some usb class
 * protocols (printers, test-and-measurement equipment, and so on).
 *
 * with hardware that also supports isochronous data transfers, this
 * can stream data using multi-buffering and AIO.  that's the way to
 * handle audio or video data, where on-time delivery is essential.
 *
 * needs "gadgetfs" and a supported USB device controller driver
 * in the kernel; this autoconfigures, based on the driver it finds.
 */

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

//static void *(*in_thread) (void *, void *);
//static void *(*out_thread) (void *, void *);

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

static void stop_io ()
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

static int init_device(__u16 vendorId, __u16 productId)
{
	int len, status, fd, i;
	char buf[4096];

	len = clone_descriptors(vendorId, productId, buf);
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
	int		status, tmp, i;
	__u8		buf [256];
	__u16		value, index, length;

	value = __le16_to_cpu(setup->wValue);
	index = __le16_to_cpu(setup->wIndex);
	length = __le16_to_cpu(setup->wLength);

	if (debug)
		fprintf (stderr, "SETUP %02x.%02x "
				"v%04x i%04x %d\n",
			setup->bRequestType, setup->bRequest,
			value, index, length);

	/*
	if ((setup->bRequestType & USB_TYPE_MASK) != USB_TYPE_STANDARD)
		goto special;
	*/

	switch (setup->bRequest) {	/* usb 2.0 spec ch9 requests */
	case USB_REQ_GET_DESCRIPTOR:
		if (setup->bRequestType != USB_DIR_IN)
			goto stall;
		switch (value >> 8) {
		case USB_DT_STRING:
			tmp = value & 0x0ff;
			if (debug > 1)
				fprintf (stderr,
					"... get string %d lang %04x\n",
					tmp, index);
			if (tmp != 0 && index != strings.language)
				goto stall;
			status = usb_gadget_get_string (&strings, tmp, buf);
			if (status < 0)
				goto stall;
			tmp = status;
			if (length < tmp)
				tmp = length;
			status = write (fd, buf, tmp);
			if (status < 0) {
				if (errno == EIDRM)
					fprintf (stderr, "string timeout\n");
				else
					perror ("write string data");
			} else if (status != tmp) {
				fprintf (stderr, "short string write, %d\n",
					status);
			}
			break;
		default:
			goto stall;
		}
		return;
	case USB_REQ_SET_CONFIGURATION:
		if (setup->bRequestType != USB_DIR_OUT)
			goto stall;
		if (debug)
			fprintf (stderr, "CONFIG #%d\n", value);

		/* Kernel is normally waiting for us to finish reconfiguring
		 * the device.
		 *
		 * Some hardware can't, notably older PXA2xx hardware.  (With
		 * racey and restrictive config change automagic.  PXA 255 is
		 * OK, most PXA 250s aren't.  If it has a UDC CFR register,
		 * it can handle deferred response for SET_CONFIG.)  To handle
		 * such hardware, don't write code this way ... instead, keep
		 * the endpoints always active and don't rely on seeing any
		 * config change events, either this or SET_INTERFACE.
		 */
		switch (value) {
		case CONFIG_VALUE:
			start_io ();
			break;
		case 0:
			stop_io ();
			break;
		default:
			/* kernel bug -- "can't happen" */
			fprintf (stderr, "? illegal config\n");
			goto stall;
		}

		/* ... ack (a write would stall) */
		status = read (fd, &status, 0);
		if (status)
			perror ("ack SET_CONFIGURATION");
		return;
	case USB_REQ_GET_INTERFACE:
		if (setup->bRequestType != (USB_DIR_IN|USB_RECIP_INTERFACE)
				|| index != 0
				|| length > 1)
			goto stall;

		/* only one altsetting in this driver */
		buf [0] = 0;
		status = write (fd, buf, length);
		if (status < 0) {
			if (errno == EIDRM)
				fprintf (stderr, "GET_INTERFACE timeout\n");
			else
				perror ("write GET_INTERFACE data");
		} else if (status != length) {
			fprintf (stderr, "short GET_INTERFACE write, %d\n",
				status);
		}
		return;
	case USB_REQ_SET_INTERFACE:
		if (setup->bRequestType != USB_RECIP_INTERFACE
				|| index != 0
				|| value != 0)
			goto stall;

		/* just reset toggle/halt for the interface's endpoints */
		status = 0;
		for(i=0; i<ep_list_len; i++) {
			if (ioctl (eps[i].poll.fd, GADGETFS_CLEAR_HALT) < 0) {
				status = errno;
				perror("reset pe fd");
			}
		}
		/* FIXME eventually reset the status endpoint too */
		if (status)
			goto stall;

		/* ... and ack (a write would stall) */
		status = read (fd, &status, 0);
		if (status)
			perror ("ack SET_INTERFACE");
		return;
	default:
		goto stall;
	}

stall:
	if (debug)
		fprintf (stderr, "... protocol stall %02x.%02x\n",
			setup->bRequestType, setup->bRequest);

	/* non-iso endpoints are stalled by issuing an i/o request
	 * in the "wrong" direction.  ep0 is special only because
	 * the direction isn't fixed.
	 */
	if (setup->bRequestType & USB_DIR_IN)
		status = read (fd, &status, 0);
	else
		status = write (fd, &status, 0);
	if (status != -1)
		fprintf (stderr, "can't stall ep0 for %02x.%02x\n",
			setup->bRequestType, setup->bRequest);
	else if (errno != EL2HLT)
		perror ("ep0 stall");
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
	int			fd = *(int*) param;
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

	/* event loop */
	last = 0;
	for (;;) {
		int				tmp;
		struct usb_gadgetfs_event	event [NEVENT];
		int				connected = 0;
		int				i, nevent;

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
				handle_control (fd, &event [i].u.setup);
				break;
			case GADGETFS_DISCONNECT:
				connected = 0;
				current_speed = USB_SPEED_UNKNOWN;
				if (debug)
					fprintf(stderr, "DISCONNECT\n");
				stop_io ();
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
			stop_io ();
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
int
main (int argc, char **argv)
{
	int fd, c;
	char *end;
	__u16 vendorId, productId;
	int have_vid, have_pid;

	//source_thread = simple_source_thread;
	//sink_thread = simple_sink_thread;

	have_vid = have_pid = 0;
	while ((c = getopt (argc, argv, "p:v:d")) != EOF) {
		switch (c) {
		case 'p':
			productId = strtol(optarg, &end, 16);
			have_pid++;
			continue;
		case 'v':
			vendorId = strtol(optarg, &end, 16);
			have_vid++;
			continue;
		case 'd':		/* verbose */
			debug++;
			continue;
		}
		usage(argv[0]);
		return 1;
	}

	if(!have_vid || !have_pid) {
		fprintf(stderr, "Both vendorId and productId are required (for now)\n");
		usage(argv[0]);
		return 1;
	}
	if (chdir ("/gadget") < 0) {
		perror ("can't chdir /gadget");
		return 1;
	}

	fd = init_device(vendorId, productId);
	if (fd < 0)
		return 1;

	fprintf (stderr, "/gadget/%s ep0 configured\n", DEVNAME);
	fflush (stderr);

	// FIXME: Temporary stall to allow testing of init(), etc
	while(1)
		sleep(5);
	(void) ep0_thread (&fd);
	return 0;
}
