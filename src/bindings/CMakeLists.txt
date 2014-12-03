#
# This file is part of USBProxy.
#

project(libusbproxyAPI C)
set(PACKAGE libusbproxyAPI)

include_directories(${libusbproxy_SOURCE_DIR} ${usbproxy_filters_SOURCE_DIR})

# Target
set(cpp_sources
	${CMAKE_CURRENT_SOURCE_DIR}/API.cpp
	CACHE INTERNAL "List of CPP sources")

add_definitions( -DPLUGIN_PATH="${CMAKE_INSTALL_PREFIX}/lib/USBProxy/")

# Dynamic library
add_library(USBProxyAPI SHARED ${cpp_sources})

set_source_files_properties(${cpp_sources} PROPERTIES COMPILE_FLAGS  "-std=c++0x")

set_target_properties(USBProxyAPI PROPERTIES VERSION ${MAJOR_VERSION}.${MINOR_VERSION} SOVERSION 0)

target_link_libraries(USBProxyAPI USBProxy)

install(TARGETS USBProxyAPI
        LIBRARY DESTINATION lib${LIB_SUFFIX}
        COMPONENT sharedlibs
        )
