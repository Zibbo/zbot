#ifndef PLRMODEL_H_
#define PLRMODEL_H_

#include <gsl/gsl_rng.h>
#include "defs.h"

void get_n_closest_local_type(struct plrmodel_node *n, int16_t global_type_i, int16_t *dest, int n_req);
int16_t get_closest_local_type(struct plrmodel_node *n, int16_t global_type_i);
void pmn_unlock_struct(struct plrmodel_node *n);
struct plrmodel_node *get_first_matching_situ(struct plrmodel_node *n, int16_t *st, double expand_odds, gsl_rng *rng, int lock);
uint32_t travel_tree_and_expand(struct plrmodel_node *n, double lo_limit, double hi_limit, uint64_t timestamp);
uint32_t reduce_tree(struct plrmodel_node *n, double lo_limit, double hi_limit, uint64_t timestamp);
uint32_t reduce_tree_percentage(struct plrmodel_node *n, double limit, double percentage, uint64_t timestamp);
int16_t copy_slot_to_type(struct plrmodel_node *n, int16_t type_i, int16_t slot_i);
uint32_t expand_tree_percentage(struct plrmodel_node *n, double limit, double percentage, uint64_t timestamp);
uint32_t expand_plrmodel_until_n_hands_random(struct plrmodel_node *n, double odds_multi);
uint32_t expand_test(struct plrmodel_node *n);
void reduce_tree_by_avg_odds_diff(struct plrmodel_node *n, double pmn_limit, double aod_limit, uint64_t timestamp);
//uint32_t expand_plrmodel_until_n_hands_random(struct plrmodel_node *n, uint32_t tot_hands, uint32_t target_hands);
int walk_all_situs_callback(struct plrmodel_node *root_n, int( *callback)(struct plrmodel_node*));
void *insert_data(void *d, void *item, int item_len, int n_items, int pos);
void *pop_data(void *d, void *item, int item_len, int n_items, int pos);
struct situ *copy_situ(struct plrmodel_node *n);
int count_pmn(struct plrmodel_node *n);
void get_tot_fitness(struct plrmodel_node *n, int *touch_count, double *tot_fitness, uint64_t timestamp);
struct plrmodel_node *copy_pmn(struct plrmodel_node *n, uint64_t timestamp);

void insert_pmn_to_next_list(struct plrmodel_node *d, struct plrmodel_node *n, int pos);
struct plrmodel_node *gen_pmn_based_on_type_type(struct type_type *t);
struct plrmodel_node *gen_pmn(struct gameinfo *info, struct plrmodel_node *n, struct unique_root *root, struct type_type *t);
void remove_pmn_from_pmn(struct plrmodel_node *n, int16_t slot_i);
int16_t add_pmn_to_pmn(struct plrmodel_node *n, struct plrmodel_node *new_n, int16_t slot_i);
int16_t get_random_unset_type(struct plrmodel_node *n);
int16_t remove_hand_from_pmn(struct plrmodel_node *n, int16_t slot_i);
void copy_situ_data(struct plrmodel_node *n, int16_t ts, int16_t ss);
int16_t add_empty_hand_to_situ(struct plrmodel_node *n, int16_t type_i);
int add_random_hand_to_situ(struct plrmodel_node *n);
struct plrmodel_node *gen_minimal_plrmodel_tree(struct gameinfo *info, struct unique_root *root, uint64_t timestamp);
void print_plrmodel_tree(struct plrmodel_node *n, int tabs);
void save_plrmodel_tree(int fileno, struct plrmodel_node *n);
struct plrmodel_node *load_plrmodel_tree(struct gameinfo *info, int fileno, struct unique_root *u);
void free_situ(void **s);
void free_plrmodel_tree(struct plrmodel_node *n);
short int choose_slot_from_normalized(float *data, int len);
int is_duplicate(int *d, int len);
void set_default_action_for_slot(struct plrmodel_node *n, int action, int slot_i);
void set_random_odds_for_slot(struct plrmodel_node *n, int slot_i);
void reset_hand_odds(struct plrmodel_node *n);
void randomize_hand_odds(struct plrmodel_node *n);
void pm_reset_ev(struct plrmodel_node *n);
void pm_reset_regs(struct plrmodel_node *n);
void pm_reset_d_regs(struct plrmodel_node *n);
void pm_reset_avg_odds(struct plrmodel_node *n);
void pm_reset_visits(struct plrmodel_node *n);

#endif
