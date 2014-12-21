#!/usr/bin/env python3

import usbproxy
import time

manager = usbproxy.Manager()
config = usbproxy.ConfigParser()

#config.set("HostProxy", "HostProxy_GadgetFS")
config.set("DeviceProxy", "DeviceProxy_LibUSB")
config.set("vendorId", "1d50")
config.set("productId", "6002")

config.set("HostProxy", "HostProxy_TCP");
config.set("HostProxy_TCP::TCPAddress", "127.0.0.1");

config.print_config()

manager.load_plugins(config)

filters = usbproxy.PacketFilter_Callback(config)
print(filters)

#manager.start_control_relaying()
#while(manager.get_status() == usbproxy.USBM_RELAYING):
#	time.sleep(10)
#manager.stop_relaying()
#manager.cleanup()

