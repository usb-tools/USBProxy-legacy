USBProxy
========

Status
------
This project is in the very early stages (alpha), it should be assumed that it
is currently non-functional. In fact, there are many occasions on which git
head doesn't even compile.

To keep up to date with the project you can join the mailing list at
http://www.freelists.org/list/usbproxy or join us on IRC (#usbproxy on Freenode).
Build
-----
Dependencies:
```
sudo apt-get install build-essential cmake
```
Build:
```
mkdir src/build
cd src/build
cmake ..
make
sudo make install
sudo ldconfig
```

If you want to try out the PCAP logger you will need to install libPCAP
headers:
```
sudo apt-get install libpcap-dev
```

Running the tool
----------------
The best way to get started with USBProxy is by trying it out. Connect a device
to your single board computer and connect it to a target host system. Then try 
running the following to view packets in real time as they are sent between your
device and host.
```
usb-mitm -l
```

If you have a USB keyboard, try running the following to act as a keylogger:
```
usb-mitm -k
```

What?
-----
A USB man in the middle device using embedded Linux devices with on the go
controllers.

Why?
----
Other USB sniffers exist, but I really like cheap ARM Linux hardware platforms
and I wanted to tinker with the USB device side.  There was also an article on
Hack a Day about sniffing USB with the BeagleBoard-xM, but on further 
inspection, it would only build against a relatively old kernel version.

ToDo
----
 * Support alternative chipsets - gadgetfs should take care of this (ish)
 * Clean up code - check return values, etc

License
-------
Unless explicitly stated otherwise, all code in this repository is released
under the GPL v2 license, a copy of which can be found in the LICENSE file. All
files that are released under a different license contain a copy of that license
in the file.

FAQ
---
Q. I need support!

A. Me too buddy, me too.  Let's hug it out.  Your best chance of getting
support is to contact me on IRC (#USBProxy on freenode.net), raise an issue on
GitHub or join the mailing list (http://www.freelists.org/list/usbproxy).

Q. How is this different to using usbmon on the host?

A. It isn't.  Although there are situations where you may not be able to access
the code running on the host system; for example, when reverse engineering USB
devices for use with closed platforms.

Q. Isn't the Beagle already a USB monitor?

A. Initially the BeagleBone Black was intended target device for this software,
but almost any Linux host with both a device and host port will do. The Total 
Phase Beagle USB monitors are excellent devices for sniffing and debugging USB
connections, in fact, one was used by "AlexP" to reverse engineer the Microsoft
Kinect.  However, they are completely unrelated to the BeagleBone Black devices
produced by TI, which are open source single board computer systems.  
