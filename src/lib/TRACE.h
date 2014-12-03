/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_TRACE_H_
#define USBPROXY_TRACE_H_

#include <stdio.h>

#define TRACE fprintf(stderr,"Trace: %s, line %d\n",__FILE__,__LINE__);
#define TRACE1(X) fprintf(stderr,"Trace(%d): %s, line %d\n",X,__FILE__,__LINE__);
#define TRACE2(X,Y) fprintf(stderr,"Trace(%d,%d): %s, line %d\n",X,Y,__FILE__,__LINE__);
#define TRACE3(X,Y,Z) fprintf(stderr,"Trace(%d,%d,%d): %s, line %d\n",X,Y,Z,__FILE__,__LINE__);

#endif /* USBPROXY_TRACE_H_ */
