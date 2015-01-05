#
# This file is part of USBProxy.
#

set(TOOL usb-mitm)

include_directories(${libusbproxy_SOURCE_DIR})
LIST(APPEND TOOLS_LINK_LIBS USBProxy)

add_executable(${TOOL} ${TOOL}.cpp)
set_source_files_properties(${TOOL}.cpp PROPERTIES COMPILE_FLAGS  "-std=c++11")
target_link_libraries(${TOOL} ${TOOLS_LINK_LIBS})
install(TARGETS ${TOOL} RUNTIME DESTINATION ${INSTALL_DEFAULT_BINDIR})
