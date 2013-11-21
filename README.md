USB Man-in-the-Middle
=====================

Status
------
This project is in the very early stages, it should be assumesd that it is 
currently non-functional.

What?
-----
A man in the middle device for USB connections using the BeagleBone Black
hardware.  So far it only supports passing through control transfers - I'm still
working on bulk and interrupt transfers.

Why?
----
Other USB sniffers exist, but I really like the BeagleBone Black as a hardware
platform and I wanted to tinker with the USB device side.  There was also an
article on Hack a Day about sniffing USB with the BeagleBoard-xM, but on further
inspection, it would only build against a relatively old kernel version.

ToDo
----
 * Pretty much everything!
 * Bulk transfers
 * Interrupt transfers
 * Asynchronous I/O
 * Man in the middle function hooks
 * Some sort of library to make all of this easier to use
 * Support alternative chipsets

License
-------
All files should have a license displayed at the start of the file.  Any code
that I have written is released under the GPL v2 license.

Some of the supporting code was licensed under the LGPL v2.1, notably USB
string support (currently not working/required).  That code remains under the
same license, as shown in the files.

FAQ
---
Q. How is this different to using usbmon on the host?

A. It isn't.  Although there are situations where you may not be able to access
the code running on the host system; for example, when reverse engineering USB
devices for use with closed platforms.

Q. Will this damamge my BeagleBone Black or host system?

A. Maybe, it crashes my host quite often, although I don't think there is any
damage.  Try to disconnect the USB cable before killing the usb-mitm process.  I
take no responsibility for any damage to your equipment - if in doubt, don't run
the code.

Q. Isn't the Beagle already a USB monitor?

A. The Total Phase Beagle USB monitors are excellent devices for sniffing and
debugging USB connections, in fact, one was used by "AlexP" to reverse engineer
the Microsoft Kinect.  However, they are completely unrelated to the BeagleBone
Black devices produced by TI, which are open source single board computer
systems.  The BeagleBone Black is the intended target device for this software.
