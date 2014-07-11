# Copyright 2013 Dominic Spill
# Copyright 2013 Adam Stasiak
#
# This file is part of USBProxy.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
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

CXXFLAGS=-std=gnu++0x -pedantic -felide-constructors -fno-exceptions -fno-rtti
CPPFLAGS=-Wall -g -fPIC -fdata-sections -ffunction-sections -MMD -MP -DNVALGRIND -I/usr/src -I/usr/include -I/usr/local/include -I../include -I../Filters -I../Injectors -I../Hosts
LDFLAGS=-Wl,--gc-sections -L../lib -lUSBProxy -ludev -lstdc++ -lpthread -lpcap -lrt -ldl

$(TARGET):
	$(CC) $(CXXFLAGS) $(CPPFLAGS) -o $@ ../Filters/*.o ../Injectors/*.o ../Hosts/*.o $(LDFLAGS) $@.cpp

clean:
	rm -f $(TARGET) $(TARGET).d

.PHONY : clean
