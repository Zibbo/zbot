#ifndef HANDVAL_H
#define HANDVAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "poker-eval/poker_defs.h"

#include "poker-eval/inlines/eval.h"

#include "defs.h"
#include "worker.h"

int get_blo_index(int b[5]);
void precalc_cards();
int startup();
//void get_turn_slots(int flop, int turn, short slots[HANDS]);
int get_flop_morph_count(int flop);

void calc_flop_slots_all(struct gameinfo *g, short int *slots, int flop);

int get_river_wtl_single(int flop, int turn, int river, int hand, struct wtl *retval);

int get_river_slot_single(int flop, int turn, int river, int hand, int n_slots, double scale);
double get_river_raw_single(int flop, int turn, int river, int hand);
void get_river_raw_all(int flop, int turn, int river, double slots[HANDS]);
void get_river_slot_all(int flop, int turn, int river, int n_slots, double scale, int slots[HANDS]);
void get_river_handhv_all(int flop, int turn, int river, HandVal hvs[HANDS]);

void get_victory_odds_against_spread(double **hd, struct hand_hv *hv, int target_pos);
void get_victory_odds_against_spread_float(struct hwev_data **hd, struct hand_hv2 *hv, int target_pos);
void get_victory_odds_against_spread_float_no_dead_cards(struct hwev_data **hd, struct hand_hv2 *hv, int target_pos);

void get_river_wtl_all(int flop, int turn, int river, struct wtl retval[HANDS]);

void get_river_hand_hv2_all(int flop, int turn, int river, struct hand_hv2 retval[SAMPLES]);

void calc_turn_slots_all(struct gameinfo *g, short int *slots, int flop, int turn);
short int get_preflop_slot(struct gameinfo *g, int hand);
short int *get_preflop_slots(struct gameinfo *g);
short int get_flop_slot(struct gameinfo *g, int flop, int hand);
short int *get_flop_slots(struct gameinfo *g, int flop);
short int get_turn_slot(struct gameinfo *g, int flop, int turn, int hand);
short int *get_turn_slots(struct gameinfo *g, int flop, int turn);
short int get_river_slot(struct gameinfo *g, int flop, int turn, int river, int hand);
short int *get_river_slots(struct gameinfo *g, int flop, int turn, int river);
#endif
