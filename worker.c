#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <search.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <fenv.h>

#include "defs.h"
#include "queue.h"
#include "zbtree.h"
#include "types.h"
#include "worker.h"
#include "plrmodel.h"
#include "handval.h"
#include "zconv.h"
#include "bit.h"

extern CardMask int_to_cardmask_3[22100];
extern CardMask int_to_cardmask_2[1326];

extern struct cards_3 int_to_cards_3[22100];
extern struct cards_2 int_to_cards_2[1326];
extern struct cards_1 int_to_cards_1[52];

extern int preflop_morph_mapping[1326];

int us_tree_cmp(const void *p1, const void *p2)
{
  return ((struct unique_root*)p1)->root_idx - ((struct unique_root*)p2)->root_idx;
}

int td_tree_cmp(const void *p1, const void *p2)
{
  return ((struct types_data *)p1)->id - ((struct types_data *)p2)->id;
}

int wm_tree_cmp(const void *p1, const void *p2)
{
  struct worker_message *w1 = (struct worker_message*)p1, *w2 = (struct worker_message*)p2;
  int retval = w1->td->id - w2->td->id;
  if (retval != 0)
    return retval;
  retval = w1->v[0]->path_i - w2->v[0]->path_i;
  if (retval != 0)
    return retval;
  return memcmp(w1->v[0]->path,w2->v[0]->path, w1->v[0]->path_i);
}

int msg_pri(struct worker_message *msg)
{
  return -msg->types_data_id;
  /* if (msg->direction == 0) */
  /*   return msg->v[0]->path_i; */
  /* else */
  /*   return 1000-msg->v[0]->path_i; */
}

void set_all_us_to_tree(struct unique_root *us, struct zbtree *t)
{
  int i;
  //printf("trying to add %i to tree\n", us->root_idx);
  if (zbtree_find(t, us, false) != NULL)
    return;

  (void)zbtree_search(t, us, false);  
  //printf("added %i to tree\n", us->root_idx);
  for (i = 0; i < ACTS;i++)
    if (us->next[i] != NULL)
      set_all_us_to_tree(us->next[i], t);
}

/* void set_handler_addr(struct unique_root *us, char *addr, uint16_t port) */
/* { */
/*   if (us->handler_addr != NULL) */
/*     { */
/*       free(us->handler_addr); */
/*       us->handler_addr = NULL; */
/*     } */
  
/*   us->handler_addr = calloc(1,sizeof(struct sockaddr_in)); */

/*   us->handler_addr->sin_family = AF_INET; */
/*   us->handler_addr->sin_port = htons(port); */
/*   inet_pton(AF_INET, addr, &us->handler_addr->sin_addr); */
  
/* } */

	  
struct zbtree *init_us_tree(struct unique_root *us)
{
  struct zbtree *t = zbtree_init(us_tree_cmp);
  set_all_us_to_tree(us, t);
  return t;
}

/* struct zbtree *init_td_tree() */
/* { */
/*   return zbtree_init(td_tree_cmp); */
/* } */
/* struct zbtree *init_wm_tree() */
/* { */
/*   return zbtree_init(wm_tree_cmp); */
/* } */

void setup_trees_and_queues(struct thread_control_args *args)
{
  args->receiver_to_worker_queue = zp_queue_init();
  args->worker_to_sender_queue = zp_queue_init();
  //args->in_queue = queue_init();
  
  args->us_tree = init_us_tree(args->root_us);
  args->td_tree = zbtree_init(td_tree_cmp);
  args->wm_tree = zbtree_init(wm_tree_cmp);
}


/* void setup_thread_args(struct thread_control_args *args) */
/* { */
/*   args->w_args->in_queue = args->receiver_to_worker_queue; */
/*   args->w_args->out_queue = args->worker_to_sender_queue; */
/*   args->w_args->us = args->root_us; */
/*   args->w_args->us_tree = args->us_tree; */
/*   args->w_args->wm_tree = args->wm_tree; */
/*   args->w_args->td_tree = args->td_tree; */
/*   args->w_args->cont = 1; */

/*   args->r_args->out_queue = args->receiver_to_worker_queue; */
/*   args->r_args->td_tree = args->td_tree; */
/*   /\* IP args already set *\/ */
/*   args->r_args->cont = 1; */

/*   args->s_args->us_tree = args->us_tree; */
/*   args->s_args->in_queue = args->worker_to_sender_queue; */
/*   args->s_args->loopback_queue = args->receiver_to_worker_queue; */
/*   args->s_args->cont = 1; */
/* } */

/* void free_resources(struct thread_control_args *args) */
/* { */
/* } */

/* void start_threads(struct thread_control_args *args) */
/* { */
/*   //  int r_sched_policy; */
/*   int res, i; */

  
/*   args->worker_thread_id = calloc(1, sizeof(pthread_t)*args->n_worker_thread); */

/*   /\* res = pthread_create( &args->sender_thread_id, NULL, sender_thread, (void*) args->s_args); *\/ */
/*   /\* res = pthread_create( &args->sender_thread_id, NULL, sender_thread, (void*) args->s_args); *\/ */
/*   /\* res = pthread_create( &args->sender_thread_id, NULL, sender_thread, (void*) args->s_args); *\/ */
/*   /\* res = pthread_create( &args->sender_thread_id, NULL, sender_thread, (void*) args->s_args); *\/ */

/*   res = pthread_create( &args->receiver_thread_id, NULL, receiver_thread, (void*) args->r_args); */
  
/*   for (i = 0; i < args->n_worker_thread; i++) */
/*     { */
/*       res = pthread_create( &args->worker_thread_id[i], NULL, worker_thread, (void*) args->w_args); */
/*     } */
/* } */

/* /\* void *thread_control(void *thread_args) *\/ */
/* /\* { *\/ */
/* /\*   int32_t *cmd; *\/ */
/* /\*   struct thread_control_args *args = thread_args; *\/ */

/* /\*   for(;;) *\/ */
/* /\*     { *\/ */
/* /\*       cmd = queue_get(args->in_queue); *\/ */
/* /\*       switch(*cmd) *\/ */
/* /\* 	{ *\/ */
/* /\* 	case 1: *\/ */
/* /\* 	  setup_trees_and_queues(args); *\/ */
/* /\* 	  start_threads(args); *\/ */
/* /\* 	  break; *\/ */
/* /\* 	case 2: *\/ */
/* /\* 	  free_resources(args); *\/ */
/* /\* 	  stop_threads(args); *\/ */
/* /\* 	  break; *\/ */
/* /\* 	default: *\/ */
/* /\* 	  break; *\/ */
/* /\* 	} *\/ */
/* /\*     } *\/ */
/* /\*   return NULL; *\/ */
/* /\* } *\/ */

/* void clean_old_types_data(struct thread_control_args *args, double timelimit) */
/* { */
/*   struct types_data *td; */
/*   double cutof_time = ztime_double() - timelimit; */
/*   int clean_count = 0; */

/*   zbtree_lock(args->td_tree); */
/*   while(args->td_tree->tree != NULL)  */
/*     { */
/*       td = *(struct types_data **)args->td_tree->tree; */
/*       if (td->timestamp < cutof_time) */
/* 	{ */
/* 	  //printf("deleting td %f %f\n", td->timestamp, cutof_time); */
/* 	  zbtree_delete(args->td_tree, td, false); */
/* 	  free(td); */
/* 	  clean_count ++; */
/* 	} */
/*       else */
/* 	break; */
/*     } */
/*   zbtree_unlock(args->td_tree); */
/*   if (clean_count) */
/*     printf("Cleaned %i types_data\n", clean_count); */
/* } */
      
  
/* /\* void recv_new_types_data(struct zbtree *td_tree, int fd) *\/ */
/* /\* { *\/ */
/* /\*   struct types_data *ret_td, *td = malloc(sizeof(struct types_data)); *\/ */
/* /\*   int len; *\/ */

/* /\*   len = recv(fd, td, sizeof(struct types_data), MSG_WAITALL); *\/ */
/* /\*   td->timestamp = ztime_float(); *\/ */
/* /\*   assert(len == sizeof(struct types_data)); *\/ */

/* /\*   ret_td = zbtree_search(td_tree, td, true); *\/ */
/* /\*   if (ret_td != td) *\/ */
/* /\*     { *\/ */
/* /\*       ret_td->timestamp = td->timestamp; *\/ */
/* /\*       free(td); *\/ */
/* /\*     } *\/ */
/* /\* } *\/ */


/* void *sender_child_thread(void *thread_args) */
/* { */
/*   struct worker_message *wm; */
/*   char send_buf[65000]; */
/*   int i, sb_i, s = -1, sret; */
/*   struct sockaddr_in local_addr; */
/*   struct unique_root *us = thread_args; */

/*   local_addr.sin_family = AF_INET; */
/*   local_addr.sin_port = htons(0); */
/*   inet_pton(AF_INET, "", &local_addr.sin_addr); */


/*   for(;;) */
/*     { */
/*       wm = zp_queue_pop_max(us->in_queue); */
/*       assert(wm->types_data_id == wm->td->id); */
/*       if (wm == NULL) */
/* 	{ */
/* 	  printf("NULL wm, shutting down\n"); */
/* 	  close(s); */
/* 	  s = -1; */
/* 	  pthread_exit(EXIT_SUCCESS); */
/* 	} */
/*       wm->td = NULL; */
/*       sb_i = 0;       */
/*       ((int*)send_buf)[0] = 2; */
/*       sb_i += sizeof(int); */
/*       memcpy(&send_buf[sb_i], wm, sizeof(struct worker_message)); */
/*       sb_i += sizeof(struct worker_message); */
/*       for (i = 0; i < MAX_HWEV; i++) */
/* 	{ */
/* 	  if (wm->hwev[i] == NULL) */
/* 	    continue; */
/* 	  memcpy(&send_buf[sb_i], wm->hwev[i], sizeof(struct hwev_data)); */
/* 	  sb_i += sizeof(struct hwev_data); */
/* 	} */
/*       memcpy(&send_buf[sb_i], wm->v[0], sizeof(struct variation)); */
/*       sb_i += sizeof(struct variation); */
/*       sret = 0; */
/*       //      printf("sending len: %i\n", sb_i); */
/*       while(socket <= 0 || (sret = send(s, send_buf, sb_i, 0)) <= 0) */
/* 	{ */
/* 	  if (sret == -1) */
/* 	    { */
/* 	      perror("Send failed"); */
/* 	      close(s); */
/* 	      s = -1; */
/* 	    } */
/* 	  if (s > 0) */
/* 	    { */
/* 	      close(s); */
/* 	      s = -1; */
/* 	    } */
/* 	  s = socket(AF_INET, SOCK_STREAM, 0); */
/* 	  if (s == -1) */
/* 	    { */
/* 	      perror("Socket create fail"); */
/* 	    } */
/* 	  else if (bind(s, (struct sockaddr*)&local_addr, sizeof(local_addr)) == -1) */
/* 	    { */
/* 	      perror("Socket bind fail"); */
/* 	      close(s); */
/* 	      s = -1; */
/* 	    } */
/* 	  else if(connect(s, (struct sockaddr*)us->handler_addr, sizeof(struct sockaddr_in)) == -1) */
/* 	    { */
/* 	      perror("Connect fail"); */
/* 	      close(s); */
/* 	      s = -1; */
/* 	    } */
/* 	}	   */
/*       for(i = 0; i < MAX_HWEV; i++) */
/* 	if (wm->hwev[i] != NULL) */
/* 	  zfree_refcount(wm->hwev[i]); */
/*       free(wm->v[0]); */
/*       free(wm);      	   */

/*     } */
/* } */


/* void *worker_thread(void *thread_args) */
/* { */
/*   struct worker_message *msg, *stored_msg, *hax_msg, *int_msg; */
/*   struct unique_root *us; */
/*   int all_hwev_received, i, hwev_i; */
/*   struct worker_thread_args *args = thread_args; */
/*   struct types_data dummy_td; */


/*   gsl_rng * rng = gsl_rng_alloc (gsl_rng_taus2); */
/*   gsl_rng_set(rng, (unsigned long int)(ztime()*1000000)); */
  
/*   //thread_test(); */
/*   hax_msg = NULL; */
/*   while(args->cont) */
/*     { */
/*       if (hax_msg != NULL) */
/* 	msg = hax_msg; */
/*       else */
/* 	msg = zp_queue_pop_max(args->in_queue); */
/*       hax_msg = NULL; */
/*       if (msg->td == NULL) */
/* 	{ */
/* 	  dummy_td.id = msg->types_data_id; */
/* 	  msg->td = zbtree_find(args->td_tree, &dummy_td, true); */
/* 	  //assert(msg->types_data_id == msg->td->id); */
/* 	} */
/*       if (msg->td == NULL) */
/* 	{ */
/* 	  zp_queue_put(args->in_queue, msg, msg_pri(msg)); */
/* 	  printf("types data %i, not loaded yet\n", msg->types_data_id); */
/* 	  continue; */
/* 	} */

/*       assert(msg->types_data_id == msg->td->id); */
  
/*       /\* if (td != msg->td) // jos kyseinen types data on jo muistissa, käytetään sitä *\/ */
/*       /\* 	{ *\/ */
/*       /\* 	  free(msg->td); *\/ */
/*       /\* 	  msg->td = td; *\/ */
/*       /\* 	} *\/ */
/*       if (msg->direction == 0) //up */
/* 	{ */
/* 	  update_msg_up(msg, args->us, rng); */
/* 	  //zp_queue_put(args->out_queue, msg, msg_pri(msg)); */
/* 	} */
/*       else */
/* 	{ */
/* 	  zbtree_lock(args->wm_tree); */
/* 	  stored_msg = zbtree_search(args->wm_tree, msg, false); */
/* 	  if (stored_msg != msg) */
/* 	    { */
/* 	      assert(stored_msg->hwev[msg->action] == NULL); */
/* 	      assert(msg->hwev[msg->pov_seat] != NULL); */
/* 	      stored_msg->hwev[msg->action] = msg->hwev[msg->pov_seat]; */
/* 	      stored_msg->v[0]->path_weight += msg->v[0]->path_weight; */
/* 	      /\* if (msg->positive_regs < stored_msg->positive_regs)  *\/ */
/* 	      /\* 	stored_msg->positive_regs = msg->positive_regs; *\/ */
/* 	      stored_msg->positive_regs += msg->positive_regs; */
/* 	      //stored_msg->n_hwev++; */
/* 	      free(msg->v[0]); */
/* 	      free(msg); */
/* 	    } */
/* 	  else */
/* 	    { */
/* 	      stored_msg->hwev[msg->action] = msg->hwev[msg->pov_seat]; */
/* 	      if (msg->action != msg->pov_seat) */
/* 		stored_msg->hwev[msg->pov_seat] = NULL; */
/* 	      //stored_msg->n_hwev = 1; */
/* 	    } */
/* 	  us = zbtree_find(args->us_tree, &stored_msg->v[0]->us_id, true);/\* pitäisi toimia pelkalla int osotteella *\/ */
/* 	  assert(us != NULL); */

