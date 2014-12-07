#
# This file is part of USBProxy.
#

project(usbproxy_devices C)
set(PACKAGE usbproxy_devices)

include_directories(${libusbproxy_SOURCE_DIR})
LIST(APPEND PLUGINS_LINK_LIBS USBProxy)

LIST(APPEND PLUGINS DeviceProxy_Loopback DeviceProxy_TCP DeviceProxy_Callback)

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

find_package(lorcon)

if( ${LORCON_FOUND} )
	LIST(APPEND PLUGINS DeviceProxy_dot11)
	include_directories(${LORCON_INCLUDE_DIR})
	LIST(APPEND PLUGINS_LINK_LIBS ${LORCON_LIBRARIES})
else( ${LORCON_FOUND} )
	if( USE_LORCON AND NOT ${LORCON_FOUND} )
		message( FATAL_ERROR
			"Cannot find lorcon, which is required for USE_LORCON")
	endif()
endif( ${LORCON_FOUND} )

foreach(plugin ${PLUGINS})
	add_library(${plugin} SHARED ${plugin}.cpp)
	set_source_files_properties(${plugin}.cpp PROPERTIES COMPILE_FLAGS  "${CMAKE_CPP_FLAGS}")
	set_target_properties(${plugin} PROPERTIES PREFIX "")
	target_link_libraries(${plugin} ${PLUGINS_LINK_LIBS})
	install(TARGETS ${plugin}
        LIBRARY DESTINATION lib/USBProxy${LIB_SUFFIX}
        COMPONENT sharedlibs
        )
endforeach(plugin)
