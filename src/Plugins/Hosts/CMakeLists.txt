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

project(usbproxy_hosts C)
set(PACKAGE usbproxy_hosts)

LIST(APPEND PLUGINS HostProxy_TCP)

include_directories(${libusbproxy_SOURCE_DIR})
LIST(APPEND PLUGINS_LINK_LIBS USBProxy)

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

# HostProxy_GadgetFS is a special case because of the helpers
add_library(HostProxy_GadgetFS SHARED HostProxy_GadgetFS.cpp GadgetFS_helpers.c)
set_target_properties(HostProxy_GadgetFS PROPERTIES PREFIX "")
set_source_files_properties(HostProxy_GadgetFS.cpp PROPERTIES COMPILE_FLAGS  "-std=c++0x")
target_link_libraries(HostProxy_GadgetFS ${PLUGINS_LINK_LIBS})
install(TARGETS HostProxy_GadgetFS
    LIBRARY DESTINATION lib/USBProxy${LIB_SUFFIX}
    COMPONENT sharedlibs
    )