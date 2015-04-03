/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_PROXY_H
#define USBPROXY_PROXY_H

#include "Plugins.h"
#include <stddef.h>
#include <linux/types.h>
#include "ConfigParser.h"

class Configuration;

class Proxy {
public:
	static const __u8 plugin_type=0;

	Proxy()
		: debugLevel(0)
	{}
	Proxy(unsigned _debugLevel)
		: debugLevel(_debugLevel)
	{}

	virtual ~Proxy() {}

	virtual void send_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8* dataptr,int length)=0;
	virtual bool send_wait_complete(__u8 endpoint,int timeout=500) {return true;}
	virtual void receive_data(__u8 endpoint,__u8 attributes,__u16 maxPacketSize,__u8** dataptr, int* length,int timeout=500)=0;
	virtual void setConfig(Configuration* fs_cfg,Configuration* hs_cfg,bool hs)=0;
	virtual char* toString() {return NULL;}
	unsigned debugLevel;

private:
        /// \brief Disallow copying.
        ///
        /// This method is not implemented (besides beeing private).
        Proxy(const Proxy&);

        /// \brief Disallow assignment.
        ///
        /// This method is not implemented (besides beeing private).
        void operator= (const Proxy&);
};

#endif /* USBPROXY_PROXY_H */
