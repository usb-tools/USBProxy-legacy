#
# Copyright 2014 Dominic Spill
#
# This file is part of USBProxy.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#
# Based upon the hackrf/ubertooth cmake setup.

project(usbproxy_devices C)
set(PACKAGE usbproxy_devices)

include_directories(${libusbproxy_SOURCE_DIR})
LIST(APPEND PLUGINS_LINK_LIBS USBProxy)

LIST(APPEND PLUGINS DeviceProxy_Loopback DeviceProxy_TCP)

find_package(USB1)

if( ${LIBUSB_FOUND} )
	LIST(APPEND PLUGINS DeviceProxy_LibUSB)
	include_directories(${LIBUSB_INCLUDE_DIR})
	LIST(APPEND PLUGINS_LINK_LIBS ${LIBUSB_LIBRARIES})
else( ${LIBUSB_FOUND} )
	if( USE_LIBUSB1 AND NOT ${LIBUSB_FOUND} )
		message( FATAL_ERROR
			"Cannot find libUSB-1.0, which is required for USE_LIBUSB1")
	endif()
endif( ${LIBUSB_FOUND} )

foreach(plugin ${PLUGINS})
	add_library(${plugin} SHARED ${plugin}.cpp)
	set_source_files_properties(${plugin}.cpp PROPERTIES COMPILE_FLAGS  "-std=c++0x")
	set_target_properties(${plugin} PROPERTIES PREFIX "")
	target_link_libraries(${plugin} ${PLUGINS_LINK_LIBS})
	install(TARGETS ${plugin}
        LIBRARY DESTINATION lib/USBProxy${LIB_SUFFIX}
        COMPONENT sharedlibs
        )
endforeach(plugin)
