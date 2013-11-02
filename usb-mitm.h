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
#include "device_enumeration.h"