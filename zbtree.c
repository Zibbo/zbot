#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <search.h>

#include "zbtree.h"

struct zbtree *zbtree_init(int (*compar) (const void *, const void *))
{
  struct zbtree *t = calloc(1,sizeof(struct zbtree));
  pthread_mutex_init(&t->mutex, NULL);
  t->tree = NULL;
  t->compar = compar;
  return t;
}

void *zbtree_find(struct zbtree *t, void *key, int lock)
{
  void **retval;
  if (lock)
    zbtree_lock(t);
  retval = tfind(key, &t->tree, t->compar); 
  if (lock)
    zbtree_unlock(t);
  if (retval != NULL)
    return *retval;
  else
    return NULL;
}

void *zbtree_search(struct zbtree *t, void *key, int lock)
{
  void **retval;
  if (lock)
    zbtree_lock(t);
  retval = tsearch(key, &t->tree, t->compar); 
  if (lock)
    zbtree_unlock(t);
  if (retval != NULL)
    return *retval;
  else
    return NULL;

}

void zbtree_delete(struct zbtree *t, void *key, int lock)
{
  if (lock)
    zbtree_lock(t);
  tdelete(key, &t->tree, t->compar);
  if (lock)
    zbtree_unlock(t);
}

void zbtree_destroy(struct zbtree *t)
{
}

void zbtree_lock(struct zbtree *t)
{
  pthread_mutex_lock( &t->mutex );
}

void zbtree_unlock(struct zbtree *t)
{
  pthread_mutex_unlock( &t->mutex );
}
