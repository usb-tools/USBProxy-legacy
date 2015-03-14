/*
 * This file is part of USBProxy.
 */

#include <unistd.h>
#include <memory.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/types.h>
#include "FDInfo.h"

 void showFDDetail( __s32 fd ) {
   char buf[256];

   __s32 fd_flags = fcntl( fd, F_GETFD );
   if ( fd_flags == -1 ) return;

   int fl_flags = fcntl( fd, F_GETFL );
   if ( fl_flags == -1 ) return;

   char path[256];
   snprintf( path, sizeof(path), "/proc/self/fd/%d", fd );

   memset( &buf[0], 0, 256 );
   ssize_t s = readlink( path, &buf[0], 256 );
   if ( s == -1 )
   {
        fprintf(stderr," (%s): not available\n",path);
        return;
   }
   fprintf(stderr,"%d (%s): ",fd,buf);

   if ( fd_flags & FD_CLOEXEC )  fprintf(stderr,"cloexec ");

   // file status
   if ( fl_flags & O_APPEND   )  fprintf(stderr,"append ");
   if ( fl_flags & O_NONBLOCK )  fprintf(stderr,"nonblock ");

   // acc mode
   if ( fl_flags & O_RDONLY   )  fprintf(stderr,"read-only ");
   if ( fl_flags & O_RDWR     )  fprintf(stderr,"read-write ");
   if ( fl_flags & O_WRONLY   )  fprintf(stderr,"write-only ");

   if ( fl_flags & O_DSYNC    )  fprintf(stderr,"dsync ");
   if ( fl_flags & O_RSYNC    )  fprintf(stderr,"rsync ");
   if ( fl_flags & O_SYNC     )  fprintf(stderr,"sync ");

   struct flock fl;
   fl.l_type = F_WRLCK;
   fl.l_whence = 0;
   fl.l_start = 0;
   fl.l_len = 0;
   fcntl( fd, F_GETLK, &fl );
   if ( fl.l_type != F_UNLCK )
   {
      if ( fl.l_type == F_WRLCK )
    	  fprintf(stderr,"write-locked");
      else
    	  fprintf(stderr,"read-locked");
      fprintf(stderr,"(pid:%d) ",fl.l_pid);
   }
   fprintf(stderr,"\n");
}

void showFDInfo() {
   int numHandles = getdtablesize();
   __s32 i;
   for ( i = 0; i < numHandles; i++ )
   {
      __s32 fd_flags = fcntl( i, F_GETFD );
      if ( fd_flags == -1 ) continue;
      showFDDetail(i);
   }
}


