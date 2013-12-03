USB Man-in-the-Middle Documentation
===================================

Status
------
Eventually this is where we will keep documentation.  For now, it houses a patch
to the gadgetfs kernel module.

inode.c.patch
-------------
This patch should be applied to drivers/usb/gadget/inode.c in the Linux kernel
source tree to avoid locking issues when using GadgetFS.  If you have used an
OS image supplied by the usb-mitm project or don't intend to use the BeagleBone
Black's USB gadget hardware then you can safely ignore this patch.

Apply the patch before building the gadgetfs module from the kernel source tree.