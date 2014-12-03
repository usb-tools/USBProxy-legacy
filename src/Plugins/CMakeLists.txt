#
# This file is part of USBProxy.
#

project(usbproxy_plugins C)
set(PACKAGE usbproxy_plugins)

add_subdirectory(Devices)
add_subdirectory(Hosts)
add_subdirectory(Filters)
add_subdirectory(Injectors)
