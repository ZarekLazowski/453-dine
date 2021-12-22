#ifndef DINEH
#define DINEH

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>


#ifndef NUM_PHILOSOPHERS
#define NUM_PHILOSOPHERS 5
#endif

#define statusEating "Eat"
#define statusThinking "Think"
#define statusChanging ""

/*Structure for a philosopher*/
typedef struct philo
{
  int ID;      /*Their ID: IDs start at A and increment for each philo*/
  long int count;   /*Amount of times to do the eat/think loop*/
  int held[2]; /*Array of forks currently held*/
  int first;   /*Index of first fork to grab/drop*/
  int second;  /*Index of second fork to grab/drop*/
  char *status;/*Current status of the philosopher*/
} *philoP;


/*Philosopher function prototypes*/
void *eat(void *philosopher);

void *think(void *philosopher);

void update();

/*Given dawdle function*/
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

#endif
