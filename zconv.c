#include <sys/time.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <gsl/gsl_rng.h>

double ztime()
{
  struct timeval tv;
  gettimeofday( &tv, NULL );
  
  return (double)tv.tv_sec + 0.000001 * (double)tv.tv_usec;
}

double ztime_double()
{
  struct timeval tv;
  gettimeofday( &tv, NULL );
  
  return (double)tv.tv_sec + 0.000001 * (double)tv.tv_usec;
}

float ztime_float()
{
  struct timeval tv;
  gettimeofday( &tv, NULL );
  
  return (float)((double)tv.tv_sec + 0.000001 * (double)tv.tv_usec);
}

void zsleep_float(float seconds)
{
  struct timespec ts; 
  ts.tv_sec = (time_t)seconds;
  seconds = floorf(seconds);
  ts.tv_nsec = (long)seconds*1000000000.0;
  nanosleep(&ts, NULL);
}

void zsleep_minimal()
{
  struct timespec ts; 
  ts.tv_sec = 0;
  ts.tv_nsec = 1;
  nanosleep(&ts, NULL);
}


void zrandom_seed()
{
  srandom(time(NULL));
}

/* __thread int thread_id; */

/* void thread_test() */
/* { */
/*   thread_id = zrandom_r(); */
/*   zsleep_float(5.0); */
/*   printf("ti: %i\n", thread_id); */

/* } */
long zrandom_r()
{
  static int random_lock = 0;
  long ret;
  while (!__sync_bool_compare_and_swap(&random_lock, 0, 1))
    zsleep_minimal();
  ret = random();
  assert(__sync_bool_compare_and_swap(&random_lock, 1, 0));
  return ret;
}

double zrand()
{
  return (double)zrandom_r()/((double)RAND_MAX+1);
}


double zrand_gsl(gsl_rng *rng)
{
  return gsl_rng_uniform(rng);
}
//int alloc_count = 0;
//int free_count = 0;

void *zalloc_refcount(int size)
{
  void *a = calloc(1,size);
  *(int32_t*)a = 1;
  
  //printf("allocated %lli, size: %i, refcount 1\n", (int64_t)a, size);
  // __sync_fetch_and_add(&alloc_count,1);
  //alloc_count++;
  //if (!(alloc_count%1000))
  // printf("alloc %i free %i %i\n", alloc_count, free_count, alloc_count-free_count);
  return a;
}

void *zfree_refcount(void *item)
{
  int32_t *refcount = item;
  int32_t retval;
  //printf("zfree_refcount, item: %lli, refcount begin %i, ", (int64_t)item, *refcount);
  retval = __sync_sub_and_fetch(refcount, 1);
  //printf("refcount end after:%i, now:%i, ", retval, *refcount);
  assert(retval >= 0);
  if (retval == 0)
    {
      //printf("freeing\n");
      //  __sync_fetch_and_add(&free_count,1);
      //if (!(free_count%1000))
      //	printf("alloc %i free %i %i\n", alloc_count, free_count, alloc_count-free_count);
      //free_count++;
      free(item);
      return NULL;
    }
  //printf("not freeing\n");
  return item;
}

void zinc_refcount(void *item)
{
  int32_t *refcount = item;
  int32_t retval;
  //printf("zinc_refcount, item: %lli, refcount begin %i, ", (int64_t)item, *refcount);
  retval = __sync_add_and_fetch(refcount, 1);
  //printf("refcount end aftre:%i now:%i\n", retval, *refcount);
}

int *get_shuffled_array(int start, int len, int inc)
{
  int i, is, tmp_i;

  int *ret = malloc(sizeof(int)*len);
  is = 0;
  for (i = 0; i < len; i++)
    ret[i] = start+i*inc;
  
  for (i = 0; i < len; i++)
    {
      is = zrandom_r()%len;
      tmp_i = ret[is];
      ret[is] = ret[i];
      ret[i] = tmp_i;
    }
  return ret;
}