/* 	  all_hwev_received = 1; */
/* 	  for (i = 0; i < ACTS;i++) */
/* 	    if (us->next[i] != NULL && stored_msg->hwev[i] == NULL) */
/* 	      { */
/* 		all_hwev_received = 0; */
/* 		break; */
/* 	      } */
/* 	  if (all_hwev_received) */
/* 	    { */
/* 	      zbtree_delete(args->wm_tree, stored_msg, false); */
/* 	      zbtree_unlock(args->wm_tree); */

/* 	      msg = update_msg_down(stored_msg, args->us, rng); */
/* 	      assert(msg->types_data_id == msg->td->id); */
/* 	      //zp_queue_put(args->out_queue, stored_msg, msg_pri(stored_msg)); */
/* 	    } */
/* 	  else */
/* 	    { */
/* 	      msg = NULL; */
/* 	      zbtree_unlock(args->wm_tree); */
/* 	    } */
/* 	} */
      
/*       //work_msg = msg; */
/*       if (msg != NULL) */
/* 	{ */
/* 	  for (i = 0; i < msg->n_variations;i++) */
/* 	    { */
/* 	      assert(msg->types_data_id == msg->td->id); */
/* 	      us = zbtree_find(args->us_tree, &msg->v[i]->us_id, true); */
/* 	      assert(us != NULL); */
	      
/* 	      int_msg = malloc(sizeof(struct worker_message)); */
/* 	      memcpy(int_msg, msg, sizeof(struct worker_message)); */
/* 	      for (hwev_i = 0; hwev_i < MAX_HWEV;hwev_i++) */
/* 		if (int_msg->hwev[hwev_i] != NULL) */
/* 		  zinc_refcount(int_msg->hwev[hwev_i]); */
/* 	      int_msg->n_variations = 1; */
/* 	      int_msg->v[0] = msg->v[i]; */
/* 	      if (us->in_queue == NULL) */
/* 		{ */
/* 		  if (us->handler_addr == NULL) */
/* 		    us->in_queue = args->in_queue; */
/* 		  else */
/* 		    { */
/* 		      us->in_queue = zp_queue_init(); */
/* 		      if (pthread_create(&us->handler_thread, NULL, sender_child_thread, (void*)us) == -1) */
/* 			{ */
/* 			  perror("sender child create"); */
/* 			  exit(EXIT_FAILURE); */
/* 			} */
		      
/* 		    }  */
/* 		} */
/* 	      if (i == 0 && us->in_queue == args->in_queue) */
/* 		hax_msg = int_msg; */
/* 	      else */
/* 		zp_queue_put(us->in_queue, int_msg, msg_pri(int_msg)); */
/* 	    } */
	
/* 	  for(i = 0; i < MAX_HWEV; i++) */
/* 	    if (msg->hwev[i] != NULL) */
/* 	      zfree_refcount(msg->hwev[i]); */
/* 	  free(msg);      	   */
/* 	  msg = NULL; */
/* 	} */
      
/*     } */
/*   return NULL; */
/* } */



/* void *sender_thread(void *thread_args) */
/* { */
/*   struct sockaddr_in local_addr; */
/*   struct worker_message *work_msg, *int_msg; */
/*   //  struct net_message *net_msg; */
/*   char *send_buf = NULL; */
/*   struct unique_root dummy_us, *us; */
/*   int i,hwev_i; */

/*   struct sender_thread_args *args = thread_args; */

/*   //  msg_size = sizeof(struct worker_message) + sizeof(struct types_data) + sizeof(struct variation); */

/*   local_addr.sin_family = AF_INET; */
/*   local_addr.sin_port = htons(0); */
/*   inet_pton(AF_INET, "", &local_addr.sin_addr); */

/*   /\* s = socket(AF_INET, SOCK_DGRAM, 0); *\/ */
/*   /\* if (s == -1) *\/ */
/*   /\*   { *\/ */
/*   /\*     perror("Socket create fail"); *\/ */
/*   /\*     exit(EXIT_FAILURE); *\/ */
/*   /\*   } *\/ */
/*   /\* if (bind(s, (struct sockaddr*)&local_addr, sizeof(local_addr)) == -1) *\/ */
/*   /\*   { *\/ */
/*   /\*     perror("Socket bind fail"); *\/ */
/*   /\*     exit(EXIT_FAILURE); *\/ */
/*   /\*   } *\/ */

/*   send_buf = malloc(65000); */
/*   //net_msg = (struct net_message *)send_buf; */

/*   while(args->cont) */
/*     { */
/*       work_msg = zp_queue_pop_max(args->in_queue); */
/*       //printf("SENDER received msg: dir %i, variations:%i\n", work_msg->direction, work_msg->n_variations); */

/*       for (i = 0; i < work_msg->n_variations;i++) */
/* 	{ */
/* 	  assert(work_msg->types_data_id == work_msg->td->id); */
/* 	  dummy_us.root_idx = work_msg->v[i]->us_id; */
/* 	  us = zbtree_find(args->us_tree, &dummy_us, true); */
/* 	  assert(us != NULL); */

/* 	  int_msg = malloc(sizeof(struct worker_message)); */
/* 	  memcpy(int_msg, work_msg, sizeof(struct worker_message)); */
/* 	  for (hwev_i = 0; hwev_i < MAX_HWEV;hwev_i++) */
/* 	    if (int_msg->hwev[hwev_i] != NULL) */
/* 	      zinc_refcount(int_msg->hwev[hwev_i]); */
/* 	  int_msg->n_variations = 1; */
/* 	  int_msg->v[0] = work_msg->v[i]; */
/* 	  if (us->in_queue == NULL) */
/* 	    { */
/* 	      if (us->handler_addr == NULL) */
/* 		us->in_queue = args->loopback_queue; */
/* 	      else */
/* 		{ */
/* 		  us->in_queue = zp_queue_init(); */
/* 		  if (pthread_create(&us->handler_thread, NULL, sender_child_thread, (void*)us) == -1) */
/* 		    { */
/* 		      perror("sender child create"); */
/* 		      exit(EXIT_FAILURE); */
/* 		    } */

/* 		}  */
/* 	    } */
/* 	  zp_queue_put(us->in_queue, int_msg, msg_pri(int_msg)); */
/* 	} */
/*       for(i = 0; i < MAX_HWEV; i++) */
/* 	if (work_msg->hwev[i] != NULL) */
/* 	  zfree_refcount(work_msg->hwev[i]); */
/*       free(work_msg);      	   */
/*     } */
/*   return NULL; */
/* } */

/* void *recv_child_thread(void *thread_args) */
/* { */
/*   int ret, i, msg_id, last_queue_size = 0, new_queue_size = 0, queue_increasing = 0, bc; */
/*   struct recv_child_thread_args *args = thread_args; */
/*   struct worker_message *wm; */
/*   struct types_data *ret_td, *td; */
/*   struct types_data dummy_td; */
	      
/*   while(args->cont) */
/*     { */
/*       /\* if (new_queue_size > last_queue_size) *\/ */
/*       /\* 	queue_increasing = 1; *\/ */
/*       /\* else *\/ */
/*       /\* 	queue_increasing = 0; *\/ */
/*       /\* last_queue_size = new_queue_size; *\/ */


/*       /\* while (last_queue_size > 2000 || (last_queue_size > 1000 && queue_increasing)) *\/ */
/*       /\* 	{ *\/ */
/*       /\* 	  zsleep_float(0.01); *\/ */
/*       /\* 	  new_queue_size = queue_get_item_count(args->out_queue); *\/ */
/*       /\* 	  if (new_queue_size > last_queue_size) *\/ */
/*       /\* 	    queue_increasing = 1; *\/ */
/*       /\* 	  else *\/ */
/*       /\* 	    queue_increasing = 0; *\/ */
/*       /\* 	  last_queue_size = new_queue_size; *\/ */
/*       /\* 	} *\/ */


/*       ret = recv(args->s, &msg_id, sizeof(msg_id), MSG_WAITALL); */
/*       //printf("RECV msg_id ret: %i %i\n", ret, msg_id);  */
/*       if (ret == 4) */
/* 	{ */
/* 	  switch(msg_id) */
/* 	    { */
/* 	    case 1: */
/* 	      //printf("recv types data\n"); */
/* 	      td = malloc(sizeof(struct types_data)); */
	      
/* 	      ret = recv(args->s, td, sizeof(struct types_data), MSG_WAITALL); */
/* 	      if (ret > 0) */
/* 		{ */
/* 		  td->timestamp = ztime_double(); */
/* 		  assert(ret == sizeof(struct types_data)); */
		  
/* 		  ret_td = zbtree_search(args->td_tree, td, true); */
/* 		  if (ret_td != td) */
/* 		    { */
/* 		      ret_td->timestamp = td->timestamp; */
/* 		      free(td); */
/* 		      td = NULL; */
/* 		    } */
/* 		} */
/* 	      break; */
/* 	    case 2: */
/* 	      //printf("recv wm\n"); */
/* 	      wm = malloc(sizeof(struct worker_message)); */
/* 	      bc = 0; */
/* 	      ret = recv(args->s, wm, sizeof(struct worker_message), MSG_WAITALL); */
/* 	      if (ret > 0) */
/* 		{ */
/* 		  bc += ret; */
/* 		  for (i = 0; i < MAX_HWEV && ret > 0; i++) */
/* 		    { */
/* 		      if (wm->hwev[i] != NULL) */
/* 			{ */
/* 			  wm->hwev[i] = zalloc_refcount(sizeof(struct hwev_data)); */
/* 			  ret = recv(args->s, wm->hwev[i], sizeof(struct hwev_data), MSG_WAITALL); */
/* 			  bc += ret; */
/* 			  wm->hwev[i]->ref_count = 1; */
/* 			} */
/* 		    } */
/* 		  for (i = 0; i < wm->n_variations && ret > 0; i++) */
/* 		    { */
/* 		      wm->v[i] = malloc(sizeof(struct variation)); */
/* 		      ret = recv(args->s, wm->v[i], sizeof(struct variation), MSG_WAITALL); */
/* 		      bc+= ret; */
/* 		    } */
/* 		  if (ret_td != NULL && wm->types_data_id == ret_td->id) */
/* 		    wm->td = ret_td; */
/* 		  else */
/* 		    wm->td = NULL; */
		  
/* 		  new_queue_size = zp_queue_put(args->out_queue, wm, msg_pri(wm)); */
/* 		  if (new_queue_size > last_queue_size) */
/* 		    queue_increasing = 1; */
/* 		  else */
/* 		    queue_increasing = 0; */
/* 		  last_queue_size = new_queue_size; */
/* 		  //printf("received %i\n", bc); */
/* 		} */
/* 	      else */
/* 		free(wm); */
/* 	      break; */
/* 	    case 3: */
/* 	      recv(args->s, &dummy_td.id, sizeof(dummy_td.id), MSG_WAITALL); */
/* 	      td = zbtree_find(args->td_tree, &dummy_td, true); */
/* 	      zbtree_delete(args->td_tree, td, true); */
/* 	      free(td); */
/* 	      //printf("freeing td %i \n", dummy_td.id); */
/* 	    default: */
/* 	      break; */
/* 	    } */
/* 	} */
/*       if (ret == -1) */
/* 	{ */
/* 	  perror("RECV failed"); */
/* 	  close(args->s); */
/* 	  pthread_exit(NULL); */
/* 	} */
/*       else if (ret == 0) */
/* 	{ */
/* 	  assert(ret == 0); */
/* 	  printf("Client closed connetion\n"); */
/* 	  close(args->s); */
/* 	  pthread_exit(NULL); */
/* 	} */
/*     } */
/*   return NULL; */
/* } */
  

/* void *receiver_thread(void *thread_args) */
/* { */
  
/*   struct sockaddr_in listen_addr; */
  
/*   struct recv_child_thread_args *new_client; */
/*   struct receiver_thread_args *args = thread_args; */
  
/*   socklen_t addrlen; */
/*   int  msg_size, s; */
  
/*   msg_size = sizeof(struct worker_message) + sizeof(struct types_data) + sizeof(struct variation); */
/*   memset(&listen_addr, 0, sizeof(struct sockaddr_in)); */
/*   listen_addr.sin_family = AF_INET; */
/*   listen_addr.sin_port = htons(args->listen_port); */
/*   //inet_pton(AF_INET, INADDR_ANY, &listen_addr.sin_addr); */
/*   inet_pton(AF_INET, "", &listen_addr.sin_addr); */

/*   printf("binding to %i port\n", args->listen_port); */

/*   s = socket(AF_INET, SOCK_STREAM, 0); */
/*   if (s == -1) */
/*     { */
/*       perror("Socket create fail"); */
/*       exit(EXIT_FAILURE); */
/*     } */
/*   if (bind(s, (struct sockaddr*)&listen_addr, sizeof(listen_addr)) == -1) */
/*     { */
/*       perror("Socket bind fail"); */
/*       exit(EXIT_FAILURE); */
/*     } */

/*   addrlen = sizeof(struct sockaddr_in); */
/*   getsockname(s, (struct sockaddr*)&listen_addr, &addrlen); */
/*   args->listen_port = ntohs(listen_addr.sin_port); */
  
/*   listen(s, 64); */
  
/*   while (args->cont) */
/*     { */

/*       new_client = calloc(1,sizeof(struct recv_child_thread_args)); */
/*       new_client->addrlen = sizeof(struct sockaddr_in); */
/*       new_client->cont = 1; */
/*       new_client->out_queue = args->out_queue; */
/*       new_client->td_tree = args->td_tree; */
/*       new_client->s = accept(s, (struct sockaddr*)&new_client->addr, &new_client->addrlen); */
      
      
/*       if (new_client->s == -1) */
/* 	{ */
/* 	  perror("RECEIVER: accept failed"); */
/* 	  free(new_client); */
/* 	} */
/*       else */
/* 	{ */
/* 	  int ret; */
/* 	  ret = pthread_create( &new_client->thread_id, NULL, recv_child_thread, (void*) new_client); */
/* 	  assert(ret == 0); */
/* 	  pthread_detach(new_client->thread_id); */
/* 	} */
/*     } */
/*   return NULL; */
/* } */




