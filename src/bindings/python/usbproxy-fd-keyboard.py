#!/usr/bin/env python3
#
# usbproxy-fd-keyboard.py

from USBProxyApp import USBProxyApp
from USBKeyboard import USBKeyboardDevice

u = USBProxyApp(verbose=1)

d = USBKeyboardDevice(u, verbose=4)

d.connect()

try:
    d.run()
# SIGINT raises KeyboardInterrupt
except KeyboardInterrupt:
    d.disconnect()
