/*
 * This file is part of USBProxy.
 */

#ifndef HALTSIGNAL_H_
#define HALTSIGNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

int haltsignal_setup(int haltsignal,struct pollfd* haltpoll,int* haltfd);
int haltsignal_check(int haltsignal,struct pollfd* haltpoll,int* haltfd);

#ifdef __cplusplus
}
#endif

#endif /* HALTSIGNAL_H_ */