void generate_mapping_from_global_to_local(struct plrmodel_node *n)
{
  n->type_mapping = malloc(sizeof(int16_t)*n->t->n_types);
  generate_mapping_to_slots(n->types_bmap, n->type_mapping, n->t, n->len);
}

/* void generate_alt_mapping_from_global_to_local(struct plrmodel_node *n) */
/* { */
/*   n->type_mapping_alt = malloc(sizeof(int16_t)*n->t->n_types); */
/*   generate_alt_mapping_to_slots(n->types_bmap, n->type_mapping_alt, n->t); */
/* } */


int generate_next_variations_up(struct unique_root *us, struct variation *old_v, struct variation *var_list[MAX_VARIATION], int var_i, int path_i, int pov_seat, float potsize, double path_weight)
{
  struct variation *new_v;
  int act_i, n_acts;

  if (us->cur_seat == pov_seat && us->gamestate != GS_S)
    {
      n_acts = 0;
      for (act_i = 0; act_i < ACTS-1; act_i++)
	if (us->next[act_i] != NULL)
	  n_acts++;
      for (act_i = 0; act_i < ACTS-1; act_i++)
	{
	  if (us->next[act_i] != NULL)
	    {
	      old_v->path[path_i] = act_i;
	      var_i = generate_next_variations_up(us->next[act_i], old_v, var_list, var_i, path_i+1, pov_seat, potsize+us->action_cost[act_i], path_weight/n_acts);
	    }
	}
    }
  else
    {
      new_v = malloc(sizeof(struct variation));
      memcpy(new_v->path, old_v->path, path_i*sizeof(char));
      new_v->path_i = path_i;
      new_v->us_id = us->root_idx;
      new_v->potsize = potsize;
      new_v->path_weight = path_weight;
      var_list[var_i] = new_v;
      var_i += 1;
    }
  return var_i;
}
      
void generate_next_variation_down(struct unique_root *us, struct worker_message *msg)
{
  int i;
  char *path = msg->v[0]->path;
  struct unique_root *last_us = NULL;
  int last_action = -1;
  int last_i = -1;
  float potsize = 0.0;

  msg->n_variations = 1;
  for (i = 0; i < msg->v[0]->path_i;i++) /*muuten path_i-1, mutta ekassa loopissa ei olla edetty actioneja yhtaan*/
    {
      if (us->cur_seat == msg->pov_seat)// && us->root_idx >= 2)
	{
	  last_us = us;
	  last_action = path[i];
	  last_i = i;
	}
      potsize += us->action_cost[(int)path[i]];
      us = us->next[(int)path[i]];
    }

  if (last_us != NULL)
    {
      msg->action = last_action;
      msg->v[0]->us_id = last_us->root_idx;
      msg->v[0]->path_i = last_i;
      msg->v[0]->potsize = potsize;
    }
  else
    {
      msg->action = -1;
      memset(msg->v[0], 0, sizeof(struct variation));
    }
  //return msg;
}


float **get_odds_from_double(double **src, int n_acts, int n_items, gsl_rng *rng)
{
  float **odds;
  float tot;
  int i,j, act_i;

  odds = malloc(sizeof(float*)*n_acts);
  for (i = 0; i < n_acts;i++)
      odds[i] = malloc(sizeof(float)*n_items);
  for (i = 0; i < n_items; i++)
    {
      tot = 0;
      for (j = 0; j < n_acts;j++)
	{
	  odds[j][i] = src[j][i] > 0?src[j][i]:0.0;
	  tot += odds[j][i];
	}
      if (tot != 0)
	for (j = 0; j < n_acts;j++)
	  odds[j][i] /= tot;
      else
	{
	  double hi = -10000000000,lo = 100000000000;
	  for (act_i = 0; act_i < ACTS; act_i++)
	    {
	      if (src[act_i][i] > hi)
		hi = src[act_i][i];
	      if (src[act_i][i] < lo)
		lo = src[act_i][i];
	    }
	  if (hi != lo)
	    {
	      tot = 0;
	      for (act_i = 0; act_i < ACTS; act_i++)
		{
		  if (src[act_i][i] == hi)
		    {
		      odds[act_i][i] = 1;
		      tot += 1;
		    }
		  else
		    {
		      odds[act_i][i] = 0;
		    }
		}
	    }
	  else
	    {
	      tot = 0;
	      for (act_i = 0; act_i < ACTS; act_i++)
		{
		  odds[act_i][i] = zrand_gsl(rng);
		  tot += odds[act_i][i];
		}
	      /* if (n->root->next[0] == NULL) */
	      /* 	{ */
	      /* 	  tot -= odds[0][i]; */
	      /* 	  odds[0][i] = 0; */
	      /* 	} */
	      /* if (n->root->action_cost[1] == 0) */
	      /* 	{ */
	      /* 	  tot -= odds[2][i]; */
	      /* 	  odds[2][i] = 0; */
	      /* 	} */
	    }
	  for (act_i = 0; act_i < ACTS; act_i++)
	    {
	      odds[act_i][i] /= tot;
	    }


	  
	  /* /\* odds[0][i] = 0.5; *\/ */
	  /* /\* odds[1][i] = 0.5; *\/ */
	  /* /\* odds[2][i] = 0; *\/ */

	  /* tot = 0; */
	  /* //odds[0][i] = zrand(); */
	  /* for (j = 0; j < n_acts;j++) */
	  /*   { */
	  /*     odds[j][i] = zrand(); */
	  /*     tot += odds[j][i]; */
	  /*   } */
	  /* //odds[j][i] = 0; */
	  /* for (j = 0; j < n_acts;j++) */
	  /*   { */
	  /*     odds[j][i] /= tot; */
	  /*   } */
	}
    }
  return odds;
}



float **get_odds_from_float(float **src, int n_acts, int n_items)
{
  float **odds;
  float tot;
  int i,j;

  odds = malloc(sizeof(float*)*n_acts);
  for (i = 0; i < n_acts;i++)
      odds[i] = malloc(sizeof(float)*n_items);
  for (i = 0; i < n_items; i++)
    {
      tot = 0;
      for (j = 0; j < n_acts;j++)
	{
	  odds[j][i] = src[j][i] > 0?src[j][i]:0.0;
	  tot += odds[j][i];
	}
      if (tot != 0)
	for (j = 0; j < n_acts;j++)
	  odds[j][i] /= tot;
      else
	{
	  tot = 0;
	  odds[0][i] = 0;
	  odds[1][i] = 1;
	  odds[2][i] = 0;
	  /* odds[0][i] = zrand(); */
	  /* for (j = 0; j < n_acts-1;j++) */
	  /*   { */
	  /*     odds[j][i] = zrand()*(1.0-tot); */
	  /*     tot += odds[j][i]; */
	  /*   } */
	  /* odds[j][i] = 1.0-tot; */
	}
    }
  return odds;
}

void set_odds_from_regs(struct plrmodel_node *n, int slot_i, double *odds, gsl_rng *rng)
{
  struct situ *s = (struct situ*) n->next_list;
  double tot;
  int act_i;

  tot = 0;
  for (act_i = 0; act_i < ACTS; act_i++)
    {
      odds[act_i] = s->regs[act_i][slot_i] > 0?s->regs[act_i][slot_i]:0;
      tot += odds[act_i];
    }
  if (likely(tot > 0))
    {
      for (act_i = 0; act_i < ACTS; act_i++)
	{
	  odds[act_i] /= tot;
	}
    }
  else
    {
      double hi = -10000000000,lo = 100000000000;
      for (act_i = 0; act_i < ACTS; act_i++)
	{
	  if (s->regs[act_i][slot_i] > hi)
	    hi = s->regs[act_i][slot_i];
	  if (s->regs[act_i][slot_i] < lo)
	    lo = s->regs[act_i][slot_i];
	}
      if (hi != lo)
	{
	  tot = 0;
	  for (act_i = 0; act_i < ACTS; act_i++)
	    {
	      if (s->regs[act_i][slot_i] == hi)
		{
		  odds[act_i] = 1;
		  tot += 1;
		}
	      else
		{
		  odds[act_i] = 0;
		}
	    }
	}
      else
	{
	  tot = 0;
	  for (act_i = 0; act_i < ACTS; act_i++)
	    {
	      odds[act_i] = zrand_gsl(rng);
	      tot += odds[act_i];
	    }
	  if (n->root->next[0] == NULL)
	    {
	      tot -= odds[0];
	      odds[0] = 0;
	    }
	  if (n->root->action_cost[1] == 0)
	    {
	      tot -= odds[2];
	      odds[2] = 0;
	    }
	}
      for (act_i = 0; act_i < ACTS; act_i++)
	{
	  odds[act_i] /= tot;
	}
    
    }
}

void set_odds_from_avg(struct plrmodel_node *n, int slot_i, double *odds, gsl_rng *rng)
{
  struct situ *s = (struct situ*) n->next_list;
  double tot;
  int act_i;

  tot = 0;
  for (act_i = 0; act_i < ACTS; act_i++)
    {
      odds[act_i] = s->avg_odds[act_i][slot_i];
      tot += odds[act_i];
    }
  if (likely(tot > 0))
    {
      for (act_i = 0; act_i < ACTS; act_i++)
	{
	  odds[act_i] /= tot;
	}
    }
  else
    {
      tot = 0;
      for (act_i = 0; act_i < ACTS; act_i++)
	{
	  odds[act_i] = zrand_gsl(rng);
	  tot += odds[act_i];
	}
      if (n->root->next[0] == NULL)
	{
	  tot -= odds[0];
	  odds[0] = 0;
	}
      for (act_i = 0; act_i < ACTS; act_i++)
	{
	  odds[act_i] /= tot;
	}
    }
}

/* void set_odds_from_ev(struct plrmodel_node *n, int slot_i, double *odds) */
/* { */
/*   struct situ *s = (struct situ*) n->next_list; */

/*   int act_i; */
/*   memset(odds, 0, sizeof(odds[0])*ACTS); */
/*   if (n->root->next[0] != NULL && s->ev[0][slot_i] > s->ev[1][slot_i]) */
/*     act_i = 0; */
/*   else */
/*     act_i = 1; */
/*   if (s->ev[act_i][slot_i] < 0) */
/*     act_i = 2; */
/*   odds[act_i] = 1; */
/* } */


/* float **get_odds_from_ev(double **src, int n_items) */
/* { */
/*   float **odds; */
/*   float tot; */
/*   int i, act_i; */

/*   odds = malloc(sizeof(float*)*ACTS); */
/*   for (i = 0; i < ACTS;i++) */
/*     odds[i] = calloc(1, sizeof(float)*n_items); */
/*   for (i = 0; i < n_items; i++) */
/*     { */
/*       tot = 0; */
/*       if (src[0][i] > src[1][i]) */
/* 	act_i = 0; */
/*       else */
/* 	act_i = 1; */
/*       if (src[act_i][i] < 0) */
/* 	act_i = 2; */
/*       odds[act_i][i] = 1; */
/*     } */
/*   return odds; */
/* } */


void update_msg_with_ev(struct worker_message *msg, struct unique_root *us)
{
  int i;
  assert(msg->hwev[msg->pov_seat] == NULL);
  msg->hwev[msg->pov_seat] = zalloc_refcount(sizeof(struct hwev_data));
  get_victory_odds_against_spread_float(msg->hwev, msg->td->vals, msg->pov_seat);
  //get_victory_odds_against_spread_float_no_dead_cards(msg->hwev, msg->td->vals, msg->pov_seat);
  msg->hwev[msg->pov_seat]->pw = 0;
  for (i = 0; i < SAMPLES; i++)
    {
      if (msg->hwev[msg->pov_seat]->d[i] < -100000.0)
	continue;
      msg->hwev[msg->pov_seat]->d[i] *= msg->v[0]->potsize;
      msg->hwev[msg->pov_seat]->pw += msg->hwev[msg->pov_seat]->d[i];
    }
  msg->hwev[msg->pov_seat]->pw /= (double)SAMPLES;

  for (i = 0; i < MAX_HWEV; i++)
    {
      if (msg->hwev[i] != NULL && msg->pov_seat != i)
	{
	  zfree_refcount(msg->hwev[i]);
	  msg->hwev[i] = NULL;
	}
	    
    }
  generate_next_variation_down(us, msg);
  msg->direction = 1;
}

void update_msg_folded(struct worker_message *msg, struct unique_root *us)
{
  int i;
  msg->hwev[msg->pov_seat] = zalloc_refcount(sizeof(struct hwev_data));

  for (i = 0; i < SAMPLES; i++)
    {
      msg->hwev[msg->pov_seat]->d[i] = msg->v[0]->potsize;
    }

  msg->hwev[msg->pov_seat]->pw = msg->v[0]->potsize;

  for (i = 0; i < MAX_HWEV; i++)
    {
      if (msg->hwev[i] != NULL && msg->pov_seat != i)
	{
	  zfree_refcount(msg->hwev[i]);
	  msg->hwev[i] = NULL;
	}	    
    }
  msg->direction = 1;
  msg->v[0]->path[msg->v[0]->path_i] = 2;
  msg->v[0]->path_i++;
  generate_next_variation_down(us, msg);
}


void lock_plrmodel_node_read(uint32_t *lock)
{
  uint32_t new_lock;
  while(1)
    {
      new_lock = __sync_add_and_fetch(lock, 1);
      if (new_lock < MAX_READERS)
	return;
      
      __sync_sub_and_fetch(lock, 1);
      
      zsleep_minimal();  
      while (*lock >= MAX_READERS)
	zsleep_minimal();
    }
}

void unlock_plrmodel_node_read(uint32_t *lock)
{
  __sync_sub_and_fetch(lock, 1);
}

void lock_plrmodel_node_write(uint32_t *lock)
{
  uint32_t new_lock;
  while(1)
    {
      new_lock = __sync_add_and_fetch(lock, MAX_READERS);
      if (new_lock == MAX_READERS)
	return;
      else if (new_lock >= 2*MAX_READERS) //multiple writers
	{
	  __sync_sub_and_fetch(lock, MAX_READERS);
	  while (*lock >= MAX_READERS)
	    zsleep_minimal();
	}
      else
	break;
    }

  while (*lock != MAX_READERS)
    zsleep_minimal();
}

void unlock_plrmodel_node_write(uint32_t *lock)
{
  __sync_sub_and_fetch(lock, MAX_READERS);
}
 

