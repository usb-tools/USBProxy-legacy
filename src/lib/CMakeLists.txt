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

project(libusbproxy C)
set(PACKAGE libusbproxy)

# Targets
set(cpp_sources
	${CMAKE_CURRENT_SOURCE_DIR}/ConfigParser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Criteria.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/DeviceQualifier.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/HexString.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Interface.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Manager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/RelayReader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TCP_Helper.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Configuration.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Device.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Endpoint.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/HID.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/InterfaceGroup.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/PluginManager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/RelayWriter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/USBString.cpp
	CACHE INTERNAL "List of CPP sources")

set(c_sources
	${CMAKE_CURRENT_SOURCE_DIR}/FDInfo.c
	${CMAKE_CURRENT_SOURCE_DIR}/get_tid.c
	${CMAKE_CURRENT_SOURCE_DIR}/HaltSignal.c
	${CMAKE_CURRENT_SOURCE_DIR}/mqueue_helpers.c
	CACHE INTERNAL "List of C sources")

set(headers
	${CMAKE_CURRENT_SOURCE_DIR}/ConfigParser.h
	${CMAKE_CURRENT_SOURCE_DIR}/DefinitionErrors.h
	${CMAKE_CURRENT_SOURCE_DIR}/DeviceQualifier.h
	${CMAKE_CURRENT_SOURCE_DIR}/HaltSignal.h
	${CMAKE_CURRENT_SOURCE_DIR}/Injector.h
	${CMAKE_CURRENT_SOURCE_DIR}/PacketFilter.h
	${CMAKE_CURRENT_SOURCE_DIR}/PluginManager.h
	${CMAKE_CURRENT_SOURCE_DIR}/TCP_Helper.h
	${CMAKE_CURRENT_SOURCE_DIR}/Configuration.h
	${CMAKE_CURRENT_SOURCE_DIR}/Device.h
	${CMAKE_CURRENT_SOURCE_DIR}/Endpoint.h
	${CMAKE_CURRENT_SOURCE_DIR}/HexString.h
	${CMAKE_CURRENT_SOURCE_DIR}/Interface.h
	${CMAKE_CURRENT_SOURCE_DIR}/Plugins.h
	${CMAKE_CURRENT_SOURCE_DIR}/Criteria.h
	${CMAKE_CURRENT_SOURCE_DIR}/DeviceProxy.h
	${CMAKE_CURRENT_SOURCE_DIR}/get_tid.h
	${CMAKE_CURRENT_SOURCE_DIR}/HostProxy.h
	${CMAKE_CURRENT_SOURCE_DIR}/Manager.h
	${CMAKE_CURRENT_SOURCE_DIR}/Packet.h
	${CMAKE_CURRENT_SOURCE_DIR}/Proxy.h
	${CMAKE_CURRENT_SOURCE_DIR}/USBString.h
	CACHE INTERNAL "List of headers")

# Dynamic library
add_library(USBProxy SHARED ${cpp_sources} ${c_sources})

set_source_files_properties(${cpp_sources} PROPERTIES COMPILE_FLAGS  "-std=c++0x")

set_target_properties(USBProxy PROPERTIES VERSION ${MAJOR_VERSION}.${MINOR_VERSION} SOVERSION 0)
set_target_properties(USBProxy PROPERTIES CLEAN_DIRECT_OUTPUT 1)

target_link_libraries(USBProxy ${LIBUSBPROXY_LIBS})

install(TARGETS USBProxy
        LIBRARY DESTINATION lib${LIB_SUFFIX}
        COMPONENT sharedlibs
        )
install(FILES ${headers}
		DESTINATION include/USBProxy
        COMPONENT headers
        )
