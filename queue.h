#ifndef _QUEUE_H_
#define _QUEUE_H_

struct queue_item
{
  struct queue_item *next;
  void *data;
};

struct queue
{
  pthread_mutex_t mutex;
  pthread_cond_t  condition_var;
  struct queue_item *first;
  struct queue_item *last;
  int items;
};

struct zp_queue_item
{
  int pri;
  int padding;
  void *data;
};

struct zp_queue
{
  pthread_mutex_t mutex;
  pthread_cond_t  condition_var;
  int n_items;
  int size;
  struct zp_queue_item *items;
};

struct queue *queue_init();
int queue_put(struct queue *q, void *data);
void *queue_get(struct queue *q);
int queue_get_item_count(struct queue *q);
int queue_lock(struct queue *q);
void queue_unlock(struct queue *q);
struct zp_queue *zp_queue_init();
int zp_queue_get_item_count(struct zp_queue *q);
int zp_queue_put(struct zp_queue *q, void *item, int priority);
void *zp_queue_pop_max(struct zp_queue *q);
int zp_queue_lock(struct zp_queue *q);
void zp_queue_unlock(struct zp_queue *q);
#endif