/* void lock_plrmodel_node(struct plrmodel_node *n) */
/* { */
/*   while (!__sync_bool_compare_and_swap(&n->lock, 0, 1)) */
/*     zsleep_minimal(); */
/* } */

/* void unlock_plrmodel_node(struct plrmodel_node *n) */
/* { */
/*   assert(__sync_bool_compare_and_swap(&n->lock, 1, 0)); */
/* } */


double update_avg_odds_from_regs(struct plrmodel_node *n)
{
  struct situ *s;
  int i, act_i;
  double tot;
  double tmp_odds[ACTS];
  double old_odds[ACTS];
  double odds_diff = 0;
  
  s = (struct situ *)n->next_list;
  memset(old_odds, 0, sizeof(old_odds));
  for (i = 0; i < n->len;i++)
    {
      
      tot = 0;
      for (act_i = 0; act_i < ACTS; act_i++)
	tot+=s->avg_odds[act_i][i];
      if (tot > 0)
	for (act_i = 0; act_i < ACTS; act_i++)
	  old_odds[act_i] = s->avg_odds[act_i][i]/tot;
      
      tot = 0;
      for (act_i = 0; act_i < ACTS; act_i++)
	{
	  tmp_odds[act_i] = s->regs[act_i][i] > 0?s->regs[act_i][i]:0.0;
	  tot+=tmp_odds[act_i];
	}
      
      if (tot != 0)
	for (act_i = 0; act_i < ACTS; act_i++)
	  s->avg_odds[act_i][i] += tmp_odds[act_i]/tot;
      else
	s->avg_odds[1][i] += 1.0;
      tot = 0;
      for (act_i = 0; act_i < ACTS; act_i++)
	tot+=s->avg_odds[act_i][i];
      if (tot > 0)
	for (act_i = 0; act_i < ACTS; act_i++)
	  odds_diff += fabs(s->avg_odds[act_i][i]/tot - old_odds[act_i]);
    }
  return odds_diff;
}
	  


void vector_add(double *dest, double *src, int len)
{
  int i;
  for (i = 0; i < len;i++)
    {
      dest[i] += src[i];
    }
}

void vector_mul(double *dest, double *src1, double *src2, int len)
{
  int i;
  for (i = 0; i < len;i++)
    {
      dest[i] = src1[i] * src2[i];
    }
}

void vector_mul_scalar(double *dest, double *src1, double src2, int len)
{
  int i;
  for (i = 0; i < len;i++)
    {
      dest[i] = src1[i] * src2;
    }
}


void vector_mul_add(double *dest, double *src1, double *src2, int len)
{
  int i;
  for (i = 0; i < len;i++)
    {
      dest[i] += src1[i] * src2[i];
    }
}

void *get_rng()
{
  gsl_rng * rng = gsl_rng_alloc (gsl_rng_taus2);
  gsl_rng_set(rng, (unsigned long int)(ztime()*1000000));
  return (void*)rng;
}

/* void set_feexc() */
/* { */
/*   feenableexcept(FE_INVALID | FE_OVERFLOW | FE_DIVBYZERO); */
/* } */

double vector_sum(double *src, int len)
{
  double sum = 0;
  int i;
  for (i = 0; i < len;i++)
    {
      sum += src[i];
    }
  return sum;
}
  

void get_odds_from_regs(struct plrmodel_node *n, double odds[ACTS][SAMPLES], int16_t local_mapping[SAMPLES], gsl_rng *rng)
{
  double tot;
  int sample_i, act_i;
  struct situ *s;
  int16_t local_type;

  s = (struct situ*)n->next_list;

  for (sample_i = 0; sample_i < SAMPLES; sample_i++)
    {
      local_type = local_mapping[sample_i];      
      tot = 0;

      if (n->root->next[0] == NULL)
	{
	  assert(s->regs[0][local_type] <= 0);
	  odds[0][sample_i] = 0;
	}
      else
	{
	  odds[0][sample_i] = s->regs[0][local_type] > 0?s->regs[0][local_type]:0.0;
	  tot += odds[0][sample_i];
	}
      odds[1][sample_i] = s->regs[1][local_type] > 0?s->regs[1][local_type]:0.0;
      tot += odds[1][sample_i];
      
      if (n->root->action_cost[1] == 0)
	{
	  odds[2][sample_i] = 0;
	}
      else
	{
	  odds[2][sample_i] = s->regs[2][local_type] > 0?s->regs[2][local_type]:0.0;
	  tot += odds[2][sample_i];
	}

      /* for (act_i = 0; act_i < ACTS;act_i++) */
      /* 	{ */
      /* 	  odds[act_i][sample_i] = s->regs[act_i][local_type] > 0?s->regs[act_i][local_type]:0.0; */
      /* 	  tot += odds[act_i][sample_i]; */
      /* 	} */
      /* if (n->root->next[0] == NULL) */
      /* 	{ */
      /* 	  tot -= odds[0][sample_i]; */
      /* 	  odds[0][sample_i] = 0; */
      /* 	} */
      /* if (n->root->action_cost[1] == 0) */
      /* 	{ */
      /* 	  tot -= odds[2][sample_i]; */
      /* 	  odds[2][sample_i] = 0; */
      /* 	} */
	
      if (tot == 0)
	{
	  for (act_i = 0; act_i < ACTS; act_i++)
	    {
	      odds[act_i][sample_i] = zrand_gsl(rng);
	      tot += odds[act_i][sample_i];
	    }
	
	  if (n->root->next[0] == NULL)
	    {
	      tot -= odds[0][sample_i];
	      odds[0][sample_i] = 0;
	    }
	  if (n->root->action_cost[1] == 0)
	    {
	      tot -= odds[2][sample_i];
	      odds[2][sample_i] = 0;
	    }
	}
      for (act_i = 0; act_i < ACTS; act_i++)
	{
	  odds[act_i][sample_i] /= tot;
	}
    }
}

void get_odds_from_avg(struct plrmodel_node *n, double odds[ACTS][SAMPLES], int16_t local_mapping[SAMPLES], gsl_rng *rng)
{
  double tot;
  int sample_i, act_i;
  struct situ *s;
  int16_t local_type;

  s = (struct situ*)n->next_list;
 

  for (sample_i = 0; sample_i < SAMPLES; sample_i++)
    {
      local_type = local_mapping[sample_i];      
      tot = 0;

      if (n->root->next[0] == NULL)
	{
	  assert(s->avg_odds[0][local_type] <= 0);
	  odds[0][sample_i] = 0;
	}
      else
	{
	  odds[0][sample_i] = s->avg_odds[0][local_type];
	  tot += odds[0][sample_i];
	}
      odds[1][sample_i] = s->avg_odds[1][local_type];
      tot += odds[1][sample_i];
      
      if (n->root->action_cost[1] == 0)
	{
	  odds[1][sample_i] += s->avg_odds[2][local_type];
	  tot += s->avg_odds[2][local_type];
	  odds[2][sample_i] = 0;
	}
      else
	{
	  odds[2][sample_i] = s->avg_odds[2][local_type];
	  tot += odds[2][sample_i];
	}

      /* for (act_i = 0; act_i < ACTS;act_i++) */
      /* 	{ */
      /* 	  odds[act_i][sample_i] = s->avg_odds[act_i][local_type] > 0?s->avg_odds[act_i][local_type]:0.0; */
      /* 	  tot += odds[act_i][sample_i]; */
      /* 	} */
      /* if (n->root->next[0] == NULL) */
      /* 	{ */
      /* 	  tot -= odds[0][sample_i]; */
      /* 	  odds[0][sample_i] = 0; */
      /* 	} */
      /* if (n->root->action_cost[1] == 0) */
      /* 	{ */
      /* 	  tot -= odds[2][sample_i]; */
      /* 	  odds[2][sample_i] = 0; */
      /* 	} */
      //tot = 0;
      if (tot == 0)
	{
	  for (act_i = 0; act_i < ACTS; act_i++)
	    {
	      odds[act_i][sample_i] = zrand_gsl(rng);
	      tot += odds[act_i][sample_i];
	    }
	
	  if (n->root->next[0] == NULL)
	    {
	      tot -= odds[0][sample_i];
	      odds[0][sample_i] = 0;
	    }
	  if (n->root->action_cost[1] == 0)
	    {
	      tot -= odds[2][sample_i];
	      odds[2][sample_i] = 0;
	    }
	}
      for (act_i = 0; act_i < ACTS; act_i++)
	{
	  odds[act_i][sample_i] /= tot;
	}
      /* odds[0][sample_i] = 0;  */
      /* odds[1][sample_i] = 1;  */
      /* odds[2][sample_i] = 0;  */
    }
}

void get_odds_from_byte_odds(struct plrmodel_node *n, double odds[ACTS][SAMPLES], int16_t local_mapping[SAMPLES], gsl_rng *rng)
{
  int sample_i;
  struct situ *s;
  int16_t local_type;

  s = (struct situ*)n->next_list;
 
  memset(odds, 0, sizeof(double)*ACTS*SAMPLES);

  for (sample_i = 0; sample_i < SAMPLES; sample_i++)
    {
      local_type = local_mapping[sample_i];      
      odds[0][sample_i] = ((double)s->byte_odds[local_type*2])/255.0;
      odds[1][sample_i] = ((double)s->byte_odds[local_type*2+1])/255.0;
      odds[2][sample_i] = 1.0-odds[0][sample_i]-odds[1][sample_i];
    }
}

double *get_fold_ev(double *hw, double *ev, struct hand_hv2 *hv, double potsize, double stake, int win)
{
  int i;
  double cardcount[52];
  double tot_ev = 0;
  memset(cardcount, 0, sizeof(cardcount));
  //ev = malloc(sizeof(double)*SAMPLES);
  
  for (i = 0; i < SAMPLES; i++)
    {
      tot_ev += hw[hv[i].sample_i];
      cardcount[hv[i].c[0]] += hw[hv[i].sample_i];
      cardcount[hv[i].c[1]] += hw[hv[i].sample_i];
    }
  
  if (win)
    {
      for (i = 0; i < SAMPLES; i++)
	{
	  ev[hv[i].sample_i] = (tot_ev - cardcount[hv[i].c[0]] - cardcount[hv[i].c[1]] + hw[hv[i].sample_i]) * (potsize-stake);
	}
    }
  else
    {
      for (i = 0; i < SAMPLES; i++)
	{
	  ev[hv[i].sample_i] = -((tot_ev - cardcount[hv[i].c[0]] - cardcount[hv[i].c[1]] + hw[hv[i].sample_i]) * stake);
	}
    }
    
  return ev;
}

void get_showdown_ev(double *hw, double *ev, struct hand_hv2 *hv, double potsize, double stake)
{
  //HAX 2PLR KOKO FUNKTIO
  int i,j, prev_hv_change = 0;
  double tot;
  double curhv_hw = 0, tot_hw = 0;
  //double w,t;
  HandVal curhv;
  double cardcount[52];
  double cardcount_curhv[52];
  
  memset(cardcount, 0, sizeof(cardcount));
  memset(cardcount_curhv, 0, sizeof(cardcount_curhv));

  curhv = -1;
  for (i = 0; i < SAMPLES ;i++)
    {
      if (hv[i].hv != curhv)
	{
	  for (j = prev_hv_change; j < i;j++)
	    {
	      ev[hv[j].sample_i] = tot_hw - cardcount[hv[j].c[0]] - cardcount[hv[j].c[1]];
	      ev[hv[j].sample_i] += (curhv_hw - cardcount_curhv[hv[j].c[0]] + hw[hv[j].sample_i] - cardcount_curhv[hv[j].c[1]])/2.0;
	    }
	  prev_hv_change = i;

	  for (j = 0; j < 52; j++)
	    cardcount[j] += cardcount_curhv[j];

	  tot_hw+=curhv_hw;

	  memset(cardcount_curhv, 0, sizeof(cardcount_curhv));	  
	  cardcount_curhv[hv[i].c[0]] = hw[hv[i].sample_i];
	  cardcount_curhv[hv[i].c[1]] = hw[hv[i].sample_i];
	  curhv_hw = hw[hv[i].sample_i];
	  curhv = hv[i].hv;
	}
      else
	{
	  cardcount_curhv[hv[i].c[0]]+=hw[hv[i].sample_i];
	  cardcount_curhv[hv[i].c[1]]+=hw[hv[i].sample_i];
	  curhv_hw+=hw[hv[i].sample_i];
	}
      
    }
  
  for (j = prev_hv_change; j < i;j++)
    {
      ev[hv[j].sample_i] = tot_hw - cardcount[hv[j].c[0]] - cardcount[hv[j].c[1]];
      ev[hv[j].sample_i] += (curhv_hw - cardcount_curhv[hv[j].c[0]] + hw[hv[j].sample_i] - cardcount_curhv[hv[j].c[1]])/2.0;
    }


  
  prev_hv_change = i;
  
  for (j = 0; j < 52; j++)
    cardcount[j] += cardcount_curhv[j];
  
  tot_hw+=curhv_hw;
  

   for (i = 0; i < SAMPLES;i++)
     { 
       tot = tot_hw - cardcount[hv[i].c[0]] + hw[hv[i].sample_i] - cardcount[hv[i].c[1]];
       ev[hv[i].sample_i] = ev[hv[i].sample_i] * potsize - tot*stake;
     } 
  /* 	  odds[hv[i].sample_i] /= tmp_hw; */
  /* 	  //odds[hv[i].sample_i] /= (tot_hw - cardcount[hv[i].c[0]] - cardcount[hv[i].c[1]] + hw[hv[i].sample_i]); */
  /* 	} */
  /*     else */
  /* 	{ */
  /* 	  //printf("get odds: %e\n", tmp_hw); */
  /* 	  //kaveri voi olla taalla ainoastaan korteilla jotka tama kasi blockaa -> voitto??? */
  /* 	  assert(tmp_hw > -1e-12); */
  /* 	  odds[hv[i].sample_i] = -1000000000.0; */
  /* 	  //odds[hv[i].sample_i] = 0; */
  /* 	} */
  /*     assert(odds[hv[i].sample_i] < 1.001); */
  /*   } */
}

void *my_alloc(struct mem_list **m)
{
  void *ret;
  struct mem_list *l = *m;

  if (l != NULL)
    {
      ret = l->mem;
      *m = l->next;
      free(l);
    }
  else
    {
      ret = malloc(sizeof(double)*SAMPLES);
    }
  return ret;
}

void my_free(void *ptr, struct mem_list **m)
{
  struct mem_list *l = malloc(sizeof(struct mem_list));
  l->next = *m;
  l->mem = ptr;
  *m = l;
}

