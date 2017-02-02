#ifndef _TYPES_H_
#define _TYPES_H_

#include "defs.h"
void add_wtl_value_to_type(float *type, int wl_slots, int tie_slots, float wl_scale, float tie_scale, struct wtl *wtl_value);
void gen_all_preflop_types(struct gameinfo *info, float *types);
void gen_types_for_flop(struct gameinfo *info, float *types, int flop);
void gen_types_for_turn(struct gameinfo *info, float *types, int flop, int turn);
void gen_single_flop_type(float *type, int flop, int hand, int wl_slots, int tie_slots, float wl_scale, float tie_scale);
void gen_single_turn_type(struct gameinfo *info, float *type, int flop, int turn, int hand);
void gen_single_river_type(struct gameinfo *info, float *type, int flop, int turn, int river, int hand);
void gen_random_flop_type(float *type, int wl_slots, int tie_slots, float wl_scale, float tie_scale);
void gen_random_turn_type(struct gameinfo *info, float *type);
void gen_random_river_type(struct gameinfo *info, float *type);
void get_slots_for_river(struct gameinfo *info, short int *slots, int flop, int turn, int river);
short int get_slots_wtl_stats_for_river(struct gameinfo *info, struct wtl_f *wtl_stats, int flop, int turn, int river);
float gauss(float height, float center, float width, float x);
void degauss_type(float *type, int wl_slots, int tie_slots, float wl_width, float tie_width);
void degauss_type_with_diffs(float *type, float *diffs, short int *diffs_order, float width, int n_slots);
float try_new_type(float *types, float *diffs, float *t, int n_types, int n_slots_per_type, float smallest, int replace_i);
float get_difference(float *t1, float *t2, int n_slots);
float get_difference_pow2(float *t1, float *t2, int n_slots);
float calc_diffs(float *types, float *diffs, int n_types, int n_slots_per_type, int t);
void calc_diffs_for_one_type(float *types, float *type, float *diff, int n_types, int n_slots_per_type);
short int get_slot(float *types, float *t, int n_types, int n_slots_per_type);
void get_slots_for_all(short int *slots, float *slot_types, float *types, int n_slots, int n_types, int n_slots_per_type);
float get_closest_match(float *diffs, int n_types, int *t1, int *t2);
void generate_mapping_from_diffs(float *diffs, int *mapping, int n_types, int reduce_to);
void gen_board_type(struct gameinfo *info, float *bt, int gs, int flop, int turn, int river);
//float get_avg_diff(float *diff, int n_types);
//short int get_slot(float *types, float *t, int n_types, int n_slots_per_type);
void find_two_smallest(float *diffs, short int *diffs_order, int n_diff, int new_type);
void pair_and_minimize_diffs(float *diffs, short int *diffs_order, float *new_diffs, int n_types);
void add_values_to_gs_switch_and_odds(float *switch_table, float *odds_table, short int *hss, short int *hse, int types_end);

int get_bets_from_hand_info(struct hand_info *hi, int gamestate);
int get_last_act_from_hand_info(struct hand_info *hi, int gamestate);
struct unique_root *get_unique_state_from_hand_info(struct hand_info *hi, struct unique_root *root);
void add_action_to_hand_info(struct hand_info *hi, int action);


void add_action_to_hi(struct hand_info *hi, int action);
struct hand_info *get_blank_hi(struct unique_root *us);
int16_t get_potsize_slot_from_hi(struct type_type *t, struct hand_info *hi);
int16_t get_last_act_slot_from_hi(struct type_type *t, struct hand_info *hi);
int16_t get_bets_slot_from_hi(struct type_type *t, struct hand_info *hi);
int16_t get_board_slot_from_hi(struct type_type *t, struct hand_info *hi);
int16_t get_hand_slot_from_hi(struct type_type *t, struct hand_info *hi);


int16_t type_to_slot(uint64_t *types_bmap, int16_t type_i);
int16_t slot_to_type(uint64_t *types_bmap, int16_t slot_i);
int find_closest_type_diffs(float *diffs, uint64_t *types_bmap, int n_types, short int slot, int len);
int find_closest_type_diffs_order(short int *diffs_order, uint64_t *types_bmap, int n_types, short int slot);
int find_closest_type_linear_int(uint64_t *types_bmap, int n_types, short int slot);
int find_closest_type_linear_float(uint64_t *types_bmap, int n_types, float slot);
int find_closest_type(struct type_type *tt, uint64_t *types_bmap, int16_t slot, int len);
void find_n_closest_type(struct type_type *t, uint64_t *types_bmap, int16_t slot, int16_t *dest, int n_req);
int find_n_closest_type_valid_edge(struct type_type *t, uint64_t *types_bmap, int16_t slot, int16_t *dest, double *weights, int n_req);
int16_t find_unset_type_with_smallest_combined_diff(struct type_type *t, uint64_t *types_bmap, uint64_t *valid_bmap, int16_t *cmp_types, int n_cmp_types);
void generate_mapping_to_slots(uint64_t *types_bmap, int16_t *mapping, struct type_type *t, int len);
void generate_alt_mapping_to_slots(uint64_t *types_bmap, int16_t *mapping, struct type_type *t);
struct unique_root *set_mutable_types_from_path(int16_t *types, char *path, int path_len, struct unique_root *r);
int get_river_index(int8_t *board);


#endif
