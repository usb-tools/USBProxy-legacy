/*
 * This file is part of USBProxy.
 */

#include <unistd.h>
#include <sys/syscall.h>
#include <asm/unistd.h>
#include "get_tid.h"

long int gettid() {
	return (pid_t)syscall(__NR_gettid);
}