void walk_tree(struct unique_root *us, struct types_data *td, double **to_down_ev, double **hw, double *pw, int bets, double stake[MAX_PLR], double stake_adjust[MAX_PLR][ACTS], double potsize, uint64_t flags[MAX_PLR], gsl_rng *rng, double hand_expand_odds, double pub_expand_odds, struct mem_list **mlist)
{
  int i, j,sample_i, act_i,pov_seat, nonpov_seat;
  struct plrmodel_node *n;
  struct situ *s;
  //double pw;
  double *ev[ACTS];
  double *new_hw[MAX_PLR];
  double new_pw[MAX_PLR];
  double new_stake[MAX_PLR];
  double new_potsize;
  double *from_up_ev[MAX_PLR];
  //double **to_down_ev;
  int16_t *private_types;
  int16_t local_mapping[SAMPLES];
  //int16_t local_mapping2[SAMPLES];
  double odds[ACTS][SAMPLES];
/* get plrmodel_node */
  int16_t local_type, global_type, local_slot;
  int new_bets;
  uint64_t pov_flags, nonpov_flags;
      
  //return to_down_ev;
  /* palataan jos kaikki pw nolla */
  for (i = 0; i < MAX_PLR;i++)
    if (pw[i] > 0)
      break;
  if (i == MAX_PLR)
    {
      //printf("break pw0 branch %i %f\n", us->root_idx, potsize);
      return;
    }

  pov_seat = us->cur_seat;
  nonpov_seat = (us->cur_seat+1)%2;
  
  pov_flags = flags[pov_seat];
  nonpov_flags = flags[nonpov_seat];

  if (us->gamestate == GS_S)
    {
      /*potsize = 0;
      for (i = 0; i < MAX_PLR;i++)
	potsize += stake[i];
      */
      if (us->to_act == 1) //FOLD
	{
	  get_fold_ev(hw[pov_seat], to_down_ev[nonpov_seat], td->vals, potsize, stake[nonpov_seat], 0);
	  get_fold_ev(hw[nonpov_seat], to_down_ev[pov_seat], td->vals, potsize, stake[pov_seat], 1);
	}
      else
	{
	  get_showdown_ev(hw[0], to_down_ev[1], td->vals, potsize, stake[1]);
	  get_showdown_ev(hw[1], to_down_ev[0], td->vals, potsize, stake[0]);
	}
      return;
    }



  n = get_first_matching_situ(us->model_tree, td->public_types, pub_expand_odds*(pw[pov_seat]/(double)SAMPLES), rng,1);
  s = (struct situ*)n->next_list;

  /* if (msg->flags &POV_UPDATE_VISITS)  */
  /*   update_visits(n, msg->v[0]->path_weight, msg->direction); */
  
  private_types = td->private_types[n->t->id];

  /* LOCK PLRMODEL_NODE */
  //lock_plrmodel_node(n);
  
      
  // generate private types mapping if needed


  if (n->type_mapping == NULL)
    {
      lock_plrmodel_node_write(&n->use_lock);
      if (n->type_mapping == NULL)
  	{
  	  n->type_mapping = malloc(sizeof(int16_t)*n->t->n_types);
  	  memset(n->type_mapping, 0xff, sizeof(int16_t)*n->t->n_types);
  	  //generate_mapping_from_global_to_local(n);
  	}
      unlock_plrmodel_node_write(&n->use_lock);
    }

  /* if (hand_expand_odds > 0 && n->root->gamestate > 0) */
  /*   if (gsl_rng_uniform(rng) < hand_expand_odds*(pw[pov_seat]/SAMPLES)*pub_expand_odds) */
  /*     { */
  /* 	double sum = 0; */
  /* 	double cutoff = gsl_rng_uniform(rng)*pw[pov_seat]; */
  /* 	for (i = 0; i < SAMPLES; i++) */
  /* 	  { */
  /* 	    sum+=hw[pov_seat][i]; */
  /* 	    if (sum > cutoff) */
  /* 	      { */
  /* 		int16_t new_type; */
  /* 		int16_t closest_slot_i; */
  /* 		lock_plrmodel_node_write(&n->struct_lock); */
  /* 		new_type = private_types[i]; */
  /* 		if (!is_bit_set(n->types_bmap, new_type)) */
  /* 		  { */
  /* 		    closest_slot_i = get_closest_local_type(n, new_type); */
  /* 		    copy_slot_to_type(n, new_type, closest_slot_i); */
  /* 		  } */
  /* 		//free(n->type_mapping); */
  /* 		//generate_mapping_from_global_to_local(n); */
  /* 		//set_bit(n->expand_bmap, private_types[i]); */
  /* 		memset(n->type_mapping, 0xff, sizeof(int16_t)*n->t->n_types); */
  /* 		unlock_plrmodel_node_write(&n->struct_lock); */
  /* 	      } */
  /* 	  } */
  /*     } */
 
  if (hand_expand_odds > 0 && n->root->gamestate > 0)
    {
      int type_i;
      int closest_slot_i;
      //int changed = 0;
      int locked = 0;

      for (i = 0; i < SAMPLES; i++)
	{
	  type_i = private_types[i];
	  if (hw[pov_seat][i] > 0 && !is_bit_set(n->types_bmap, type_i))
	    if (hw[pov_seat][i]*hand_expand_odds > gsl_rng_uniform(rng))
	      {
		if (!locked)
		  {
		    lock_plrmodel_node_write(&n->struct_lock);
		    locked = 1;
		  }
		if (!is_bit_set(n->types_bmap, type_i))
		  {
		    closest_slot_i = get_closest_local_type(n, type_i);
		    copy_slot_to_type(n, type_i, closest_slot_i);
		    //changed = 1;
		  }
		
		
	      }
	}
      if (locked)
	{
	  memset(n->type_mapping, 0xff, sizeof(int16_t)*n->t->n_types);
	  unlock_plrmodel_node_write(&n->struct_lock);
	}


    }

  lock_plrmodel_node_read(&n->struct_lock);
  //lock_plrmodel_node_read(&n->use_lock);
  
  for (sample_i = 0; sample_i < SAMPLES; sample_i++)
    {
      global_type = private_types[sample_i];
      if (n->type_mapping[global_type] == -1)
  	{
  	  int tmp_type_i = find_closest_type(n->t, n->types_bmap, global_type, n->len);
  	  assert(tmp_type_i != -1);
  	  n->type_mapping[global_type] = type_to_slot(n->types_bmap, tmp_type_i);

  	}
      local_type = n->type_mapping[global_type];
      assert(local_type != -1);
      local_mapping[sample_i] = local_type;
    }
  
  /* for (i = 0; i < td->n_types[n->root->gamestate]; i++) */
  /*   { */
  /*     struct type_count *tc = &td->tc[n->root->gamestate][i]; */
	
  /*     global_type = tc->type; */
  /*     local_type = find_closest_type(n->t, n->types_bmap, global_type, n->len); */
  /*     local_slot = type_to_slot(n->types_bmap, local_type); */
  /*     for (j = 0; j < tc->count; j++) */
  /* 	{ */
  /* 	  local_mapping[tc->mapping[j]] = local_slot; */
  /* 	} */
  /*   } */

  //assert(memcmp(local_mapping, local_mapping2, sizeof(local_mapping)) == 0);

  /* if (pov_flags&UPDATE_OHD) */
  /*   { */
  /*     if (n->ohd == NULL) */
  /* 	{ */
  /* 	  n->ohd = calloc(1,sizeof(double)*n->t->n_types); */
  /* 	} */
  /*     for (sample_i = 0; sample_i < SAMPLES; sample_i++) */
  /* 	{ */
  /* 	  n->ohd[private_types[sample_i]] += hw[sample_i]; */
  /* 	} */
  /*   } */

  if (pov_flags&POV_ODDS_FROM_REGS)
    get_odds_from_regs(n, odds, local_mapping, rng);
  else if (pov_flags&POV_ODDS_FROM_AVG)
    get_odds_from_avg(n, odds, local_mapping, rng);
  else if (pov_flags&POV_ODDS_FROM_BYTE_ODDS)
    get_odds_from_byte_odds(n, odds, local_mapping, rng);

  //unlock_plrmodel_node_read(&n->use_lock);
  
  //rnd = (double)zrandom_r()/((double)RAND_MAX+1);
  //assert(msg->n_hwev == 1);
 
    
    
  //new_hw = new_v->hwev;
  
  new_hw[nonpov_seat] = hw[nonpov_seat];
  new_pw[nonpov_seat] = pw[nonpov_seat];
  //new_hw[pov_seat] = malloc(sizeof(double)*SAMPLES);
  new_hw[pov_seat] = my_alloc(mlist);

  for (i = 0; i < MAX_PLR;i++)
    from_up_ev[i] = my_alloc(mlist);
  //from_up_ev[i] = malloc( sizeof(double)*SAMPLES);

  for (act_i = 0; act_i < ACTS; act_i++)
    {
      //ev[act_i] = calloc(1, sizeof(double)*SAMPLES);
      
      if (pw[pov_seat] > 0)
	{
	  vector_mul(new_hw[pov_seat], hw[pov_seat], odds[act_i], SAMPLES);
	  new_pw[pov_seat] = vector_sum(new_hw[pov_seat], SAMPLES);
	}
      else
	{
	  memset(new_hw[pov_seat], 0, sizeof(double)*SAMPLES);
	  new_pw[pov_seat] = 0;
	}
      
	
      if (us->next[act_i] != NULL)
	{
	  if (us->next[act_i]->gamestate != us->gamestate && us->gamestate < 3)
	    {
	      td->public_types[1+us->gamestate] = us->cur_seat;
	      td->public_types[4+us->gamestate] = bets;
	      new_bets = 0;
	    }
	  else
	    {
	      if (act_i == 0)
		new_bets = bets+1;
	      else
		new_bets = bets;
	    }
	  memcpy(new_stake, stake, sizeof(double)*MAX_PLR);
	  new_stake[pov_seat] += n->root->action_cost[act_i]*stake_adjust[pov_seat][act_i];//*0.9;
	  new_potsize = potsize + n->root->action_cost[act_i];

	  for (i = 0; i < MAX_PLR;i++)
	    memset(from_up_ev[i], 0, sizeof(double)*SAMPLES);
	  walk_tree(us->next[act_i], td, from_up_ev, new_hw, new_pw, new_bets, new_stake, stake_adjust, new_potsize, flags, rng, hand_expand_odds, pub_expand_odds, mlist);
	  
	  ev[act_i] = from_up_ev[pov_seat];
	  //from_up_ev[pov_seat] = malloc(sizeof(double)*SAMPLES);
	  from_up_ev[pov_seat] = my_alloc(mlist);

	  vector_add(to_down_ev[nonpov_seat], from_up_ev[nonpov_seat], SAMPLES);	
	  vector_mul_add(to_down_ev[pov_seat], ev[act_i], odds[act_i], SAMPLES);
	}
      else
	{
	  ev[act_i] = NULL;
	}
    }

  for (i = 0; i < MAX_PLR;i++)
    my_free(from_up_ev[i], mlist);
    //free(from_up_ev[i]);

  lock_plrmodel_node_write(&n->use_lock);

  if (pov_flags&UPDATE_REGS)
    {
      for (act_i = 0; act_i < ACTS; act_i++)
  	{
	  if (us->next[act_i] == NULL || (act_i == 2 && us->action_cost[1] == 0))
	    continue;

	  for (sample_i = 0; sample_i < SAMPLES; sample_i++)
	    {
	      local_type = local_mapping[sample_i];
	      s->regs[act_i][local_type] += ev[act_i][sample_i] - to_down_ev[pov_seat][sample_i];
	    }
	  
	}
    }  
    
  if (pov_flags&UPDATE_AVG_ODDS)
    {
      for (act_i = 0; act_i < ACTS; act_i++)
  	{
	  if (us->next[act_i] == NULL || (act_i == 2 && us->action_cost[1] == 0))
	    continue;

	  for (sample_i = 0; sample_i < SAMPLES; sample_i++)
	    {
	      local_type = local_mapping[sample_i];
	      s->avg_odds[act_i][local_type] += odds[act_i][sample_i]*hw[pov_seat][sample_i];
	    }
	}
    }
  unlock_plrmodel_node_write(&n->use_lock);
  pmn_unlock_struct(n);
  //  unlock_plrmodel_node_read(&n->struct_lock);

  //free(new_hw[pov_seat]);
  my_free(new_hw[pov_seat], mlist);
  for (act_i = 0; act_i < ACTS; act_i++)
    if(ev[act_i] != NULL)
      my_free(ev[act_i], mlist);
      //free(ev[act_i]);
  //return to_down_ev;
}

void get_odds_from_avg_br(struct plrmodel_node *n, double odds[ACTS][HANDS], int16_t local_mapping[HANDS], gsl_rng *rng)
{
  double tot;
  int sample_i, act_i;
  struct situ *s;
  int16_t local_type;

  s = (struct situ*)n->next_list;
 

  for (sample_i = 0; sample_i < HANDS; sample_i++)
    {
      local_type = local_mapping[sample_i];      
      if (local_type == -1)
	{
	  odds[0][sample_i] = 0;
	  odds[1][sample_i] = 0;
	  odds[2][sample_i] = 0; 
	  continue;
	}
      tot = 0;

      if (n->root->next[0] == NULL)
	{
	  //assert(s->avg_odds[0][local_type] <= 0);
	  odds[0][sample_i] = 0;
	}
      else
	{
	  odds[0][sample_i] = s->avg_odds[0][local_type];
	  tot += odds[0][sample_i];
	}
      odds[1][sample_i] = s->avg_odds[1][local_type];
      tot += odds[1][sample_i];
      
      if (n->root->action_cost[1] == 0)
	{
	  odds[1][sample_i] += s->avg_odds[2][local_type];
	  tot += s->avg_odds[2][local_type];
	  odds[2][sample_i] = 0;
	}
      else
	{
	  odds[2][sample_i] = s->avg_odds[2][local_type];
	  tot += odds[2][sample_i];
	}

      /* for (act_i = 0; act_i < ACTS;act_i++) */
      /* 	{ */
      /* 	  odds[act_i][sample_i] = s->avg_odds[act_i][local_type] > 0?s->avg_odds[act_i][local_type]:0.0; */
      /* 	  tot += odds[act_i][sample_i]; */
      /* 	} */
      /* if (n->root->next[0] == NULL) */
      /* 	{ */
      /* 	  tot -= odds[0][sample_i]; */
      /* 	  odds[0][sample_i] = 0; */
      /* 	} */
      /* if (n->root->action_cost[1] == 0) */
      /* 	{ */
      /* 	  tot -= odds[2][sample_i]; */
      /* 	  odds[2][sample_i] = 0; */
      /* 	} */
      //tot = 0;
      if (tot == 0)
	{
	  for (act_i = 0; act_i < ACTS; act_i++)
	    {
	      odds[act_i][sample_i] = zrand_gsl(rng);
	      tot += odds[act_i][sample_i];
	    }
	
	  if (n->root->next[0] == NULL)
	    {
	      tot -= odds[0][sample_i];
	      odds[0][sample_i] = 0;
	    }
	  if (n->root->action_cost[1] == 0)
	    {
	      tot -= odds[2][sample_i];
	      odds[2][sample_i] = 0;
	    }
	}
      for (act_i = 0; act_i < ACTS; act_i++)
	{
	  odds[act_i][sample_i] /= tot;
	}
      /* if (n->root->gamestate < 3) */
      /* 	{ */
      /* 	  odds[0][sample_i] = 0; */
      /* 	  odds[1][sample_i] = 1; */
      /* 	  odds[2][sample_i] = 0; */
      /* 	} */
      /* else */
      /* 	{ */
      /* 	  odds[0][sample_i] = 0; */
      /* 	  odds[1][sample_i] = 0; */
      /* 	  odds[2][sample_i] = 1; */
      /* 	} */
      if (n->root->next[0] != NULL)
      	{
      	  odds[0][sample_i] = 1;
      	  odds[1][sample_i] = 0;
      	  odds[2][sample_i] = 0;
      	}
      else
      	{
      	  odds[0][sample_i] = 0;
      	  odds[1][sample_i] = 1;
      	  odds[2][sample_i] = 0;
	}
    }
}



