#
# This file is part of USBProxy.
#

project(usbproxy_injectors C)
set(PACKAGE usbproxy_injectors)

LIST(APPEND PLUGINS Injector_UDP Injector_UDPHID)

include_directories(${libusbproxy_SOURCE_DIR})
LIST(APPEND PLUGINS_LINK_LIBS USBProxy)

foreach(plugin ${PLUGINS})
	add_library(${plugin} SHARED ${plugin}.cpp)
	set_source_files_properties(${plugin}.cpp PROPERTIES COMPILE_FLAGS  "-std=c++11 -pthread -Wall")
	set_target_properties(${plugin} PROPERTIES PREFIX "")
	target_link_libraries(${plugin} ${PLUGINS_LINK_LIBS})
	install(TARGETS ${plugin}
        LIBRARY DESTINATION lib/USBProxy${LIB_SUFFIX}
        COMPONENT sharedlibs
        )
endforeach(plugin)
