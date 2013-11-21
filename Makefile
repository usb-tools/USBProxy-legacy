# Copyright 2013 Dominic Spill
#
# This file is part of USB-MitM.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING. If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
TARGET=usb-mitm

PLATFORM = $(shell uname -m)
ifneq ($(PLATFORM), armv7l)
        CROSS_COMPILE ?= /opt/gcc-linaro-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
        CC = $(CROSS_COMPILE)gcc
        CXX = $(CROSS_COMPILE)g++
else
        CC = /usr/bin/gcc
        CXX = /usr/bin/g++
endif

# CPPFLAGS = compiler options for C and C++
CPPFLAGS = -Wall -g -Os -mthumb -fdata-sections -ffunction-sections -MMD -MP $(OPTIONS) -I/usr/src -I/usr/include -I/usr/local/include

# compiler options for C++ only
CXXFLAGS = -std=c++98 -pedantic -felide-constructors -fno-exceptions -fno-rtti

# compiler options for C only
CFLAGS =

OS = $(shell uname)
ifeq ($(OS), FreeBSD)
        LIBUSB = usb
        CFLAGS += -DFREEBSD
else
        LIBUSB = usb-1.0
endif

LDFLAGS += -Wl,--gc-sections
LDFLAGS += -l$(LIBUSB) -ludev -lstdc++ -lpthread -lusb-gadget -lboost_atomic

C_FILES := $(wildcard *.c)
CPP_FILES := $(wildcard *.cpp)
OBJS := $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o)
HEADERS := $(C_FILES:.c=.h) $(CPP_FILES:.cpp=.h)

all: $(TARGET)

-include $(OBJS:.o=.d)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -g -o $(TARGET) $(OBJS) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o *.d

.PHONY: all clean