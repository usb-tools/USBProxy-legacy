# - Try to find the lorcon library
# Once done this defines
#
#  LORCON_FOUND - system has lorcon
#  LORCON_INCLUDE_DIR - the lorcon include directory
#  LORCON_LIBRARIES - Link these to use lorcon

# Copyright (c) 2014 Dominic Spill
# Copyright (c) 2013 Benjamin Vernoux
#

if (LORCON_INCLUDE_DIR AND LORCON_LIBRARIES)

  # in cache already
  set(LORCON_FOUND TRUE)

else (LORCON_INCLUDE_DIR AND LORCON_LIBRARIES)
  IF (NOT WIN32)
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    find_package(PkgConfig)
    pkg_check_modules(PC_LORCON QUIET lorcon)
  ENDIF(NOT WIN32)

  FIND_PATH(LORCON_INCLUDE_DIR
    NAMES lorcon2/lorcon.h
    HINTS $ENV{LORCON_DIR}/include ${PC_LORCON_INCLUDEDIR}
    PATHS /usr/local/include /usr/include
    /usr/include ${CMAKE_SOURCE_DIR}/../lib
    /opt/local/include/lorcon
    ${LORCON_INCLUDE_DIR}
  )

  FIND_LIBRARY(LORCON_LIBRARIES
    NAMES orcon2
    HINTS $ENV{LORCON_DIR}/lib ${PC_LORCON_LIBDIR}
    PATHS /usr/local/lib /usr/lib /opt/local/lib ${PC_LORCON_LIBDIR} ${PC_LORCON_LIBRARY_DIRS} ${CMAKE_SOURCE_DIR}/../lib
  )

  if(LORCON_INCLUDE_DIR)
    set(CMAKE_REQUIRED_INCLUDES ${LORCON_INCLUDE_DIR})
  endif()

  if(LORCON_LIBRARIES)
    set(CMAKE_REQUIRED_LIBRARIES ${LORCON_LIBRARIES})
  endif()

  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(LORCON DEFAULT_MSG LORCON_LIBRARIES LORCON_INCLUDE_DIR)

  MARK_AS_ADVANCED(LORCON_INCLUDE_DIR LORCON_LIBRARIES)

endif (LORCON_INCLUDE_DIR AND LORCON_LIBRARIES)