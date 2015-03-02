#
# This file is part of USBProxy.
#

project(libusbproxy C)
set(PACKAGE libusbproxy)

find_package(Threads REQUIRED)

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
	${CMAKE_CURRENT_SOURCE_DIR}/Injector.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/PacketFilter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/PluginManager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/RelayWriter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/USBString.cpp
	CACHE INTERNAL "List of CPP sources")

set(c_sources
	${CMAKE_CURRENT_SOURCE_DIR}/FDInfo.c
	${CMAKE_CURRENT_SOURCE_DIR}/get_tid.c
	${CMAKE_CURRENT_SOURCE_DIR}/mqueue_helpers.c
	CACHE INTERNAL "List of C sources")

set(headers
	${CMAKE_CURRENT_SOURCE_DIR}/ConfigParser.h
	${CMAKE_CURRENT_SOURCE_DIR}/DefinitionErrors.h
	${CMAKE_CURRENT_SOURCE_DIR}/DeviceQualifier.h
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

add_definitions( -DPLUGIN_PATH="${CMAKE_INSTALL_PREFIX}/lib/USBProxy/")

# Dynamic library
add_library(USBProxy SHARED ${cpp_sources} ${c_sources})

set_source_files_properties(${cpp_sources} PROPERTIES COMPILE_FLAGS  "${CMAKE_CPP_FLAGS}")

set_target_properties(USBProxy PROPERTIES VERSION ${MAJOR_VERSION}.${MINOR_VERSION} SOVERSION 0)

target_link_libraries(USBProxy ${LIBUSBPROXY_LIBS} ${CMAKE_DL_LIBS}
                               ${CMAKE_THREAD_LIBS_INIT} rt)

install(TARGETS USBProxy
        LIBRARY DESTINATION lib${LIB_SUFFIX}
        COMPONENT sharedlibs
        )
install(FILES ${headers}
		DESTINATION include/USBProxy
        COMPONENT headers
        )

########################################################################
# Create Pkg Config File (experimental)
########################################################################
FOREACH(inc ${LIBUSB_INCLUDE_DIR})
    LIST(APPEND USBPROXY_PC_CFLAGS "-I${inc}")
ENDFOREACH(inc)

# use space-separation format for the pc file
STRING(REPLACE ";" " " USBPROXY_PC_CFLAGS "${USBPROXY_PC_CFLAGS}")
STRING(REPLACE ";" " " USBPROXY_PC_LIBS "${USBPROXY_PC_LIBS}")

# unset these vars to avoid hard-coded paths to cross environment
IF(CMAKE_CROSSCOMPILING)
    UNSET(USBPROXY_PC_CFLAGS)
    UNSET(USBPROXY_PC_LIBS)
ENDIF(CMAKE_CROSSCOMPILING)

set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix \${prefix})
set(libdir \${exec_prefix}/lib)
set(includedir \${prefix}/include)

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/usbproxy.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/usbproxy.pc
@ONLY)

INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/usbproxy.pc
    DESTINATION lib/pkgconfig
)
