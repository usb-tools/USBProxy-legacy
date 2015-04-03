#
# This file is part of USBProxy.
#

set(TOOL usb-mitm)

include_directories(${libusbproxy_SOURCE_DIR})
LIST(APPEND TOOLS_LINK_LIBS USBProxy)

add_executable(${TOOL} ${TOOL}.cpp)
set_source_files_properties(${TOOL}.cpp PROPERTIES COMPILE_FLAGS  "-std=c++11 -pthread -Wall -I.")
target_link_libraries(${TOOL} ${TOOLS_LINK_LIBS})
install(TARGETS ${TOOL} RUNTIME DESTINATION ${INSTALL_DEFAULT_BINDIR})

add_custom_target(version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/git-version-gen.sh
        ${CMAKE_CURRENT_BINARY_DIR}/version.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../version
        ${CMAKE_CURRENT_SOURCE_DIR}/../.. )
ADD_DEPENDENCIES(${TOOL} version.h)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/version.h PROPERTIES GENERATED 1)
SET_SOURCE_FILES_PROPERTIES(${TOOL}.cpp PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/version.h)
