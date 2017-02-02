#ifndef _PERF_MAIN_H_
#define _PERF_MAIN_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <poker_defs.h>
#include <inlines/eval.h>
#include <limits.h>
#include <stdint.h>
#include <gsl/gsl_rng.h>
//#include "defs.h"
#include "precalc_conversions.h"
//#include "handval.h"

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)


struct cards_1
{
  short int c1;
};


struct cards_2
{
  short int c1;
  short int c2;
};

struct cards_3
{
  short int c1;
  short int c2;
  short int c3;
};

struct cards_4
{
  short int c1;
  short int c2;
  short int c3;
  short int c4;
};

struct hand_hv
{
  HandVal hv;
  short int hand;
  unsigned char c1;
  unsigned char c2;
};

struct hand_hv2
{
  uint16_t hv;
  int16_t sample_i;
  int8_t c[4];
};

extern int cards_to_int_4[52][52][52][52];
extern int cards_to_int_3[52][52][52];
extern int cards_to_int_2[52][52];
extern int cards_to_int_1[52];
extern int cards_to_int_2_nosuit[52][52];

//struct cards_4 int_to_cards_4[270725];
extern struct cards_3 int_to_cards_3[22100];
extern struct cards_2 int_to_cards_2[1326];
extern struct cards_1 int_to_cards_1[52];
extern struct cards_2 int_to_cards_2_nosuit[1326];

extern CardMask int_to_cardmask_3[22100];
extern CardMask int_to_cardmask_2[1326];

#define HANDS 1326

#define REGS_DECAY 0x1
#define UPDATE_REGS 0x2
#define POV_ODDS_FROM_REGS 0x4
#define POV_ODDS_FROM_OPT 0x8
#define POV_EV_FROM_ODDS 0x10
#define POV_EV_FROM_OPT 0x20
#define NONPOV_ODDS_FROM_ODDS 0x40
#define NONPOV_ODDS_FROM_AVG_ODDS 0x80
#define NONPOV_ODDS_FROM_REGS 0x100
#define UPDATE_AVG_ODDS 0x200
#define NO_RIVER_ABS 0x400
#define FAST_SHOWDOWN_EV 0x800

struct path_nfo
{
  int8_t path_i;
  int8_t path[31];
  double potsize;
  double tocall;
  double toraise;
  double stake[2];
  int32_t gamestate;
  int32_t blen;
  int32_t seat;
  int32_t actions[3];
  int32_t bets;
  
};



struct node
{
  int8_t path_i;
  int8_t path[31];

  int32_t *sm;
  int32_t *n_slots;
  double **regs;
  double **odds;
  double **avg_odds;

  //int32_t path_i;
  //int32_t board_i;
  
  //int8_t board[5];
};

struct board_data
{
  int32_t *flop;
  int32_t *turn[52];
  int32_t *river[52][52];
  struct hand_hv2 *hv[52][52];
};

struct next_cards
{
  int *cards;
  double *multi;
  int len;
};


int compare_hv2(const void *m1, const void *m2);
struct hand_hv2 *get_river_hand_hv2(int8_t b[5]);
struct board_data *gen_board_data(int8_t *flop);
struct path_nfo *add_act_to_path_nfo(struct path_nfo *nfo, int32_t act);
int8_t *sort_flop(int8_t *board, int n_board);

int8_t *get_suits(int8_t *board, int n_board);

int8_t *get_suit_mapping(int8_t *suits, int n_board);

int get_max_suit(int8_t *suits, int n_board);

int8_t morph_card(int8_t c, int8_t *mapping);

int32_t *get_slot_mapping(int8_t *board, int n_board);
struct node *gen_node(int8_t *path, int8_t path_i, int gamestate);
double *get_slot_odds(double *slot_odds, double *src, int32_t *valid_actions, int n_slots, gsl_rng *rng);

#endif
