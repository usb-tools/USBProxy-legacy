USB Proxy
=========

What?
-----


Why?
----


How?
----
As root, or using sudo:

# modprobe gadgetfs
# mkdir /gadget
# mount -t gadgetfs none /gadget
# proxy -v <vendorId> -p <productId>

ToDo
----


License
-------
All files should have a license displayed at the start of the file.  Any code
that I have written is released under the GPL v2 license.

Some of the supporting code was licensed under the LGPL v2.1, notably USB
string support.  That code remains under the same license, as shown in the
files.
