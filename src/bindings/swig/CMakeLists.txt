#
# This file is part of USBProxy.
#

project(usbproxy_bindings C)
set(PACKAGE usbproxy_swig_bindings)

# Check that we have swig and python
find_package(SWIG)
find_package(PythonLibs)
if(NOT SWIG_FOUND OR NOT PYTHONLIBS_FOUND)
    return()
endif()

INCLUDE(${SWIG_USE_FILE})
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${libusbproxy_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${usbproxy_filters_SOURCE_DIR})
INCLUDE_DIRECTORIES(${usbproxy_devices_SOURCE_DIR})

SET(CMAKE_SWIG_FLAGS "")

SET_SOURCE_FILES_PROPERTIES(usbproxy.i PROPERTIES CPLUSPLUS ON)
SWIG_ADD_MODULE(usbproxy python usbproxy.i usbproxyPYTHON_wrap.cxx API.cpp)
SWIG_LINK_LIBRARIES(usbproxy
    USBProxy
    PacketFilter_Callback
    DeviceProxy_Callback
    ${PYTHON_LIBRARIES}
)


# swig -c++ -python -I../lib -o usbproxy_wrap.cxx usbproxy.i
# g++ -fpic -shared -std=c++0x usbproxy_wrap.cxx -I/usr/include/python2.7 -I../lib -lpython2.7 -lUSBProxy -o _usbproxy.so
