/*
 * This file is part of USBProxy.
 */

#include <unistd.h>
#include <sys/syscall.h>
#include <asm/unistd.h>
#include "get_tid.h"

__pid_t gettid() {
	return (pid_t)syscall(__NR_gettid);
}
