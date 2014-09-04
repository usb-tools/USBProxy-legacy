#ifndef _MYDEBUG_H
#define _MYDEBUG_H 1
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void myDump( void*, int);

#ifdef __cplusplus
}
#endif

#define dbgMessage(str) fprintf( stderr, "%s %s() %d %s\n", __FILE__, __func__, __LINE__, str)

#define IS_FAILED(expr) (((expr)) ? false : ( fprintf( stderr, "%s: %d %s: `%s' failed.\n", __FILE__, __LINE__, __func__, __STRING(expr)), true))

#endif // _MYDEBUG_H
