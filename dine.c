#include <sys/time.h>
#include <semaphore.h>
#include <ctype.h>
#include "dine.h"

/*Philosophers grab the fork that articulates to their ID, as
 *well as their ID+1. Or, if ID+1 is outside the range of fork values, they 
 *attempt to grab forks[0]. The order in which they grab the forks depends on
 *whether their ID is even or odd.
 */

sem_t updatePass; /*Semaphore to allow only one person to update*/
sem_t forks[NUM_PHILOSOPHERS]; /*List of forks*/
static philoP philList[NUM_PHILOSOPHERS]; /*List of philosophers*/
static pthread_t philThreads[NUM_PHILOSOPHERS]; /*List of pthreads*/

/*Print the names of each philosopher*/
void printHeader()
{
  int i;
  
  for(i = 0; i < NUM_PHILOSOPHERS; i++)
  {
    /*Print the letter with half given space leading
     *Then print a blank string with half the given space leading
     */
    printf("|%*c%*s", 1+((8+NUM_PHILOSOPHERS)/2), 'A' + i,
	   (8+NUM_PHILOSOPHERS)/2, "");
  }

  /*Cap off the right most edge*/
  printf("|\n");
}

/*Print the equal sign spacer thing*/
void printSpacer()
{
  int i, j;

  for(i = 0; i < NUM_PHILOSOPHERS; i++)
  {
    /*Print leading pipe*/
    putchar('|');

    /*print 8+NUM_PHILOSOPHERS '=' characters*/
    for(j = 0; j < (8 + NUM_PHILOSOPHERS); j++)
    {
      putchar('=');
    }
  }
  
  /*Cap off the right most edge*/
  printf("|\n");
}

/*Statuses are formatted as follows:

  '| XXXXX ABCDE '

  *with a variable number of X's. This means that the width of each section is
  *9+NUM_PHILOSOPHERS. 
*/
void printStatus(philoP phil)
{
  int i;
  
  /*Print the leading characters*/
  printf("| ");

  /*I don't know where I would start to do this next part with printf*/
  for(i = 0; i < NUM_PHILOSOPHERS; i++)
  {
    /*If i is the first fork they can hold, and they are holding it*/
    if( (i == phil->first) && phil->held[0])
    {
      printf("%d", phil->first);
    }
    
    /*If i is the second fork they can hold, and they are holding it*/
    else if( (i == phil->second) && phil->held[1])
    {
      printf("%d", phil->second);
    }
    
    /*If the above isn't fulfilled, output a '-' character*/
    else
    {
      putchar('-');
    }
  }

  /*Print the status of the philosopher*/
  printf(" %-5s ", phil->status);
}

/*This function is called when:
 *   fork is picked up
 *   fork is dropped
 *   enter eat stage
 *   leave eat stage
 *   enter think stage
 *   leave think stage
 */
void update()
{
  /*Print out status of each philosopher, 
   *allow only one to do so with semaphores
   */

  int i;

  /*Try to open the update function*/
  if( sem_wait(&updatePass) )
  {
    perror("Waiting on updatePass");
    exit(EXIT_FAILURE);
  }
  
  for(i = 0; i < NUM_PHILOSOPHERS; i++)
  {
    printStatus(philList[i]);
  }

  /*Cap off the end of the status field*/
  printf("|\n");

  /*Unlock the update function*/
  if( sem_post(&updatePass) )
  {
    perror("Unlocking updatePass");
    exit(EXIT_FAILURE);
  }
}

void *think(void *philosopher)
{
  philoP phil = (philoP) philosopher;
  
  /*-----------Dropping first fork-----------*/
  /*Mark first fork as dropped*/
  phil->held[0] = 0;

  /*Drop one fork, then the other (depending on ID)*/
  if( sem_post(&(forks[phil->first])) )
  {
    perror("Posting first fork");
    exit(EXIT_FAILURE);
  }

  /*Update the user*/
  update();

  
  /*-----------Dropping second fork-----------*/

  /*Mark second fork as dropped*/
  phil->held[1] = 0;

  /*Attempt to drop the second fork*/
  if( sem_post(&(forks[phil->second])) )
  {
    perror("Posting second fork");
    exit(EXIT_FAILURE);
  }

  /*Update the user*/
  update();

  
  /*-----------Change philosopher status to thinking-----------*/  
  /*Change status to eating*/
  phil->status = statusThinking;

  /*Update the user*/
  update();

  /*Dawdle in the thinking area*/
  dawdle();

  
  /*-----------Change philosopher status to changing-----------*/
  /*Change status to eating*/
  phil->status = statusChanging;

  /*Update the user*/
  update();


  /*Decrement the loop count*/
  /*If the loop count is 0, we're done*/
  if( (--(phil->count)) == 0 )
  {
    /*Exit the pthread*/
    pthread_exit(NULL);
  }
  else
  {
    /*Return to eating*/
    eat(phil);
  }

  return NULL;
}

