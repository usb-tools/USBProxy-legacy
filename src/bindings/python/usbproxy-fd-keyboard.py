#!/usr/bin/env python3
#
# usbproxy-fd-keyboard.py

from USBProxyApp import USBProxyApp
from USBKeyboard import USBKeyboardDevice
import sys

u = USBProxyApp(verbose=1)

if len(sys.argv) > 1:
    text = ' '.join(sys.argv[1:])
else:
    text = None
print(text)
d = USBKeyboardDevice(u, verbose=4, text=text)

d.connect()

try:
    d.run()
# SIGINT raises KeyboardInterrupt
except KeyboardInterrupt:
    d.disconnect()
