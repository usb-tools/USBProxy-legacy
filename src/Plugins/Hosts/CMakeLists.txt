#
# This file is part of USBProxy.
#

project(usbproxy_hosts C)
set(PACKAGE usbproxy_hosts)

LIST(APPEND PLUGINS HostProxy_TCP)

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

# HostProxy_GadgetFS is a special case because of the helpers
add_library(HostProxy_GadgetFS SHARED HostProxy_GadgetFS.cpp GadgetFS_helpers.c)
set_target_properties(HostProxy_GadgetFS PROPERTIES PREFIX "")
set_source_files_properties(HostProxy_GadgetFS.cpp PROPERTIES COMPILE_FLAGS  "-std=c++11 -pthread -Wall")
target_link_libraries(HostProxy_GadgetFS ${PLUGINS_LINK_LIBS})
install(TARGETS HostProxy_GadgetFS
    LIBRARY DESTINATION lib/USBProxy${LIB_SUFFIX}
    COMPONENT sharedlibs
    )