double *get_fold_ev_br(double *hw, double *ev, int16_t *local_types, double potsize, double stake, int win)
{
  int i;
  double cardcount[52];
  double tot_ev = 0;
  memset(cardcount, 0, sizeof(cardcount));
  //ev = malloc(sizeof(double)*SAMPLES);
  
  for (i = 0; i < HANDS; i++)
    {
      cardcount[int_to_cards_2[i].c1] += hw[i];
      cardcount[int_to_cards_2[i].c2] += hw[i];
      tot_ev += hw[i];
    /* if (hv[i].sample_i == -1) */
    /* 	continue; */
    /*   tot_ev += hw[hv[i].sample_i]; */
    /*   cardcount[hv[i].c[0]] += hw[hv[i].sample_i]; */
    /*   cardcount[hv[i].c[1]] += hw[hv[i].sample_i]; */
    }
  
  if (win)
    {
      for (i = 0; i < HANDS; i++)
	{
	  if (local_types[i] == -1)
	    {
	      ev[i] = 0;
	      continue;
	    }

	  ev[i] = (tot_ev - cardcount[int_to_cards_2[i].c1] - cardcount[int_to_cards_2[i].c2] + hw[i]) * (potsize-stake);
	}
    }
  else
    {
      for (i = 0; i < HANDS; i++)
	{
	  if (local_types[i] == -1)
	    {
	      ev[i] = 0;
	      continue;
	    }
	  
	  ev[i] = -((tot_ev - cardcount[int_to_cards_2[i].c1] - cardcount[int_to_cards_2[i].c2] + hw[i]) * stake);
	}
    }
    
  return ev;
}

void get_showdown_ev_br(double *hw, double *ev, struct hand_hv2 *hv, double potsize, double stake)
{
  //HAX 2PLR KOKO FUNKTIO
  int i,j, prev_hv_change = 0;
  double tot;
  double curhv_hw = 0, tot_hw = 0;
  //double w,t;
  HandVal curhv;
  double cardcount[52];
  double cardcount_curhv[52];
  
  memset(cardcount, 0, sizeof(cardcount));
  memset(cardcount_curhv, 0, sizeof(cardcount_curhv));

  curhv = -1;
  for (i = 0; i < HANDS ;i++)
    {
      if (hv[i].c[0] == -1)
	continue;

      if (hv[i].hv != curhv)
	{
	  for (j = prev_hv_change; j < i;j++)
	    {
	      if (hv[j].c[0] == -1)
		continue;
	      
	      ev[hv[j].sample_i] = tot_hw - cardcount[hv[j].c[0]] - cardcount[hv[j].c[1]];
	      ev[hv[j].sample_i] += (curhv_hw - cardcount_curhv[hv[j].c[0]] + hw[hv[j].sample_i] - cardcount_curhv[hv[j].c[1]])/2.0;
	    }
	  prev_hv_change = i;

	  for (j = 0; j < 52; j++)
	    cardcount[j] += cardcount_curhv[j];

	  tot_hw+=curhv_hw;

	  memset(cardcount_curhv, 0, sizeof(cardcount_curhv));	  
	  cardcount_curhv[hv[i].c[0]] = hw[hv[i].sample_i];
	  cardcount_curhv[hv[i].c[1]] = hw[hv[i].sample_i];
	  curhv_hw = hw[hv[i].sample_i];
	  curhv = hv[i].hv;
	}
      else
	{
	  cardcount_curhv[hv[i].c[0]]+=hw[hv[i].sample_i];
	  cardcount_curhv[hv[i].c[1]]+=hw[hv[i].sample_i];
	  curhv_hw+=hw[hv[i].sample_i];
	}
      
    }
  
  for (j = prev_hv_change; j < i;j++)
    {
      if (hv[j].c[0] == -1)
	continue;

      ev[hv[j].sample_i] = tot_hw - cardcount[hv[j].c[0]] - cardcount[hv[j].c[1]];
      ev[hv[j].sample_i] += (curhv_hw - cardcount_curhv[hv[j].c[0]] + hw[hv[j].sample_i] - cardcount_curhv[hv[j].c[1]])/2.0;
    }


  
  prev_hv_change = i;
  
  for (j = 0; j < 52; j++)
    cardcount[j] += cardcount_curhv[j];
  
  tot_hw+=curhv_hw;
  

   for (i = 0; i < HANDS;i++)
     { 
       if (hv[i].c[0] == -1)
	 continue;
       
       tot = tot_hw - cardcount[hv[i].c[0]] + hw[hv[i].sample_i] - cardcount[hv[i].c[1]];
       //tot *= (HANDS/1081.0);
       ev[hv[i].sample_i] = ev[hv[i].sample_i] * potsize - tot*stake;
     } 
  /* 	  odds[hv[i].sample_i] /= tmp_hw; */
  /* 	  //odds[hv[i].sample_i] /= (tot_hw - cardcount[hv[i].c[0]] - cardcount[hv[i].c[1]] + hw[hv[i].sample_i]); */
  /* 	} */
  /*     else */
  /* 	{ */
  /* 	  //printf("get odds: %e\n", tmp_hw); */
  /* 	  //kaveri voi olla taalla ainoastaan korteilla jotka tama kasi blockaa -> voitto??? */
  /* 	  assert(tmp_hw > -1e-12); */
  /* 	  odds[hv[i].sample_i] = -1000000000.0; */
  /* 	  //odds[hv[i].sample_i] = 0; */
  /* 	} */
  /*     assert(odds[hv[i].sample_i] < 1.001); */
  /*   } */
}

void get_showdown_ev_br_hard_way(double *hw, double *ev, struct hand_hv2 *hv, double potsize, double stake)
//void get_victory_odds_against_spread_float_hard_way(struct hwev_data **hd, struct hand_hv2 *hv, int target_pos)
{

  //HAX 2PLR KOKO FUNKTIO
  int i,j;
  double tot_hw = 0;
  //double w,t;
  HandVal curhv;
  

  //double *ev = hd[target_pos]->d;//2PLR
  //double *hw = hd[(target_pos+1)%2]->d;
  //tot_hw = hw[HANDS];
  //memset(ev, 0, sizeof(double)*(HANDS));
  
  curhv = -1;
  for (i = 0; i < HANDS ;i++)
    {
      if (hv[i].c[0] == -1)
	continue;
      
      tot_hw = 0;
      for (j = 0; j < HANDS; j++)
	{
	  if (hv[i].c[0] == hv[j].c[0] || hv[i].c[0] == hv[j].c[1] || hv[i].c[1] == hv[j].c[0] || hv[i].c[1] == hv[j].c[1])
	    continue; 
	  if (hv[i].hv > hv[j].hv)
	    {
	      ev[hv[i].sample_i] += hw[hv[j].sample_i];
	    }
	  else if (hv[i].hv == hv[j].hv)
	    {
	      ev[hv[i].sample_i] += hw[hv[j].sample_i]/2.0;
	    }
	  tot_hw += hw[hv[j].sample_i];
	}
      ev[hv[i].sample_i] = ev[hv[i].sample_i] * potsize - tot_hw*stake;
    } 

}



