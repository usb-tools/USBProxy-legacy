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

//#ifdef DEBUG
#define dbgTrace() fprintf( stderr, "%s %s() %d\n", __FILE__, __func__, __LINE__)
#define dbgMessage(str) fprintf( stderr, "%s %s() %d %s\n", __FILE__, __func__, __LINE__, str)
// #define return if ( ( fprintf( stderr, "return@%s:%s()#%d\n", __FILE__, __func__, __LINE__), false)) {} else return
//#else
//#define dbgTrace()
//#define dbgMessage(str)
//#define return return
//#endif // DEBUG

#define TEST(expr) (((expr)) ? ( fprintf( stderr, "%s: %d %s: `%s' failed.\n", __FILE__, __LINE__, __func__, __STRING(expr)), false) : true)

#define assert2(expr) (((expr)) ? false : ( fprintf( stderr, "%s: %d %s: `%s' failed.\n", __FILE__, __LINE__, __func__, __STRING(expr)), abort(), true))

#endif // _MYDEBUG_H
