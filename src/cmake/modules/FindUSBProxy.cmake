# - Try to find the usbproxy library
# Once done this defines
#
#  USBPROXY_FOUND - system has usbproxy
#  USBPROXY_INCLUDE_DIR - the usbproxy include directory
#  USBPROXY_LIBRARIES - Link these to use usbproxy

# Copyright (c) 2014 Dominic Spill
# Copyright (c) 2013 Benjamin Vernoux
#


if (USBPROXY_INCLUDE_DIR AND USBPROXY_LIBRARIES)

  # in cache already
  set(USBPROXY_FOUND TRUE)

else (USBPROXY_INCLUDE_DIR AND USBPROXY_LIBRARIES)
  IF (NOT WIN32)
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    find_package(PkgConfig)
    pkg_check_modules(PC_USBPROXY QUIET usbproxy)
  ENDIF(NOT WIN32)

  FIND_PATH(USBPROXY_INCLUDE_DIR
    NAMES DeviceProxy.h
    HINTS $ENV{USBPROXY_DIR}/include ${PC_USBPROXY_INCLUDEDIR}
    PATHS /usr/local/include /usr/include
    /usr/include ${CMAKE_SOURCE_DIR}/../lib
    /opt/local/include/usbproxy
    ${USBPROXY_INCLUDE_DIR}
  )

  set(usbproxy_library_names USBProxy)

  FIND_LIBRARY(USBPROXY_LIBRARIES
    NAMES ${usbproxy_library_names}
    HINTS $ENV{USBPROXY_DIR}/lib ${PC_USBPROXY_LIBDIR}
    PATHS /usr/local/lib /usr/lib /opt/local/lib ${PC_USBPROXY_LIBDIR} ${PC_USBPROXY_LIBRARY_DIRS} ${CMAKE_SOURCE_DIR}/../lib
  )

  if(USBPROXY_INCLUDE_DIR)
    set(CMAKE_REQUIRED_INCLUDES ${USBPROXY_INCLUDE_DIR})
  endif()

  if(USBPROXY_LIBRARIES)
    set(CMAKE_REQUIRED_LIBRARIES ${USBPROXY_LIBRARIES})
  endif()

  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(USBPROXY DEFAULT_MSG USBPROXY_LIBRARIES USBPROXY_INCLUDE_DIR)

  MARK_AS_ADVANCED(USBPROXY_INCLUDE_DIR USBPROXY_LIBRARIES)

endif (USBPROXY_INCLUDE_DIR AND USBPROXY_LIBRARIES)