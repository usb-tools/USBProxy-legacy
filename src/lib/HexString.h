/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_HEXSTRING_H
#define USBPROXY_HEXSTRING_H

#define TABPADDING "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
char* hex_string_wide(const void* buf,int length,int width=32);
char* hex_string(const void* buf,int length);

#endif /* USBPROXY_HEXSTRING_H */
