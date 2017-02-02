#ifndef _ZBTREE_H_
#define _ZBTREE_H_

struct zbtree
{
  pthread_mutex_t mutex;
  void *tree;
  int (*compar) (const void *, const void *);
};

struct zbtree *zbtree_init(int (*compar) (const void *, const void *));
void *zbtree_find(struct zbtree *t, void *key, int lock);
void *zbtree_search(struct zbtree *t, void *key, int lock);
void zbtree_delete(struct zbtree *t, void *key, int lock);

void zbtree_destroy(struct zbtree *t);
void zbtree_lock(struct zbtree *t);
void zbtree_unlock(struct zbtree *t);

#endif