double br_solve(struct unique_root *us, char *path, int path_i, int bets, int flop, int turn, int river, struct street_types *preflop_slots, struct street_types *flop_slots, struct street_types turn_slots[52], struct street_types river_slots[52*52], struct hand_hv2 handvals[52*52], int16_t *public_types, int16_t *private_types, double to_down_ev[2][HANDS], double hw[2][HANDS], double stake[MAX_PLR], uint64_t flags[MAX_PLR], gsl_rng *rng, char *save_path)
{
  
  //struct br_flop local_data;
  double to_up_hw[2][HANDS];
  double odds[ACTS][HANDS];
  double ev[ACTS][2][HANDS];
  //double from_up_ev[2][HANDS];
  double new_stake[MAX_PLR];
  int16_t local_mapping[HANDS];
  uint64_t pov_flags, nonpov_flags;
  int pov_seat, nonpov_seat, act_i, i, hand_i, new_bets;
  int16_t local_type, global_type;
 
  struct plrmodel_node *n;
  struct situ *s;
  double potsize;
  //FILE *fp = NULL;
  double retval = 0;
  
//int16_t *private_types;
  //uint64_t to_up_path; 

  pov_seat = us->cur_seat;
  nonpov_seat = (us->cur_seat+1)%2;
  pov_flags = flags[pov_seat];
  nonpov_flags = flags[nonpov_seat];
  memset(ev, 0, sizeof(ev));


  /* terminal node */
  if (us->gamestate == 4)
    {
      potsize = 0;
      for (i = 0; i < MAX_PLR;i++)
	potsize += stake[i];
      
      /* for (hand_i = 0; hand_i < HANDS; hand_i++) */
      /* 	{ */
      /* 	  retval += hw[0][hand_i]; */
      /* 	  retval += hw[1][hand_i]; */
      /* 	  //assert(!isnan(hw[0][hand_i])); */
      /* 	  //assert(!isnan(hw[1][hand_i])); */
      /* 	} */
      if (us->to_act == 1) //FOLD
	{

	  get_fold_ev_br(hw[pov_seat], to_down_ev[nonpov_seat], private_types, potsize, stake[nonpov_seat], 0);
	  get_fold_ev_br(hw[nonpov_seat], to_down_ev[pov_seat], private_types, potsize, stake[pov_seat], 1);
	}
      else
	{
	  double tmp_ev[HANDS];
	  double tmp_tot = 0, tmp_tot2 = 0, tmp_tot3 = 0;
	  /* for (hand_i = 0; hand_i < HANDS; hand_i++) */
	  /*   { */
	  /*     if (handvals[turn*52*HANDS+river*HANDS+hand_i].c[0] == -1) */
	  /* 	assert(private_types[handvals[turn*52*HANDS+river*HANDS+hand_i].sample_i] == -1); */
	  /*   } */

	  get_showdown_ev_br(hw[0], to_down_ev[1], &handvals[turn*52*HANDS+river*HANDS], potsize, stake[1]);
	  memset(tmp_ev, 0, sizeof(tmp_ev));
	  get_showdown_ev_br_hard_way(hw[0], tmp_ev, &handvals[turn*52*HANDS+river*HANDS], potsize, stake[1]);
	  for (i = 0; i < HANDS; i++)
	    {
	      tmp_tot += hw[0][i];
	      tmp_tot2 += to_down_ev[1][i];
	      tmp_tot3 += tmp_ev[i];
	      tmp_ev[i] = tmp_ev[i] - to_down_ev[1][i];
	    }
	  get_showdown_ev_br(hw[1], to_down_ev[0], &handvals[turn*52*HANDS+river*HANDS], potsize, stake[0]);
	}
      /* for (hand_i = 0; hand_i < HANDS; hand_i++) */
      /* 	{ */
      /* 	  assert(!isnan(to_down_ev[0][hand_i])); */
      /* 	  assert(!isnan(to_down_ev[1][hand_i])); */
      /* 	} */

      return retval;
    }


  if (us->gamestate == 1 && pov_flags&BR_LOAD_FLOP_DATA)
    {
      char full_filename[1024];
      char filename[256];
      
      FILE *fp;
      
      strcpy(full_filename, save_path);
      sprintf(filename, "%i/", flop);
      for (i = 0; i < path_i; i++)
	{
	  if (path[i] == 0)
	    strcat(filename, "0");
	  else if (path[i] == 1)
	    strcat(filename, "1");
	  else
	    assert(0);
	}
      strcat(full_filename, filename);
      fp = fopen(full_filename, "rb");
      if (fp)
	{
	  //double tot_start, tot_end;
	  
	  printf("loading %s\n", full_filename);
	  fread(to_up_hw, sizeof(double), 2*HANDS, fp);
	  fread(ev, sizeof(double), ACTS*2*HANDS, fp);
	  fclose(fp);

	  /* bug fix */
	  /* for (hand_i = 0; hand_i < HANDS; hand_i++) */
	  /*   { */
	  /*     if (CardMask_ANY_SET(int_to_cardmask_3[flop], int_to_cardmask_2[hand_i])) */
	  /* 	{ */
		  
	  /* 	  assert(to_up_hw[0][hand_i] == 0); */
	  /* 	  assert(to_up_hw[1][hand_i] == 0); */
	  /* 	  for (act_i = 0; act_i < ACTS; act_i++) */
	  /* 	    for (i = 0; i < 2; i++) */
	  /* 	      assert(ev[act_i][i][hand_i] == 0); */
		  
	  /* 	} */
	  /*   } */
	  /*bugfix 2*/
	  //vector_mul_scalar(to_up_hw[0],to_up_hw[0],22100.0/18424.0, HANDS);
	  //vector_mul_scalar(to_up_hw[1],to_up_hw[1],22100.0/18424.0, HANDS); 
	  //for (act_i = 0; act_i < ACTS; act_i++)
	  //  for (i = 0; i < 2; i++)
	  //    vector_mul_scalar(ev[act_i][i],ev[act_i][i],22100.0/18424.0, HANDS); 
	  
	  //tot_start = vector_sum(to_down_ev[nonpov_seat], HANDS);
	  
	  for (act_i = 0; act_i < ACTS; act_i++)
	    vector_add(to_down_ev[nonpov_seat], ev[act_i][nonpov_seat], HANDS);
	  //tot_end =  vector_sum(to_down_ev[nonpov_seat], HANDS);
	  //printf("%f %f %f %f %f\n", tot_start, tot_end, tot_end-tot_start,(double)get_flop_morph_count(flop),(tot_end-tot_start)/(double)get_flop_morph_count(flop));
	  vector_add(hw[nonpov_seat], to_up_hw[nonpov_seat], HANDS);
	  vector_add(hw[pov_seat], to_up_hw[pov_seat], HANDS);
	  retval = (double)get_flop_morph_count(flop);
	}
      else
	return retval;
    }
  else
    {
      /* get plrmodel_node */
      if (vector_sum(hw[0],HANDS) == 0 && vector_sum(hw[1],HANDS) == 0)
	{
	  //printf("breaking branch ");
	  //for (i = 0; i < path_i; i++)
	  //  printf("%i", path[i]);
	  //printf("\n");
	  return retval;
	}


      n = get_first_matching_situ(us->model_tree, public_types, 0, 0,0);
      s = (struct situ*)n->next_list;
      
      if (n->type_mapping == NULL)
	{
	  lock_plrmodel_node_write(&n->use_lock);
	  generate_mapping_from_global_to_local(n);
	  unlock_plrmodel_node_write(&n->use_lock);
	}


      for (hand_i = 0; hand_i < HANDS; hand_i++)
	{
	  global_type = private_types[hand_i];
	  if (global_type == -1)
	    {
	      local_mapping[hand_i] = -1;
	      continue;
	    }
	  local_type = n->type_mapping[global_type];
	  local_mapping[hand_i] = local_type;
	}


      get_odds_from_avg_br(n, odds, local_mapping, rng);

      /* update hand weights */
     

      for (act_i = 0; act_i < ACTS; act_i++)
	{

	  if (us->next[act_i] == NULL)
	    continue;
	  path[path_i] = act_i;
      
	  memcpy(new_stake, stake, sizeof(double)*MAX_PLR);
	  new_stake[pov_seat] += n->root->action_cost[act_i];
      
	  if (us->gamestate != us->next[act_i]->gamestate && us->next[act_i]->gamestate != GS_S)
	    {
	      /* if gamestate change, update board */
	      double tmp_hw[HANDS];
	      int16_t *new_private_types = NULL;

	      vector_mul(tmp_hw, hw[pov_seat], odds[act_i], HANDS);
	      public_types[1+us->gamestate] = us->cur_seat;
	      public_types[4+us->gamestate] = bets;
	      new_bets = 0;
	      if (us->next[act_i]->gamestate == 1)
		{
		  double flop_morph_count;
		  if (pov_flags&BR_LOAD_FLOP_DATA)
		    {

		      double flop_count = 0;
		      double from_up_hw[2][HANDS];
		      assert(act_i == 1);
		      memset(from_up_hw, 0, sizeof(double)*2*HANDS);
		      //printf("SUM START %f %f\n", vector_sum(ev[act_i][0], HANDS), vector_sum(ev[act_i][1], HANDS));
		      for (flop = 0; flop < 22100; flop++)
			{
			  flop_count += br_solve(us->next[act_i], path, path_i+1, 0, flop, turn, river, preflop_slots, flop_slots, turn_slots, river_slots, handvals, public_types, new_private_types, ev[act_i], from_up_hw, new_stake, flags, rng,save_path);
			}
		      //printf("SUM %f %f\n", vector_sum(ev[act_i][0], HANDS), vector_sum(ev[act_i][1], HANDS));
		      /* cur_pw = vector_sum(tmp_hw, HANDS); */
		      /* from_up_pw = vector_sum(from_up_hw[pov_seat], HANDS); */
		      /* printf("%f %f %f %f %f %f\n", from_up_pw, cur_pw, from_up_pw/cur_pw, vector_sum(from_up_hw[nonpov_seat], HANDS), vector_sum(hw[nonpov_seat], HANDS), vector_sum(from_up_hw[nonpov_seat], HANDS)/vector_sum(hw[nonpov_seat], HANDS)); */
		      /* for (i = 0; i < HANDS; i++) */
		      /* 	{ */
		      /* 	  printf("|| %f %f %f", from_up_hw[nonpov_seat][i], hw[nonpov_seat][i], from_up_hw[nonpov_seat][i]/hw[nonpov_seat][i]); */
		      /* 	  //printf("|| %f %f %f", from_up_hw[pov_seat][i], tmp_hw[i], from_up_hw[pov_seat][i]/tmp_hw[i]); */
		      /* 	  if (!(i%5)) */
		      /* 	    printf("\n"); */
		      /* 	} */
			  
		      /* for (hand_i = 0; hand_i < HANDS; hand_i++) */
		      /* 	{ */
		      /* 	  if (from_up_hw[pov_seat][hand_i] > 0) */
		      /* 	    ev[act_i][pov_seat][hand_i] *= tmp_hw[hand_i]/from_up_hw[pov_seat][hand_i]; */
		      /* 	  if (from_up_hw[nonpov_seat][hand_i] > 0) */
		      /* 	    ev[act_i][nonpov_seat][hand_i] *= hw[nonpov_seat][hand_i]/from_up_hw[nonpov_seat][hand_i]; */
		      /* 	  printf("pov:%f %f %f   nonpov: %f %f %f\n", tmp_hw[hand_i], from_up_hw[pov_seat][hand_i],tmp_hw[hand_i]/from_up_hw[pov_seat][hand_i], hw[nonpov_seat][hand_i],from_up_hw[nonpov_seat][hand_i],hw[nonpov_seat][hand_i]/from_up_hw[nonpov_seat][hand_i]); */
		      /* 	} */
		      if (flop_count != 0)
			{
			  double avgev[169];
			  double from_up_avghw[169];
			  double cur_avghw[169];
			  int seat;
			  
			  for (seat = 0; seat < 2; seat++)
			    {
			      memset(avgev, 0, sizeof(double)*169);
			      memset(from_up_avghw, 0, sizeof(double)*169);
			      memset(cur_avghw, 0, sizeof(double)*169);
			      for (hand_i = 0; hand_i < HANDS; hand_i++)
				{
				  avgev[preflop_morph_mapping[hand_i]] += ev[act_i][seat][hand_i]*(22100.0/flop_count);		
				}
			      /* if (seat == pov_seat) */
			      /*   { */
			      /*     for (hand_i = 0; hand_i < HANDS; hand_i++) */
			      /* 	{ */
			      /* 	  cur_avghw[preflop_morph_mapping[hand_i]] += tmp_hw[hand_i]; */
			      /* 	  from_up_avghw[preflop_morph_mapping[hand_i]] += from_up_hw[pov_seat][hand_i]; */
			      /* 	} */
			      /*   } */
			      /* else */
			      /*   { */
			      /*     for (hand_i = 0; hand_i < HANDS; hand_i++) */
			      /* 	{ */
			      /* 	  cur_avghw[preflop_morph_mapping[hand_i]] += hw[nonpov_seat][hand_i]; */
			      /* 	  from_up_avghw[preflop_morph_mapping[hand_i]] += from_up_hw[nonpov_seat][hand_i]; */
			      /* 	} */
			      /*   } */
			      /* for (hand_i = 0; hand_i < 169; hand_i++) */
			      /*   { */
			      /*     if (from_up_avghw[hand_i] > 0) */
			      /* 	{ */
			      /* 	  avgev[hand_i] *= cur_avghw[hand_i]/from_up_avghw[hand_i]; */
			      /* 	  printf("%f %f %f\n", cur_avghw[hand_i],from_up_avghw[hand_i],cur_avghw[hand_i]/from_up_avghw[hand_i]); */
			      /* 	} */
			      /*   } */
			      for (hand_i = 0; hand_i < 13; hand_i++)
				avgev[hand_i] /= 6.0;
			      for (hand_i = 13; hand_i < 13+78; hand_i++)
				avgev[hand_i] /= 4.0;
			      for (hand_i = 13+78; hand_i < 13+78+78; hand_i++)
				avgev[hand_i] /= 12.0;
			      
			      for (hand_i = 0; hand_i < HANDS; hand_i++)
				{
				  ev[act_i][seat][hand_i] = avgev[preflop_morph_mapping[hand_i]];
		
				}
			    }
			}
		    }
		  else
		    {
		      //int16_t hax_new_private_types[HANDS];
		      flop_morph_count = (double)get_flop_morph_count(flop);
		      printf("flop_morph_count %i %f\n", flop, flop_morph_count);
		      public_types[9] = flop_slots->board_type;
		      new_private_types = flop_slots->hand_types;
		      //public_types[9] = 0;
		      //new_private_types = hax_new_private_types;
		      //tot = 0;
		      for (hand_i = 0; hand_i < HANDS; hand_i++)
			{
			  //if (CardMask_ANY_SET(int_to_cardmask_3[flop], int_to_cardmask_2[hand_i]))
			  if (new_private_types[hand_i] == -1)
			    {
			      //hax_new_private_types[hand_i] = -1;
			      to_up_hw[pov_seat][hand_i] = 0;
			      to_up_hw[nonpov_seat][hand_i] = 0;
			    }
			  else
			    {
			      //to_up_hw[pov_seat][hand_i] = tmp_hw[hand_i]*(flop_morph_count/19600.0)*((50.0*49.0)/(47.0*46.0));
			      //to_up_hw[nonpov_seat][hand_i] = hw[nonpov_seat][hand_i]*(flop_morph_count/19600.0)*((50.0*49.0)/(47.0*46.0));
			      //to_up_hw[pov_seat][hand_i] = tmp_hw[hand_i]*(22100.0/19600.0);//*((50.0*49.0)/(47.0*46.0));
			      //to_up_hw[nonpov_seat][hand_i] = hw[nonpov_seat][hand_i]*(22100.0/19600.0);//*((50.0*49.0)/(47.0*46.0));			      
			      //hax_new_private_types[hand_i] = 1;
			      to_up_hw[pov_seat][hand_i] = tmp_hw[hand_i]*(flop_morph_count/19600.0)*(1225.0/1081.0);
			      to_up_hw[nonpov_seat][hand_i] = hw[nonpov_seat][hand_i]*(flop_morph_count/19600.0)*(1225.0/1081.0);
			      //tot += to_up_hw[1][hand_i];
			    }
			}
		      
		      retval += br_solve(us->next[act_i], path, path_i+1, 0, flop, turn, river, preflop_slots, flop_slots, turn_slots, river_slots, handvals, public_types, new_private_types, ev[act_i], to_up_hw, new_stake, flags, rng,save_path);
		      //printf("start %f, retval %f\n", tot, retval);
		    }
		}
	      else if (us->next[act_i]->gamestate == 2)
		{
		  for (turn = 0; turn < 52; turn++)
		    {
		      if (CardMask_CARD_IS_SET(int_to_cardmask_3[flop], turn))
			continue;
		      //public_types[9] = flop_slots->board_type;
		      public_types[8] = turn_slots[turn].board_type;
		      new_private_types = turn_slots[turn].hand_types;
		      //public_types[8] = 0;
		      //new_private_types = NULL;

		      for (hand_i = 0; hand_i < HANDS; hand_i++)
			{
			  if (new_private_types[hand_i] == -1)
			    {
			      to_up_hw[pov_seat][hand_i] = 0;
			      to_up_hw[nonpov_seat][hand_i] = 0;
			    }
			  else
			    {
			      to_up_hw[pov_seat][hand_i] = tmp_hw[hand_i]*(1.0/47.0)*(1081.0/1035.0);//*((47.0*46.0)/(46.0*45.0));
			      to_up_hw[nonpov_seat][hand_i] = hw[nonpov_seat][hand_i]*(1.0/47.0)*(1081.0/1035.0);//*((47.0*46.0)/(46.0*45.0));
			    }
			}
		      retval += br_solve(us->next[act_i], path, path_i+1, 0, flop, turn, river, preflop_slots, flop_slots, turn_slots, river_slots, handvals, public_types, new_private_types, ev[act_i], to_up_hw, new_stake, flags, rng,save_path);
		    }
		}

	      else if (us->next[act_i]->gamestate == 3)
		{
		  for (river = 0; river < 52; river++)
		    {
		      if (CardMask_CARD_IS_SET(int_to_cardmask_3[flop], river) || turn == river)
			continue;
		      //public_types[9] = flop_slots->board_type;
		      public_types[7] = river_slots[turn*52+river].board_type;
		      new_private_types = river_slots[turn*52+river].hand_types;
		      for (hand_i = 0; hand_i < HANDS; hand_i++)
			{
			  if (new_private_types[hand_i] == -1)
			    {
			      to_up_hw[pov_seat][hand_i] = 0;
			      to_up_hw[nonpov_seat][hand_i] = 0;
			    }
			  else
			    {
			      to_up_hw[pov_seat][hand_i] = tmp_hw[hand_i]*(1.0/46.0)*(1035.0/990.0);//*((46.0*45.0)/(45.0*44.0));
			      to_up_hw[nonpov_seat][hand_i] = hw[nonpov_seat][hand_i]*(1.0/46.0)*(1035.0/990.0);//*((46.0*45.0)/(45.0*44.0));
			    }
			}
		      retval += br_solve(us->next[act_i], path, path_i+1, 0, flop, turn, river, preflop_slots, flop_slots, turn_slots, river_slots, handvals, public_types, new_private_types, ev[act_i], to_up_hw, new_stake, flags, rng,save_path);
		    }
		}
	  
	      //br_solve(us->next[act_i], path, path_i+1, 0, flop, turn, river, preflop_slots, flop_slots, turn_slots, river_slots, handvals, public_types, new_private_types, ev[act_i], to_up_hw, new_stake, flags, rng);
	    }
	  else
	    {
	      vector_mul(to_up_hw[pov_seat], hw[pov_seat], odds[act_i], HANDS);
	      memcpy(to_up_hw[nonpov_seat], hw[nonpov_seat], sizeof(double)*HANDS);
	      retval += br_solve(us->next[act_i], path, path_i+1, act_i == 0?bets+1:bets, flop, turn, river, preflop_slots, flop_slots, turn_slots, river_slots, handvals, public_types, private_types, ev[act_i], to_up_hw, new_stake, flags, rng,save_path);
	    }
	  vector_add(to_down_ev[nonpov_seat], ev[act_i][nonpov_seat], HANDS);
	}
    }
  for (hand_i = 0; hand_i < HANDS; hand_i++)
    {
      if (us->next[0] != NULL && ev[0][pov_seat][hand_i] > ev[1][pov_seat][hand_i])
	act_i = 0;
      else
	act_i = 1;
      if (ev[2][pov_seat][hand_i] > ev[act_i][pov_seat][hand_i])
	act_i = 2;
      to_down_ev[pov_seat][hand_i] += ev[act_i][pov_seat][hand_i];
    }
  
  /* for (hand_i = 0; hand_i < HANDS; hand_i++) */
  /*   { */
  /*     assert(!isnan(to_down_ev[0][hand_i])); */
  /*     assert(!isnan(to_down_ev[1][hand_i])); */
  /*   } */
  
  
  if (us->gamestate == 1 && pov_flags&BR_SAVE_FLOP_DATA)
    {
      char full_filename[1024];
      char filename[256];
      
      FILE *fp;

      strcpy(full_filename, save_path);
      sprintf(filename, "%i/", flop);
      for (i = 0; i < path_i; i++)
	{
	  if (path[i] == 0)
	    strcat(filename, "0");
	  else if (path[i] == 1)
	    strcat(filename, "1");
	  else
	    assert(0);
	}
      strcat(full_filename, filename);
      fp = fopen(full_filename, "wb");
      fwrite(hw, sizeof(double), 2*HANDS, fp);
      fwrite(ev, sizeof(double), ACTS*2*HANDS, fp);
      fclose(fp);
      /* write to disk */
    }
  return retval;
}


