#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void dawdle() {
  /*
   * sleep for a random amount of time between 0 and 999
   * milliseconds.  This routine is somewhat unreliable, since it
   * doesn't take into account the possiblity that the nanosleep
   * could be interrupted for some legitimate reason.
   */
  struct timespec tv;
  int msec = (int)((((double)random()) / RAND_MAX) * 1000);

  tv.tv_sec  = 0;
  tv.tv_nsec = 1000000 * msec;
  if ( -1 == nanosleep(&tv,NULL) ) {
    perror("nanosleep");
  }
}

