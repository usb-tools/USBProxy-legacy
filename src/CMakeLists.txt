#
# This file is part of USBProxy.
#
#top level cmake project for USBProxy lib, plugins, and commandline tool

cmake_minimum_required(VERSION 2.8)
set(MAJOR_VERSION 0)
set(MINOR_VERSION 1)
project(usbproxy_all)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

set(INSTALL_DEFAULT_BINDIR "bin" CACHE STRING "Appended to CMAKE_INSTALL_PREFIX")

add_subdirectory(lib)
add_subdirectory(Plugins)
add_subdirectory(tools)
add_subdirectory(bindings)

# Comment the following out for releases.
set(CMAKE_C_FLAGS "$ENV{CFLAGS}")

set(CMAKE_CPP_FLAGS "${CMAKE_CPP_FLAGS} -std=gnu++0x -pedantic -felide-constructors -fno-exceptions -fno-rtti")

# Create uninstall target
configure_file(
	${PROJECT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
@ONLY)

add_custom_target(uninstall
	${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)