/* double get_fold_ev_one_hand_gen(double *hw, int16_t *local_types, double potsize, double stake, int win) */
/* { */
/*   int i; */
/*   double cardcount[52]; */
/*   double tot_ev = 0; */
/*   memset(cardcount, 0, sizeof(cardcount)); */
/*   //ev = malloc(sizeof(double)*SAMPLES); */
  
/*   for (i = 0; i < HANDS; i++) */
/*     { */
/*       cardcount[int_to_cards_2[i].c1] += hw[i]; */
/*       cardcount[int_to_cards_2[i].c2] += hw[i]; */
/*       tot_ev += hw[i]; */
/*     /\* if (hv[i].sample_i == -1) *\/ */
/*     /\* 	continue; *\/ */
/*     /\*   tot_ev += hw[hv[i].sample_i]; *\/ */
/*     /\*   cardcount[hv[i].c[0]] += hw[hv[i].sample_i]; *\/ */
/*     /\*   cardcount[hv[i].c[1]] += hw[hv[i].sample_i]; *\/ */
/*     } */
  
/*   if (win) */
/*     { */
/*       for (i = 0; i < HANDS; i++) */
/* 	{ */
/* 	  if (local_types[i] == -1) */
/* 	    { */
/* 	      ev[i] = 0; */
/* 	      continue; */
/* 	    } */

/* 	  ev[i] = (tot_ev - cardcount[int_to_cards_2[i].c1] - cardcount[int_to_cards_2[i].c2] + hw[i]) * (potsize-stake); */
/* 	} */
/*     } */
/*   else */
/*     { */
/*       for (i = 0; i < HANDS; i++) */
/* 	{ */
/* 	  if (local_types[i] == -1) */
/* 	    { */
/* 	      ev[i] = 0; */
/* 	      continue; */
/* 	    } */
	  
/* 	  ev[i] = -((tot_ev - cardcount[int_to_cards_2[i].c1] - cardcount[int_to_cards_2[i].c2] + hw[i]) * stake); */
/* 	} */
/*     } */
    
/*   return ev; */
/* } */

/* void get_showdown_ev_br(double *hw, double *ev, struct hand_hv2 *hv, double potsize, double stake) */
/* { */
/*   //HAX 2PLR KOKO FUNKTIO */
/*   int i,j, prev_hv_change = 0; */
/*   double tot; */
/*   double curhv_hw = 0, tot_hw = 0; */
/*   //double w,t; */
/*   HandVal curhv; */
/*   double cardcount[52]; */
/*   double cardcount_curhv[52]; */
  
/*   memset(cardcount, 0, sizeof(cardcount)); */
/*   memset(cardcount_curhv, 0, sizeof(cardcount_curhv)); */

/*   curhv = -1; */
/*   for (i = 0; i < HANDS ;i++) */
/*     { */
/*       if (hv[i].c[0] == -1) */
/* 	continue; */

/*       if (hv[i].hv != curhv) */
/* 	{ */
/* 	  for (j = prev_hv_change; j < i;j++) */
/* 	    { */
/* 	      if (hv[j].c[0] == -1) */
/* 		continue; */
	      
/* 	      ev[hv[j].sample_i] = tot_hw - cardcount[hv[j].c[0]] - cardcount[hv[j].c[1]]; */
/* 	      ev[hv[j].sample_i] += (curhv_hw - cardcount_curhv[hv[j].c[0]] + hw[hv[j].sample_i] - cardcount_curhv[hv[j].c[1]])/2.0; */
/* 	    } */
/* 	  prev_hv_change = i; */

/* 	  for (j = 0; j < 52; j++) */
/* 	    cardcount[j] += cardcount_curhv[j]; */

/* 	  tot_hw+=curhv_hw; */

/* 	  memset(cardcount_curhv, 0, sizeof(cardcount_curhv));	   */
/* 	  cardcount_curhv[hv[i].c[0]] = hw[hv[i].sample_i]; */
/* 	  cardcount_curhv[hv[i].c[1]] = hw[hv[i].sample_i]; */
/* 	  curhv_hw = hw[hv[i].sample_i]; */
/* 	  curhv = hv[i].hv; */
/* 	} */
/*       else */
/* 	{ */
/* 	  cardcount_curhv[hv[i].c[0]]+=hw[hv[i].sample_i]; */
/* 	  cardcount_curhv[hv[i].c[1]]+=hw[hv[i].sample_i]; */
/* 	  curhv_hw+=hw[hv[i].sample_i]; */
/* 	} */
      
/*     } */
  
/*   for (j = prev_hv_change; j < i;j++) */
/*     { */
/*       if (hv[j].c[0] == -1) */
/* 	continue; */

/*       ev[hv[j].sample_i] = tot_hw - cardcount[hv[j].c[0]] - cardcount[hv[j].c[1]]; */
/*       ev[hv[j].sample_i] += (curhv_hw - cardcount_curhv[hv[j].c[0]] + hw[hv[j].sample_i] - cardcount_curhv[hv[j].c[1]])/2.0; */
/*     } */


  
/*   prev_hv_change = i; */
  
/*   for (j = 0; j < 52; j++) */
/*     cardcount[j] += cardcount_curhv[j]; */
  
/*   tot_hw+=curhv_hw; */
  

/*    for (i = 0; i < HANDS;i++) */
/*      {  */
/*        if (hv[i].c[0] == -1) */
/* 	 continue; */
       
/*        tot = tot_hw - cardcount[hv[i].c[0]] + hw[hv[i].sample_i] - cardcount[hv[i].c[1]]; */
/*        //tot *= (HANDS/1081.0); */
/*        ev[hv[i].sample_i] = ev[hv[i].sample_i] * potsize - tot*stake; */
/*      }  */
/* } */

double get_showdown_ev_one_hand_gen(double *hw, struct hand_hv2 *hv, double potsize, double stake, int hand, int n_hands)
{
  //assume dead cards have hw == 0
  //HAX 2PLR KOKO FUNKTIO
  int i,j;
  double tot_hw = 0, ev;
  //double w,t;
  HandVal curhv;
  

  //double *ev = hd[target_pos]->d;//2PLR
  //double *hw = hd[(target_pos+1)%2]->d;
  //tot_hw = hw[HANDS];
  //memset(ev, 0, sizeof(double)*(HANDS));
  
  curhv = -1;
  tot_hw = 0;
  ev = 0;
  for (i = 0; i < n_hands ;i++)
    {
      tot_hw += hw[i];
    }
  for (i = 0; i < n_hands ;i++)
    {
      if (hv[i].c[0] == -1)
	continue;
      if (hv[i].sample_i == hand)
	break;
      ev += hw[hv[i].sample_i];
    }
  assert(i < n_hands);
  curhv = hv[i].hv;
  for (j = i-1; j >= 0; j--)
    {
      if (hv[j].hv != curhv)
	break;
      ev -= hw[hv[j].sample_i]/2.0;
    }
  for (j = i+1; j < n_hands; j++)
    {
      if (hv[j].hv != curhv)
	break;
      ev += hw[hv[j].sample_i]/2.0;
    }
  return ev * potsize - tot_hw*stake;
} 


void get_odds_from_byte_odds_gen(struct plrmodel_node *n, double *odds, int16_t *local_mapping, int n_hands, gsl_rng *rng)
{
  int sample_i;
  struct situ *s;
  int16_t local_type;

  s = (struct situ*)n->next_list;
 
  memset(odds, 0, sizeof(double)*ACTS*SAMPLES);

  for (sample_i = 0; sample_i < n_hands; sample_i++)
    {
      local_type = local_mapping[sample_i];
      if (local_type == -1)
	{
	  odds[0*n_hands+sample_i] = 0;
	  odds[1*n_hands+sample_i] = 0;
	  odds[2*n_hands+sample_i] = 0;
	  continue;
	}
      odds[0*n_hands+sample_i] = ((double)s->byte_odds[local_type*2])/255.0;
      odds[1*n_hands+sample_i] = ((double)s->byte_odds[local_type*2+1])/255.0;
      odds[2*n_hands+sample_i] = 1.0-odds[0*n_hands+sample_i]-odds[1*n_hands+sample_i];
      if (odds[2*n_hands+sample_i] < 0)
	{
	  odds[0*n_hands+sample_i] = 0.0;
	  odds[1*n_hands+sample_i] = 1.0;
	  odds[2*n_hands+sample_i] = 0.0;
	}
    }
}
 

 
void get_action_odds(struct unique_root *us, int16_t *public_types, int16_t *private_types, double *odds, gsl_rng *rng)
{
  struct plrmodel_node *n;
  struct situ *s;
  int16_t local_mapping[HANDS];
  int hand_i, sample_i;
  int16_t global_type, local_type;
  n = get_first_matching_situ(us->model_tree, public_types, 0, rng,0);
  s = (struct situ*)n->next_list;

  // generate private types mapping if needed

  if (n->type_mapping == NULL)
    {
      n->type_mapping = malloc(sizeof(int16_t)*n->t->n_types);
      assert(n->type_mapping != NULL);
      memset(n->type_mapping, 0xff, sizeof(int16_t)*n->t->n_types);
    }
  for (sample_i = 0; sample_i < HANDS; sample_i++)
    {
      global_type = private_types[sample_i];
      if (global_type == -1)
	{
	  local_mapping[sample_i] = -1;
	  continue;
	}
      if (n->type_mapping[global_type] == -1)
	{
	  int tmp_type_i = find_closest_type(n->t, n->types_bmap, global_type, n->len);
	  assert(tmp_type_i != -1);
	  n->type_mapping[global_type] = type_to_slot(n->types_bmap, tmp_type_i);
	}
      local_type = n->type_mapping[global_type];
      assert(local_type != -1);
      local_mapping[sample_i] = local_type;
    }
  free(n->type_mapping);
  n->type_mapping = NULL;
    
  /* for (hand_i = 0; hand_i < HANDS; hand_i++) */
  /*   { */
  /*     global_type = private_types[hand_i]; */
  /*     if (global_type == -1) */
  /* 	{ */
  /* 	  local_mapping[hand_i] = -1; */
  /* 	  continue; */
  /* 	} */
  /*     local_type = n->type_mapping[global_type]; */
  /*     local_mapping[hand_i] = local_type; */
  /*   } */
  
  get_odds_from_byte_odds_gen(n, odds, local_mapping, HANDS, rng);

}



void get_odds(double *odds, double *regs, int16_t *mapping, int32_t *valid_actions, int n_regs, int n_odds, gsl_rng *rng)
{

  int i,j;
  double tot;
  double *int_odds = malloc(sizeof(double)*n_regs*3);
  
  
  for (i = 0; i < n_regs*3; i++)
    {
      if (regs[i] > 0)
	int_odds[i] = regs[i];
      else
	int_odds[i] = 0;
    }
  for (i = 0; i < n_regs; i++)
    {
      tot = int_odds[i] + int_odds[n_regs + i] + int_odds[n_regs*2+i];
      if (likely(tot > 0))
	{	 
	  int_odds[i] = int_odds[i]/tot;
	  int_odds[n_regs + i] = int_odds[n_regs + i]/tot;
	  int_odds[n_regs*2 + i] = int_odds[n_regs*2 + i]/tot;
	}
      else
	{
	  int_odds[i] = gsl_rng_uniform(rng)*valid_actions[0];
	  int_odds[n_regs + i] = gsl_rng_uniform(rng)*valid_actions[1];
	  int_odds[n_regs*2 + i] = gsl_rng_uniform(rng)*valid_actions[2];
	  tot = int_odds[i] + int_odds[n_regs + i] + int_odds[n_regs*2+i];
	  assert(tot > 0);
	  int_odds[i] = int_odds[i]/tot;
	  int_odds[n_regs + i] = int_odds[n_regs + i]/tot;
	  int_odds[n_regs*2 + i] = int_odds[n_regs*2 + i]/tot;
	}
    }
  for (j = 0; j < 3; j++)
    for (i = 0; i < n_odds; i++)
      {
	if (mapping[i] == -1)
	  continue;
	odds[j*n_odds + i] = int_odds[j*n_regs + mapping[i]];
      }
  free(int_odds);
}

void update_regs(double *regs, double *odds, double *ev, int16_t *mapping,  double *valid_actions, int n_regs, int n_odds)
{
  int i, j;
  double *avg_ev = calloc(1, sizeof(double)*n_odds);
  for (j = 0; j < 3; j++)
    {
      
      for (i = 0; i < n_odds; i++)
	{
	  avg_ev[i] += odds[j*n_odds +i]*ev[j*n_odds +i];
	}
    }
  for (j = 0; j < 3; j++)
    {
      if (!valid_actions[j])
	continue;
      for (i = 0; i < n_odds; i++)
	{
	  if (mapping[i] == -1)
	    continue;
	  regs[j*n_regs + mapping[i]] += ev[j*n_odds +i] - avg_ev[i];
	}
    }
  
  free(avg_ev);
}

  