void *eat(void *philosopher)
{
  philoP phil = (philoP) philosopher;
  
  /*-----------Grabbing first fork-----------*/
  /*Grab one fork, then the other (depending on ID)*/
  if( sem_wait(&(forks[phil->first])) )
  {
    perror("Waiting on first fork");
    exit(EXIT_FAILURE);
  }

  /*Mark first fork as held*/
  phil->held[0] = 1;

  /*Update the user*/
  update();

  
  /*-----------Grabbing second fork-----------*/
  /*Attempt to grab the second fork*/
  if( sem_wait(&(forks[phil->second])) )
  {
    perror("Waiting on first fork");
    exit(EXIT_FAILURE);
  }

  /*Mark second fork as held*/
  phil->held[1] = 1;

  /*Update the user*/
  update();

  
  /*-----------Change philosopher status to eating-----------*/
  /*Change status to eating*/
  phil->status = statusEating;

  /*Update the user*/
  update();

  /*Dawdle in the eating area*/
  dawdle();

  
  /*-----------Change philosopher status to changing-----------*/
  /*Change status to eating*/
  phil->status = statusChanging;

  /*Update the user*/
  update();
  
  /*Attempt to think (me every day)*/
  think(phil);

  return NULL;
}

/*Optional to call, seeds random like Prof Nico's published solution*/
void seedRandom()
{
  /*Kinda a waste I feel like*/
  struct timeval *tv = malloc(sizeof(struct timeval));

  if( gettimeofday(tv, NULL) == -1 )
  {
    perror("Retrieving time of day");
    exit(EXIT_FAILURE);
  }

  /*Seed Random*/
  srandom(((int) tv->tv_sec) + ((int) tv->tv_usec));

  free(tv);
}

int main(int argc, char *argv[])
{
  int i;

  /*Default value for number of loops to do*/
  long int numLoops = 1;

  if(NUM_PHILOSOPHERS <= 1)
  {
    printf("Please use a number of philosophers > 1\n");
    exit(EXIT_FAILURE);
  }
  
  /*If multiple arguments were given*/
  if(argc > 1)
  {
    /*Convert string to int*/
    long int arg = strtol( argv[1], NULL, 10);
    
    /*If the user gave us an acceptable digit (i.e. greater than 0)*/
    if(arg > 0)
    {
      /*Set the loop counter*/
      numLoops = arg;
    }
    else if(arg <= 0)
    {
      printf("Please provide a positive number of loops");
      exit(EXIT_FAILURE);
    }
    /*If they didn't give us a digit*/
    else
    {
      /*Print usage*/
      printf("Usage: ./dine [numLoops]\n");
      /*Exit*/
      exit(EXIT_FAILURE);
    }
  }  

  /*Now that input checking is done, lets seed the random function*/
  seedRandom();
  
  /*Initialize the philosophizers*/
  for(i = 0; i < NUM_PHILOSOPHERS; i++)
  {
    /*While we're looping here, lets just initialize the forks as well*/
    if( sem_init(&forks[i], 0, 1) == -1 )
    {
      perror("Initializing fork");
      exit(EXIT_FAILURE);
    }
    
    /*Attempt to allocate memory for the philosopher*/
    if( !(philList[i] = malloc(sizeof(struct philo))) )
    {
      perror("allocating memory for philosopher");
      exit(EXIT_FAILURE);
    }

    philList[i]->ID = i;      /*Set ID*/
    philList[i]->count = numLoops; /*Set the number of loops to do*/
    philList[i]->status = statusChanging; /*Start out in the changing state*/
    philList[i]->held[0] = 0; /*They start out not holding forks*/
    philList[i]->held[1] = 0;
   
    /*If its even*/
    if( i % 2 == 0)
    {
      /*Set first fork as the current*/
      philList[i]->first = i;

      /*If i+1 is outside the range*/
      if(i + 1 >= NUM_PHILOSOPHERS)
      {
      	/*Loop back around to the first index*/
      	philList[i]->second = 0; 
      }
      else
      {
      	/*Otherwise set the second fork as the next*/
      	philList[i]->second = i + 1; 
      }
    }
    /*If its odd*/
    else
    {
      /*If i+1 is outside the range*/
      if(i + 1 >= NUM_PHILOSOPHERS)
      {
      	/*Loop back around to the first index*/
      	philList[i]->first = 0; 
      }
      else
      {
      	/*Otherwise set first fork as the next*/
      	philList[i]->first = i + 1;
      }

      /*Set second fork as the current*/
      philList[i]->second = i;
    }
  }

  /*Initialize updatePass*/
  if( sem_init(&updatePass, 0, 1) == -1 )
  {
    perror("Initialize updatePass");
    exit(EXIT_FAILURE);
  }

  /*Print starting spacer*/
  printSpacer();
  
  /*Print header with single char, starting with A*/
  printHeader();

  /*Print middle spacer*/
  printSpacer();
  
  /*Initialize the pthreads*/
  for(i = 0; i < NUM_PHILOSOPHERS; i++)
  {
    if( pthread_create(&philThreads[i], NULL, eat, philList[i]) )
    {
      perror("Creating pthread");
      exit(EXIT_FAILURE);
    }
  }

  /*Wait for threads to finish*/
  for(i = 0; i < NUM_PHILOSOPHERS; i++)
  {
    /*If in the future I want to return various exit statuses, replace NULL*/
    if( pthread_join(philThreads[i], NULL) )
    {
      perror("Joining pthread");
      exit(EXIT_FAILURE);
    }
  }

  /*Print end spacer*/
  printSpacer();

  /*Destroy all semaphores when all threads have finished*/
  if( sem_destroy(&updatePass) )
  {
    perror("Destroying updatePass");
    exit(EXIT_FAILURE);
  }

  /*Clean up*/
  for(i = 0; i < NUM_PHILOSOPHERS; i++)
  {
    /*Destroy semaphores*/
    if( sem_destroy(&forks[i]) )
    {  
      perror("Destroying fork");
      exit(EXIT_FAILURE);
    }

    /*Free the structures*/
    free(philList[i]);
  }
  
  return 0;
}
