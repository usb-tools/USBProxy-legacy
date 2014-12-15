#
# This file is part of USBProxy
#

# swig -c++ -python -I../lib usbproxy.i
# g++ -fpic -shared --std=c++11 usbproxy_wrap.cxx -I/usr/include/python2.7 -I../lib -lpython2.7 -lUSBProxy -o _usbproxy.so


########################################################################
# Include swig generation macros
########################################################################
find_package(SWIG)
find_package(PythonLibs)
if(NOT SWIG_FOUND OR NOT PYTHONLIBS_FOUND)
    return()
endif()

########################################################################
# Setup swig generation
########################################################################
foreach(incdir ${GNURADIO_RUNTIME_INCLUDE_DIRS})
    list(APPEND GR_SWIG_INCLUDE_DIRS ${incdir}/gnuradio/swig)
endforeach(incdir)

set(GR_SWIG_LIBRARIES gnuradio-bluetooth btbb)

GR_SWIG_MAKE(usbproxy usbproxy.i)

########################################################################
# Install the built swig module
########################################################################
GR_SWIG_INSTALL(TARGETS usbproxy DESTINATION usbproxy)

########################################################################
# Install swig .i files for development
########################################################################
install(
    FILES
    usbproxy.i 
    DESTINATION ${GR_INCLUDE_DIR}/usbproxy/swig
)
