#ifndef _WORKER_H_
#define _WORKER_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <gsl/gsl_rng.h>

#include "defs.h"
#include "queue.h"

struct worker_thread_args
{
  struct zp_queue *in_queue;
  struct zp_queue *out_queue;
  struct unique_root *us;
  struct zbtree *us_tree;
  struct zbtree *wm_tree;
  struct zbtree *td_tree;
  int cont;
};

struct receiver_thread_args
{
  struct zp_queue *out_queue;
  struct zbtree *td_tree;
  char *listen_address;
  int listen_port;
  int cont;
};

struct recv_child_thread_args
{
  struct zp_queue *out_queue;
  struct zbtree *td_tree;
  struct sockaddr_in addr;
  socklen_t addrlen;
  pthread_t thread_id;
  int s;
  int cont;
};

struct sender_thread_args
{
  struct zbtree *us_tree;
  struct zp_queue *in_queue;
  struct zp_queue *loopback_queue;
  int cont;
};

struct sender_child_thread_args
{
  struct zp_queue *in_queue;
  struct sockaddr_in *us_addr;
  int s;
};

struct thread_control_args
{
  struct worker_thread_args *w_args;
  struct receiver_thread_args *r_args;
  struct sender_thread_args *s_args;
  struct zp_queue *receiver_to_worker_queue;
  struct zp_queue *worker_to_sender_queue;
  //struct zp_queue *in_queue;
  struct zbtree *us_tree;
  struct zbtree *td_tree;
  struct zbtree *wm_tree;
  struct unique_root *root_us;
  pthread_t *worker_thread_id;
  pthread_t sender_thread_id;
  pthread_t receiver_thread_id;
  int32_t n_worker_thread;
};

struct type_count
{
  int32_t type;
  int32_t count;
  int16_t *mapping;
};

struct types_data
{
  double timestamp;
  int32_t id;
  int16_t public_types[N_PUB_TYPES];
  int16_t private_types[N_PRIV_TYPES][SAMPLES];
  struct hand_hv2 vals[SAMPLES];
  struct type_count *tc[N_PRIV_TYPES];
  int32_t n_types[N_PRIV_TYPES];
};


struct variation
{
  //struct variation *next;
  double path_weight;
  double potsize;
  int us_id;
  int path_i;
  char path[MAX_PATH_LEN];
  //float *hwev;
};

struct hwev_data
{
  int32_t ref_count;
  int32_t padding;
  double d[SAMPLES];
  double pw; 
};

struct worker_message
{
  int8_t direction;
  int8_t action;
  int8_t pov_seat;
  //int16_t n_hwev;
  int8_t n_variations;
  int32_t types_data_id;
  double positive_regs;
  uint64_t flags;
  struct types_data *td;  
  struct hwev_data *hwev[MAX_HWEV];
  struct variation *v[MAX_VARIATION];
  
  //struct variation *v;
};

/* struct net_message */
/* { */
/*   int direction; */
/*   int action; */
/*   int pov_seat; */
/*   int16_t n_hwev; */
/*   int16_t n_variations; */
/*      //  double pw[MAX_PLR]; */
/*   // types data *1 */
/*   // float hwev[SAMPLES]*n_hwev */
/*   // struct variation *n_variations (aina 1) */

/*   /\* struct worker_message wm; *\/ */
/*   /\* struct types_data td; *\/ */
/*   /\* float hwev[SAMPLES]; *\/ */
/*   /\* struct variation v; *\/ */
/* }; */

void set_handler_addr(struct unique_root *us, char *addr, uint16_t port);
void clean_old_types_data(struct thread_control_args *args, double timelimit);
void recv_new_types_data(struct thread_control_args *args, int fd);
void *worker_thread(void *thread_args);
void *sender_thread(void *thread_args);
void *receiver_thread(void *thread_args);
void generate_mapping_from_global_to_local(struct plrmodel_node *n);

int generate_next_variations_up(struct unique_root *us, struct variation *old_v, struct variation *var_list[MAX_VARIATION], int var_i, int path_i, int pov_seat, float potsize, double path_weight);
void generate_next_variation_down(struct unique_root *us, struct worker_message *msg);
float **get_odds_from_double(double **src, int n_acts, int n_items, gsl_rng *rng);
float **get_odds_from_float(float **src, int n_acts, int n_items);
void update_msg_with_ev(struct worker_message *msg, struct unique_root *us);
void update_msg_up(struct worker_message *msg, struct unique_root *us, gsl_rng *rng);
struct worker_message *update_msg_down(struct worker_message *msg, struct unique_root *root_us,gsl_rng *rng);
void lock_plrmodel_node_read(uint32_t *lock);
void unlock_plrmodel_node_read(uint32_t *lock);
void lock_plrmodel_node_write(uint32_t *lock);
void unlock_plrmodel_node_write(uint32_t *lock);

void lock_plrmodel_node(struct plrmodel_node *n);
void unlock_plrmodel_node(struct plrmodel_node *n);



#endif
