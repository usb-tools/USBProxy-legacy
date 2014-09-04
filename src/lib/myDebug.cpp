#include <stdio.h>
#include "myDebug.h"

void myDump( void *p, int len)
{
  int i;
  for ( i = 0; i < len; i++) {
    printf( "%02x ", *(char *)(p + i));
    if ( i % 16 == 15) putchar( '\n');
  }
  if ( i % 16 != 15) putchar( '\n');
  putchar( '\n');
}

