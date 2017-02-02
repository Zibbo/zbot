#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//#include "defs.h"
#include "queue.h"


struct queue *queue_init()
{
  struct queue *q = calloc(1,sizeof(struct queue));
  pthread_mutex_init(&q->mutex, NULL);
  pthread_cond_init(&q->condition_var, NULL);
  /* q->mutex = PTHREAD_MUTEX_INITIALIZER; */
  /* q->condition_var = PTHREAD_COND_INITIALIZER; */
  q->first = NULL;
  q->last = NULL;
  q->items = 0;
  return q;
}

int queue_put(struct queue *q, void *data)
{
  struct queue_item *qi = malloc(sizeof(struct queue_item));
  int retval;

  qi->data = data;
  qi->next = NULL;
  queue_lock(q);

  /* while (q->write_lock) */
  /*   { */
  /*     queue_unlock(q); */
  /*     zsleep(0.1); */
  /*     queue_lock(q); */
  /*   } */

  if (q->items == 0)
    {
      q->first = qi;
      q->last = qi;
    }
  else
    {
      q->first->next = qi;
      q->first = qi;
    }
  q->items++;
  retval = q->items;
  pthread_cond_signal( &q->condition_var );
  pthread_mutex_unlock( &q->mutex );
  return retval;
}

void *queue_get(struct queue *q)
{
  struct queue_item *qi;
  void *retval;
  pthread_mutex_lock( &q->mutex );
  while(q->items == 0)
    {
      pthread_cond_wait( &q->condition_var, &q->mutex );
    }
  assert(q->items > 0);

  qi = q->last;
  retval = qi->data;
  q->last = qi->next;
  q->items--;
  if (q->items == 0)
    {
      q->first = NULL;
      assert(q->last == NULL);
    }
  free(qi);
  pthread_mutex_unlock( &q->mutex );
  return retval;
}

int queue_get_item_count(struct queue *q)
{
  int ret;
  pthread_mutex_lock( &q->mutex );
  ret = q->items;
  pthread_mutex_unlock( &q->mutex );
  return ret;
}

int queue_lock(struct queue *q)
{
  pthread_mutex_lock( &q->mutex );
  return q->items;
}

void queue_unlock(struct queue *q)
{
  pthread_mutex_unlock( &q->mutex );
}

int heap_left(int idx)
{
  return (idx+1)*2-1;
}
int heap_right(int idx)
{
  return (idx+1)*2;
}
int heap_parent(int idx)
{
  return (idx+1)/2-1;
}

void heapify(struct zp_queue *q, int idx)
{
  int l = heap_left(idx), r = heap_right(idx);
  int largest;
  struct zp_queue_item item;

  if (l < q->n_items && q->items[l].pri > q->items[idx].pri)
    largest = l;
  else
    largest = idx;
  if (r < q->n_items && q->items[r].pri > q->items[largest].pri)
    largest = r;
  if (largest != idx)
    {
      item = q->items[idx];
      q->items[idx] = q->items[largest];
      q->items[largest] = item;
      heapify(q, largest);
    }
}

struct zp_queue *zp_queue_init()
{
  struct zp_queue *q = calloc(1,sizeof(struct zp_queue));
  pthread_mutex_init(&q->mutex, NULL);
  pthread_cond_init(&q->condition_var, NULL);
  /* q->mutex = PTHREAD_MUTEX_INITIALIZER; */
  /* q->condition_var = PTHREAD_COND_INITIALIZER; */
  q->n_items = 0;
  q->size = 0;
  q->items = NULL;
  return q;
}

int zp_queue_get_item_count(struct zp_queue *q)
{
  int ret;
  pthread_mutex_lock( &q->mutex );
  ret = q->n_items;
  pthread_mutex_unlock( &q->mutex );
  return ret;
}

int zp_queue_put(struct zp_queue *q, void *item, int priority)
{
  const int inc_size = 1024;
  int i, retval;

  pthread_mutex_lock( &q->mutex );
  if (q->size < q->n_items+1)
    {
      q->size += inc_size;
      q->items = realloc(q->items, q->size*sizeof(struct zp_queue_item));
    }
  
  i = q->n_items;
  q->n_items++;
  retval = q->n_items;
  while (i > 0 && q->items[heap_parent(i)].pri < priority)
    {
      q->items[i] = q->items[heap_parent(i)];
      i = heap_parent(i);
    }
  q->items[i].pri = priority;
  q->items[i].data = item;

  pthread_cond_signal( &q->condition_var );
  pthread_mutex_unlock( &q->mutex );
  return retval;
}



void *zp_queue_pop_max(struct zp_queue *q)
{
  void *ret = NULL;
  pthread_mutex_lock( &q->mutex );
  while(q->n_items == 0)
    {
      pthread_cond_wait( &q->condition_var, &q->mutex );
    }

  ret = q->items[0].data;
  q->items[0] = q->items[q->n_items-1];
  q->n_items--;
  heapify(q, 0);
  pthread_mutex_unlock( &q->mutex );
  return ret;
}

int zp_queue_lock(struct zp_queue *q)
{
  pthread_mutex_lock( &q->mutex );
  return q->n_items;
}

void zp_queue_unlock(struct zp_queue *q)
{
  pthread_mutex_unlock( &q->mutex );
}


/* void queue_lock_write(struct queue *q) */
/* { */
/*   queue_lock(q); */
/*   q->write_lock = 1; */
/*   queue_unlock(q); */
/* } */

/* void queue_unlock_write(struct queue *q) */
/* { */
/*   queue_lock(q); */
/*   q->write_lock = 0; */
/*   queue_unlock(q); */
/* } */
