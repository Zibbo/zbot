#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <poker_defs.h>
#include <inlines/eval.h>

#include "defs.h"
#include "precalc_conversions.h"
#include "handval.h"
#include "types.h"
#include "zconv.h"

/* CardMask dead_cards, dead_cards2; */
/* CardMask cards, cards2, cards3; */
/* CardMask c1, c2; */
/* HandVal h1, h2; */
/* CardMask board2, board3;  */

/* int i = 0; */

//double flop_precalcs[FLOPS*HANDS*2];
//double preflop_precalcs[HANDS*2];

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

double top_w = 0, top_pp = 0, top_np = 0, bot_w = 1, bot_pp = 1, bot_np = 1;
CardMask hand_pp, hand_np, hand_w, board_pp, board_np, board_w;



int get_flop_morph_count(int flop)
{
  if (int_to_cards_3[flop].c1/13 == int_to_cards_3[flop].c2/13 && int_to_cards_3[flop].c1/13 == int_to_cards_3[flop].c3/13)
    return 4;
  else if (int_to_cards_3[flop].c1/13 != int_to_cards_3[flop].c2/13 && int_to_cards_3[flop].c1/13 != int_to_cards_3[flop].c3/13 && int_to_cards_3[flop].c2/13 != int_to_cards_3[flop].c3/13)
    {
      if (int_to_cards_3[flop].c1%13 == int_to_cards_3[flop].c2%13 && int_to_cards_3[flop].c1%13 == int_to_cards_3[flop].c3%13 && int_to_cards_3[flop].c2%13 == int_to_cards_3[flop].c3%13)
	return 4;
      if (int_to_cards_3[flop].c1%13 != int_to_cards_3[flop].c2%13 && int_to_cards_3[flop].c1%13 != int_to_cards_3[flop].c3%13 && int_to_cards_3[flop].c2%13 != int_to_cards_3[flop].c3%13)
	return 24;
      return 12;
    }
  return 12;
}


int get_flop_morph_type(int flop)
{
  if (int_to_cards_3[flop].c1/13 == int_to_cards_3[flop].c2/13 && int_to_cards_3[flop].c1/13 == int_to_cards_3[flop].c3/13)
    return cards_to_int_3[int_to_cards_3[flop].c1%13][int_to_cards_3[flop].c2%13][int_to_cards_3[flop].c3%13];
  else if (int_to_cards_3[flop].c1/13 != int_to_cards_3[flop].c2/13 && int_to_cards_3[flop].c1/13 != int_to_cards_3[flop].c3/13 && int_to_cards_3[flop].c2/13 != int_to_cards_3[flop].c3/13)
    {
      int c[3];
      if (int_to_cards_3[flop].c1%13 > int_to_cards_3[flop].c2%13)
	{
	  c[0] = int_to_cards_3[flop].c1%13;
	  c[1] = int_to_cards_3[flop].c2%13;
	}
      else
	{
	  c[0] = int_to_cards_3[flop].c2%13;
	  c[1] = int_to_cards_3[flop].c1%13;
	}
      if (int_to_cards_3[flop].c3%13 > c[1])
	{
	  if (int_to_cards_3[flop].c3%13 > c[0])
	    {
	      c[2] = c[1];
	      c[1] = c[0];
	      c[0] = int_to_cards_3[flop].c3%13;
	    }
	  else
	    {
	      c[2] = c[1];
	      c[1] = int_to_cards_3[flop].c3%13;
	    }
	}
      else
	c[2] = int_to_cards_3[flop].c3%13;
      return cards_to_int_3[c[0]][c[1]+13][c[2]+26];
    }
  else
    {
      if (int_to_cards_3[flop].c1/13 == int_to_cards_3[flop].c2/13)
	return cards_to_int_3[int_to_cards_3[flop].c1%13][int_to_cards_3[flop].c2%13][int_to_cards_3[flop].c3%13+13];
      else if (int_to_cards_3[flop].c1/13 == int_to_cards_3[flop].c3/13)
	return cards_to_int_3[int_to_cards_3[flop].c1%13][int_to_cards_3[flop].c3%13][int_to_cards_3[flop].c2%13+13];
      else 
	return cards_to_int_3[int_to_cards_3[flop].c2%13][int_to_cards_3[flop].c3%13][int_to_cards_3[flop].c1%13+13];
      
    }
}


/*int compare_board(const void *m1, const void *m2)
{
  if (*(int*)m1 > *(int*)m2)
    return -1;
  if (*(int*)m1 < *(int*)m2)
    return 1;
  return 0;
}



short int get_preflop_slot(struct gameinfo *g, int hand)
{
  return g->slots[GS_PF][hand];
}

short int *get_preflop_slots(struct gameinfo *g)
{
  printf("%i\n", g->slots[GS_PF]);
  return g->slots[GS_PF];
}

short int get_flop_slot(struct gameinfo *g, int flop, int hand)
{
  return g->slots[GS_F][flop*HANDS + hand];
}

short int *get_flop_slots(struct gameinfo *g, int flop)
{
  //printf("%i %i %i %i\n", g->slots[GS_F][flop][100], g->slots[GS_F][flop][200],g->slots[GS_F][flop][300],g->slots[GS_F][flop][400]);
  return &(g->slots[GS_F][flop*HANDS]);
}



short int get_turn_slot(struct gameinfo *g, int flop, int turn, int hand)
{
  return g->slots[GS_T][cards_to_int_4[int_to_cards_3[flop].c1][int_to_cards_3[flop].c2][int_to_cards_3[flop].c3][turn]*HANDS + hand];
}

short int *get_turn_slots(struct gameinfo *g, int flop, int turn)
{
  return &(g->slots[GS_T][cards_to_int_4[int_to_cards_3[flop].c1][int_to_cards_3[flop].c2][int_to_cards_3[flop].c3][turn]*HANDS]);
}

short int get_river_slot(struct gameinfo *g, int flop, int turn, int river, int hand)
{
  static struct wtl tmp_wtl;
  static int tot, retval;
  tot = get_river_wtl_single(flop, turn, river, hand, &tmp_wtl);
  retval = (int)(pow(((double)tmp_wtl.w + (double)tmp_wtl.t/2.0)/(double)tot, g->scale)*(double)(g->n_types[3]-1));
  return retval;
}

short int *get_river_slots(struct gameinfo *g, int flop, int turn, int river)
{
  static struct wtl tmp_wtl[HANDS];
  static short int slots[HANDS];
  static int tot, retval;

  get_river_wtl_all(flop, turn, river, tmp_wtl);
  for (i = 0; i < HANDS; i++)
    {
      if (tmp_wtl[i].w == -1)
	{
	  slots[i] = -1;
	  continue;
	}
      tot = tmp_wtl[i].w + tmp_wtl[i].t + tmp_wtl[i].l;
      retval = (int)(pow(((double)tmp_wtl[i].w + (double)tmp_wtl[i].t)/(double)tot, g->scale)*(double)(g->n_types[3]-1));
      slots[i] = retval;
    }    
  return slots;
}




void calc_flop_slots_all(struct gameinfo *g, short int *slots, int flop)
{
  double *new_types;
  double *types;
  int i;


  //printf("%x %x %x %x %i %i %i %i %i %i %i %f\n", g->types[0],g->types[1],g->types[2],g->types[3], g->n_types[0], g->n_types[1], g->n_types[2], g->n_types[3], g->slots[GS_PF], g->slots[GS_F], g->slots[GS_T], g->scale);

  types = g->types[1];  
  new_types = (double*) malloc(sizeof(double)*g->n_types[3]*HANDS);
  memset(new_types, 0, sizeof(double)*g->n_types[3]*HANDS);

  gen_all_flop_type(g, new_types, flop);
  
  for (i = 0;i < HANDS;i++)
    {
      if (CardMask_ANY_SET(int_to_cardmask_3[flop], int_to_cardmask_2[i]))
	{
	  slots[i] = -1;
	  continue;
	}      
      slots[i] = get_slot(types, &(new_types[i*g->n_types[3]]), g->n_types[1], g->n_types[3]);       
    }
  free(new_types);
    
}

void calc_turn_slots_all(struct gameinfo *g, short int *slots, int flop, int turn)
{
  double *new_types;
  double *types;
  CardMask dead_cards;
  int i;
 
  //printf("%x %x %x %x %i %i %i %i %i %i %i %f\n", g->preturn_types,g->types[2],g->types[2],g->types[3], g->n_preturn_types, g->n_types[2], g->n_types[2], g->n_types[3], g->preturn_slots, g->slots[GS_T], g->slots[GS_T], g->scale);
  types = g->types[2];
  
  new_types = (double*) malloc(sizeof(double)*g->n_types[3]*HANDS);
  memset(new_types, 0, sizeof(double)*g->n_types[3]*HANDS);
  dead_cards = int_to_cardmask_3[flop];
  CardMask_SET(dead_cards, turn);

  gen_all_turn_type(g, new_types, flop,turn);
  
  for (i = 0;i < HANDS;i++)
    {
      if (CardMask_ANY_SET(dead_cards, int_to_cardmask_2[i]))
	{
	  slots[i] = -1;
	  continue;
	}      
      slots[i] = get_slot(types, &(new_types[i*g->n_types[3]]), g->n_types[2], g->n_types[3]);
      
    }
  free(new_types);
    
}
*/
int get_river_wtl_single(int flop, int turn, int river, int hand, struct wtl *retval)
{
  int i, total = 0;

  //double total = 0;
  HandVal hv1, hv2;
  CardMask b, h1, h2, hand1, hand2;

  //printf("wtl single args: %i %i %i %i\n", flop, turn, river, hand);
  retval->w = 0;
  retval->l = 0;
  retval->t = 0;

  b = int_to_cardmask_3[flop];
  CardMask_SET(b, turn);
  CardMask_SET(b, river);
  h1 = int_to_cardmask_2[hand];
  if (CardMask_ANY_SET(b, h1))
    return -1;


  CardMask_OR(hand1, h1,b);
  hv1 = Hand_EVAL_N(hand1, 7);
  for (i = 0;i < HANDS;i++)
    {
      h2 = int_to_cardmask_2[i];
      if (CardMask_ANY_SET(hand1, h2))
        continue;

      CardMask_OR(hand2, h2, b);
      
      hv2 = Hand_EVAL_N(hand2, 7);
      if (hv1 > hv2)
	retval->w++;
      else if (hv1 < hv2)
	retval->l++;
      else
        retval->t++;
      total++;

    }
  if (total != 990)
    printf("ALAMOLO %i %i %i %i %i\n", total, flop, turn, river, hand);
  return total;
}

/* /\*int get_river_slot_single(int flop, int turn, int river, int hand, int n_slots, double scale) */
/* { */
/*   static struct wtl tmp_wtl; */
/*   static int tot, retval; */
/*   //static double tmpf1; */


/*   tot = get_river_wtl_single(flop, turn, river, hand, &tmp_wtl); */
/*   //tmpf1 = (double)wtl.w/(double)wtl.l; */
/*   retval = (int)(pow(((double)tmp_wtl.w + (double)tmp_wtl.t/2.0)/(double)tot, scale)*(double)(n_slots-1)); */
/*   if (retval < 0 || retval >= n_slots) */
/*     printf("retval kusee: %i %i %i %i %f %i\n", retval, tmp_wtl.w, tmp_wtl.t, tot, scale, n_slots); */
/*   return retval; */
/* } */
  
/* double get_river_raw_single(int flop, int turn, int river, int hand) */
/* { */
/*   static struct wtl tmp_wtl; */
/*   static int tot; */
/*   //static double tmpf1; */
/*   static double retval; */

/*   tot = get_river_wtl_single(flop, turn, river, hand, &tmp_wtl); */
/*   //printf("raw_single: %i %i %i %i\n", tmp_wtl.w, tmp_wtl.t, tmp_wtl.l, tot); */
/*   retval = ((double)tmp_wtl.w + ((double)tmp_wtl.t)/2.0)/(double)tot; */
  
/*   return retval; */
/* } */
/* *\/ */
int compare_hv(const void *m1, const void *m2) 
{
  if (((struct hand_hv *)m1)->hv > ((struct hand_hv *)m2)->hv)
    return -1;
 if (((struct hand_hv *)m1)->hv < ((struct hand_hv *)m2)->hv)
    return 1;
 return 0;
}

int compare_hv2(const void *m1, const void *m2) 
{
  return ((struct hand_hv *)m1)->hv - ((struct hand_hv *)m2)->hv;
}

/*
void get_river_raw_all(int flop, int turn, int river, double slots[HANDS])
{
  static struct wtl tmp_wtl[HANDS];
  static int tot;
  double retval;
  //static double tmpf1;

  get_river_wtl_all(flop, turn, river, tmp_wtl);
  for (i = 0; i < HANDS; i++)
    {
      //tmpf1 = (double)wtl.w/(double)wtl.l;
      if (tmp_wtl[i].w == -1)
	{
	  slots[i] = -1;
	  continue;
	}
      tot = tmp_wtl[i].w + tmp_wtl[i].t + tmp_wtl[i].l;
      retval = ((double)tmp_wtl[i].w + ((double)tmp_wtl[i].t)/2.0)/(double)tot;
      slots[i] = retval;
    }      
}
  

void get_river_slot_all(int flop, int turn, int river, int n_slots, double scale, int slots[HANDS])
{
  struct wtl tmp_wtl[HANDS];
  int tot, retval;
  //static double tmpf1;

  get_river_wtl_all(flop, turn, river, tmp_wtl);
  for (i = 0; i < HANDS; i++)
    {
      //tmpf1 = (double)wtl.w/(double)wtl.l;
      if (tmp_wtl[i].w == -1)
	{
	  slots[i] = -1;
	  continue;
	}
      tot = tmp_wtl[i].w + tmp_wtl[i].t + tmp_wtl[i].l;
      retval = (int)(pow(((double)tmp_wtl[i].w + (double)tmp_wtl[i].t)/(double)tot, scale)*(double)(n_slots-1));
      if (retval < -1 || retval >= n_slots)
	{
	  printf("retval kusee: %i %i %i %i %f %i\n", retval, tmp_wtl[i].w, tmp_wtl[i].t, tot, scale, n_slots);
	  continue;
	}

      slots[i] = retval;
    }      
}
*/  

void get_river_handhv_all(int flop, int turn, int river, HandVal hvs[HANDS])
{

  CardMask board, hand, hand1;
  int i;
  
  
  board = int_to_cardmask_3[flop];
  CardMask_SET(board, turn);
  CardMask_SET(board, river);

  for (i = 0; i < HANDS;i++)
    {
      hand = int_to_cardmask_2[i];
      if (CardMask_ANY_SET(board, hand))
	{
	  hvs[i] = 0;
	  continue;
	}
      CardMask_OR(hand1, hand, board);
      hvs[i] = Hand_EVAL_N(hand1, 7);
    }
}

/* void get_victory_odds_against_spread(double **hd, struct hand_hv *hv, int target_pos) */
/* { */

/*   //HAX 2PLR KOKO FUNKTIO */
/*   int i,j, prev_hv_change = 0; */
/*   double x, curhv_n, tot_hw; */
/*   //double w,t; */
/*   HandVal curhv; */
/*   double cardcount[52]; */
/*   double cardcount_curhv[52]; */
  

/*   double *odds = hd[target_pos]; */
/*   double *hw = hd[(target_pos+1)%2]; */
/*   tot_hw = hw[HANDS]; */
/*   memset(odds, 0x22, sizeof(double)*(HANDS+1)); */
/*   memset(cardcount, 0, sizeof(cardcount)); */
/*   memset(cardcount_curhv, 0, sizeof(cardcount_curhv)); */
/*   curhv_n = 0; */
/*   curhv = -1; */
/*   x = 0; */
/*   for (i = 0; i < HANDS ;i++) */
/*     { */
/*       if (hv[i].c1 == 0xff) */
/* 	{ */
/* 	  odds[hv[i].hand] = 0; */
/* 	  tot_hw -= hw[hv[i].hand]; */
/* 	  continue; */
/* 	} */
/*       odds[hv[i].hand] = x - cardcount[hv[i].c1] - cardcount[hv[i].c2]; */
/*       if (hv[i].hv != curhv) */
/* 	{ */
/* 	  odds[hv[i].hand] += curhv_n - cardcount_curhv[hv[i].c1] - cardcount_curhv[hv[i].c2]; */
/* 	  for (j = prev_hv_change; j < i;j++) */
/* 	    odds[hv[j].hand] += (curhv_n - cardcount_curhv[hv[j].c1] - cardcount_curhv[hv[j].c2] + hw[hv[j].hand])/2.0; */
/* 	  prev_hv_change = i; */

/* 	  for (j = 0; j < 52; j++) */
/* 	    cardcount[j] += cardcount_curhv[j]; */

/* 	  x+=curhv_n; */

/* 	  memset(cardcount_curhv, 0, sizeof(cardcount_curhv));	   */
/* 	  cardcount_curhv[hv[i].c1] = hw[hv[i].hand]; */
/* 	  cardcount_curhv[hv[i].c2] = hw[hv[i].hand]; */
/* 	  curhv_n = hw[hv[i].hand]; */
/* 	  curhv = hv[i].hv; */
/* 	} */
/*       else */
/* 	{ */
/* 	  cardcount_curhv[hv[i].c1]+=hw[hv[i].hand]; */
/* 	  cardcount_curhv[hv[i].c2]+=hw[hv[i].hand]; */
/* 	  curhv_n+=hw[hv[i].hand]; */
/* 	} */
      
/*     } */
/*   for (j = prev_hv_change; j < i;j++) */
/*     odds[hv[j].hand] += (curhv_n - cardcount_curhv[hv[j].c1] - cardcount_curhv[hv[j].c2] + hw[hv[j].hand])/2.0; */
/*   for (j = 0; j < 52; j++) */
/*     cardcount[j] += cardcount_curhv[j]; */

/*   for (i = 0; i < HANDS;i++) */
/*     { */
/*       //cardcount[int_to_cards_2[i].c1] += cardcount_curhv[int_to_cards_2[i].c1]; */
/*       //cardcount[int_to_cards_2[i].c2] += cardcount_curhv[int_to_cards_2[i].c2];	   */
/*       odds[i] /= (tot_hw - cardcount[int_to_cards_2[i].c1] - cardcount[int_to_cards_2[i].c2] + hw[i]); */
/*     } */
/* } */

void get_victory_odds_against_spread(double **hd, struct hand_hv *hv, int target_pos)
{

  //HAX 2PLR KOKO FUNKTIO
  int i,j, prev_hv_change = 0;
  
  double curhv_hw = 0, tot_hw = 0, tmp_hw = 0;
  //double w,t;
  HandVal curhv;
  double cardcount[52];
  double cardcount_curhv[52];
  

  double *odds = hd[target_pos];//2PLR
  double *hw = hd[(target_pos+1)%2];
  //tot_hw = hw[HANDS];
  memset(odds, 0x22, sizeof(double)*(HANDS));
  memset(cardcount, 0, sizeof(cardcount));
  memset(cardcount_curhv, 0, sizeof(cardcount_curhv));
  
  curhv = -1;
  for (i = 0; i < HANDS ;i++)
    {
      if (hv[i].c1 == 0xff)
	{
	  odds[hv[i].hand] = 0;
	  //tot_hw -= hw[hv[i].hand];
	  prev_hv_change = i+1;	  
	  continue;
	}
      //odds[hv[i].hand] = x - cardcount[hv[i].c1] - cardcount[hv[i].c2];
      if (hv[i].hv != curhv)
	{
	  //odds[hv[i].hand] += curhv_n - cardcount_curhv[hv[i].c1] - cardcount_curhv[hv[i].c2];
	  for (j = prev_hv_change; j < i;j++)
	    {
	      odds[hv[j].hand] = tot_hw - cardcount[hv[j].c1] - cardcount[hv[j].c2];
	      odds[hv[j].hand] += (curhv_hw - cardcount_curhv[hv[j].c1] - cardcount_curhv[hv[j].c2] + hw[hv[j].hand])/2.0;
	    }
	  prev_hv_change = i;

	  for (j = 0; j < 52; j++)
	    cardcount[j] += cardcount_curhv[j];

	  tot_hw+=curhv_hw;

	  memset(cardcount_curhv, 0, sizeof(cardcount_curhv));	  
	  cardcount_curhv[hv[i].c1] = hw[hv[i].hand];
	  cardcount_curhv[hv[i].c2] = hw[hv[i].hand];
	  curhv_hw = hw[hv[i].hand];
	  curhv = hv[i].hv;
	}
      else
	{
	  cardcount_curhv[hv[i].c1]+=hw[hv[i].hand];
	  cardcount_curhv[hv[i].c2]+=hw[hv[i].hand];
	  curhv_hw+=hw[hv[i].hand];
	}
      
    }
  
  for (j = prev_hv_change; j < i;j++)
    {
      odds[hv[j].hand] = tot_hw - cardcount[hv[j].c1] - cardcount[hv[j].c2];
      odds[hv[j].hand] += (curhv_hw - cardcount_curhv[hv[j].c1] - cardcount_curhv[hv[j].c2] + hw[hv[j].hand])/2.0;
    }
  /* prev_hv_change = i; */
  
  for (j = 0; j < 52; j++)
    cardcount[j] += cardcount_curhv[j];
  
  tot_hw+=curhv_hw;
  
  /* memset(cardcount_curhv, 0, sizeof(cardcount_curhv));	   */
  /* cardcount_curhv[hv[i].c1] = hw[hv[i].hand]; */
  /* cardcount_curhv[hv[i].c2] = hw[hv[i].hand]; */
  /* curhv_hw = hw[hv[i].hand]; */
  /* curhv = hv[i].hv; */

  for (i = 0; i < HANDS;i++)
    {
      if ((tmp_hw = (tot_hw - cardcount[int_to_cards_2[i].c1] - cardcount[int_to_cards_2[i].c2] + hw[i])) != 0)
	odds[i] /= (tot_hw - cardcount[int_to_cards_2[i].c1] - cardcount[int_to_cards_2[i].c2] + hw[i]);
      else
	odds[i] = 0;
    }
}

void get_victory_odds_against_spread_float(struct hwev_data **hd, struct hand_hv2 *hv, int target_pos)
{

  //HAX 2PLR KOKO FUNKTIO
  int i,j, prev_hv_change = 0;
  
  double curhv_hw = 0, tot_hw = 0, tmp_hw = 0;
  //double w,t;
  HandVal curhv;
  double cardcount[52];
  double cardcount_curhv[52];
  

  double *odds = hd[target_pos]->d;//2PLR
  double *hw = hd[(target_pos+1)%2]->d;
  //tot_hw = hw[HANDS];
  //memset(odds, 0x22, sizeof(float)*(SAMPLES));
  memset(cardcount, 0, sizeof(cardcount));
  memset(cardcount_curhv, 0, sizeof(cardcount_curhv));
  //rnd = zrand();
  curhv = -1;
  for (i = 0; i < SAMPLES ;i++)
    {
      if (hv[i].c[0] == -1)
	{
	  printf("NEVERHETERERERER\n");
	  exit(0);
	  odds[hv[i].sample_i] = 0;
	  //tot_hw -= hw[hv[i].sample_i];
	  prev_hv_change = i+1;	  
	  continue;
	}
      //odds[hv[i].sample_i] = x - cardcount[hv[i].c[0] - cardcount[hv[i].c[1];
      if (hv[i].hv != curhv)
	{
	  //odds[hv[i].sample_i] += curhv_n - cardcount_curhv[hv[i].c[0] - cardcount_curhv[hv[i].c[1];
	  for (j = prev_hv_change; j < i;j++)
	    {
	      odds[hv[j].sample_i] = tot_hw - cardcount[hv[j].c[0]] - cardcount[hv[j].c[1]];
	      odds[hv[j].sample_i] += (curhv_hw - cardcount_curhv[hv[j].c[0]] + hw[hv[j].sample_i] - cardcount_curhv[hv[j].c[1]])/2.0;
	      assert(odds[hv[j].sample_i] > -1e-12);
	      if (odds[hv[j].sample_i] < 1e-12)
		odds[hv[j].sample_i] = 0;
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
      odds[hv[j].sample_i] = tot_hw - cardcount[hv[j].c[0]] - cardcount[hv[j].c[1]];
      odds[hv[j].sample_i] += (curhv_hw - cardcount_curhv[hv[j].c[0]] + hw[hv[j].sample_i] - cardcount_curhv[hv[j].c[1]])/2.0;
      assert(odds[hv[j].sample_i] > -1e-12);
      if (odds[hv[j].sample_i] < 1e-12)
	odds[hv[j].sample_i] = 0;
      
    }
  /* prev_hv_change = i; */
  
  for (j = 0; j < 52; j++)
    cardcount[j] += cardcount_curhv[j];
  
  tot_hw+=curhv_hw;
  

  for (i = 0; i < SAMPLES;i++)
    {
      if ((tmp_hw = (tot_hw - cardcount[hv[i].c[0]] + hw[hv[i].sample_i] - cardcount[hv[i].c[1]])) > 1e-12)
	{
	  odds[hv[i].sample_i] /= tmp_hw;
	  //odds[hv[i].sample_i] /= (tot_hw - cardcount[hv[i].c[0]] - cardcount[hv[i].c[1]] + hw[hv[i].sample_i]);
	}
      else
	{
	  //printf("get odds: %e\n", tmp_hw);
	  //kaveri voi olla taalla ainoastaan korteilla jotka tama kasi blockaa -> voitto???
	  assert(tmp_hw > -1e-12);
	  odds[hv[i].sample_i] = -1000000000.0;
	  //odds[hv[i].sample_i] = 0;
	}
      assert(odds[hv[i].sample_i] < 1.001);
    }
}

void get_victory_odds_against_spread_pcs(double *hw, double *odds, struct hand_hv2 *hv)
{
  //HAX 2PLR KOKO FUNKTIO
  int i,j, prev_hv_change = 0;
  
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
	      odds[hv[j].sample_i] = tot_hw - cardcount[hv[j].c[0]] - cardcount[hv[j].c[1]];
	      odds[hv[j].sample_i] += (curhv_hw - cardcount_curhv[hv[j].c[0]] + hw[hv[j].sample_i] - cardcount_curhv[hv[j].c[1]])/2.0;
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
      odds[hv[j].sample_i] = tot_hw - cardcount[hv[j].c[0]] - cardcount[hv[j].c[1]];
      odds[hv[j].sample_i] += (curhv_hw - cardcount_curhv[hv[j].c[0]] + hw[hv[j].sample_i] - cardcount_curhv[hv[j].c[1]])/2.0;
    }
  /* prev_hv_change = i; */
  
  /* for (j = 0; j < 52; j++) */
  /*   cardcount[j] += cardcount_curhv[j]; */
  
  /* tot_hw+=curhv_hw; */
  

  /* for (i = 0; i < SAMPLES;i++) */
  /*   { */
  /*     if ((tmp_hw = (tot_hw - cardcount[hv[i].c[0]] + hw[hv[i].sample_i] - cardcount[hv[i].c[1]])) > 1e-12) */
  /* 	{ */
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


void get_victory_odds_against_spread_float_no_dead_cards(struct hwev_data **hd, struct hand_hv2 *hv, int target_pos)
{

  //HAX 2PLR KOKO FUNKTIO
  int i,j, prev_hv_change = 0;
  
  double curhv_hw = 0, tot_hw = 0;
  //double w,t;
  HandVal curhv;
  double cardcount[52];
  double cardcount_curhv[52];
  

  double *odds = hd[target_pos]->d;//2PLR
  double *hw = hd[(target_pos+1)%2]->d;
  //tot_hw = hw[HANDS];
  //memset(odds, 0x22, sizeof(float)*(SAMPLES));
  memset(cardcount, 0, sizeof(cardcount));
  memset(cardcount_curhv, 0, sizeof(cardcount_curhv));
  
  curhv = -1;
  for (i = 0; i < SAMPLES ;i++)
    {
      if (hv[i].c[0] == -1)
	{
	  printf("NEVERHETERERERER\n");
	  exit(0);
	  odds[hv[i].sample_i] = 0;
	  //tot_hw -= hw[hv[i].sample_i];
	  prev_hv_change = i+1;	  
	  continue;
	}
      //odds[hv[i].sample_i] = x - cardcount[hv[i].c[0] - cardcount[hv[i].c[1];
      if (hv[i].hv != curhv)
	{
	  //odds[hv[i].sample_i] += curhv_n - cardcount_curhv[hv[i].c[0] - cardcount_curhv[hv[i].c[1];
	  for (j = prev_hv_change; j < i;j++)
	    {
	      odds[hv[j].sample_i] = tot_hw;
	      odds[hv[j].sample_i] += curhv_hw/2.0;

	      assert(odds[hv[j].sample_i] > -1e-12);
	      if (odds[hv[j].sample_i] < 1e-12)
		odds[hv[j].sample_i] = 0;
	    }
	  prev_hv_change = i;

	  tot_hw+=curhv_hw;

	  curhv_hw = hw[hv[i].sample_i];
	  curhv = hv[i].hv;
	}
      else
	{
	  curhv_hw+=hw[hv[i].sample_i];
	}
      
    }
  
  for (j = prev_hv_change; j < i;j++)
    {
      odds[hv[j].sample_i] = tot_hw ;
      odds[hv[j].sample_i] += curhv_hw/2.0;
      assert(odds[hv[j].sample_i] > -1e-12);
      if (odds[hv[j].sample_i] < 1e-12)
	odds[hv[j].sample_i] = 0;
      
    }
  /* prev_hv_change = i; */
    
  tot_hw+=curhv_hw;
  

  for (i = 0; i < SAMPLES;i++)
    {
      if (tot_hw > 1e-12)
	odds[hv[i].sample_i] /= tot_hw;
      //odds[hv[i].sample_i] /= (tot_hw - cardcount[hv[i].c[0]] - cardcount[hv[i].c[1]] + hw[hv[i].sample_i]);
      else
	{
	  //printf("get odds: %e\n", tmp_hw);
	  //kaveri voi olla taalla ainoastaan korteilla jotka tama kasi blockaa -> voitto???
	  assert(0);
	  assert(tot_hw > -1e-12);
	  odds[hv[i].sample_i] = 1.0;
	}
      assert(odds[hv[i].sample_i] < 1.001);
    }
}


void get_victory_odds_against_spread_float_hard_way(struct hwev_data **hd, struct hand_hv2 *hv, int target_pos)
{

  //HAX 2PLR KOKO FUNKTIO
  int i,j;
  
  double tot_hw = 0;
  //double w,t;
  HandVal curhv;
  

  double *odds = hd[target_pos]->d;//2PLR
  double *hw = hd[(target_pos+1)%2]->d;
  //tot_hw = hw[HANDS];
  memset(odds, 0, sizeof(double)*(SAMPLES));
  
  curhv = -1;
  for (i = 0; i < SAMPLES ;i++)
    {
      tot_hw = 0;
      for (j = 0; j < SAMPLES; j++)
	{
	  if (hv[i].c[0] == hv[j].c[0] || hv[i].c[0] == hv[j].c[1] || hv[i].c[1] == hv[j].c[0] || hv[i].c[1] == hv[j].c[1])
	    continue; 
	  if (hv[i].hv > hv[j].hv)
	    {
	      odds[hv[i].sample_i] += hw[hv[j].sample_i];
	    }
	  else if (hv[i].hv == hv[j].hv)
	    {
	      odds[hv[i].sample_i] += hw[hv[j].sample_i]/2.0;
	    }
	  tot_hw += hw[hv[j].sample_i];
	}
      if (tot_hw > 0)
	odds[hv[i].sample_i] /= tot_hw;
    }
}



void get_river_hand_hv_all(int flop, int turn, int river, struct hand_hv retval[HANDS])
{
  int i;
  //double w,t;
  CardMask board, hand, hand1;
  HandVal hv1;
  //struct hand_hv tmp_hv

  board = int_to_cardmask_3[flop];

  CardMask_SET(board, turn);
  CardMask_SET(board, river);

  
  for (i = 0; i < HANDS;i++)
    {
      hand = int_to_cardmask_2[i];
      if (CardMask_ANY_SET(board, hand))
	{
	  retval[i].hv = 0;
	  retval[i].hand = i;
	  retval[i].c1 = 0xff;
	  retval[i].c2 = 0xff;
	  continue;
	}

      CardMask_OR(hand1, hand, board);
      hv1 = Hand_EVAL_N(hand1, 7);
      retval[i].hv = hv1;
      retval[i].hand = i;
      retval[i].c1 = int_to_cards_2[i].c1;
      retval[i].c2 = int_to_cards_2[i].c2;
    }
  
  qsort(retval, HANDS, sizeof(struct hand_hv), compare_hv2);
}

void get_river_hand_hv2_all_hands(int flop, int turn, int river, struct hand_hv2 retval[HANDS])
{
  int i, hand_i, hv_count;
  //uint16_t cur_hv2;
  //double w,t;
  CardMask board, hand, hand1;
  HandVal hv1, cur_hv;
  struct hand_hv tmp_hv[HANDS];

  board = int_to_cardmask_3[flop];

  CardMask_SET(board, turn);
  CardMask_SET(board, river);
  i = 0;

  for (hand_i = 0; hand_i < HANDS;hand_i++)
    {
      hand = int_to_cardmask_2[hand_i];
      if (CardMask_ANY_SET(board, hand))
	{
	  tmp_hv[hand_i].hv = 0;
	  tmp_hv[hand_i].hand = hand_i;
	  tmp_hv[hand_i].c1 = 0xff;
	  tmp_hv[hand_i].c2 = 0xff;
	  //	  tmp_hv[i].hv = 0;
	  //tmp_hv[i].sample_i = i;
	  //tmp_hv[i].c[0] = int_to_cards_2[hand_i].c1;
	  //tmp_hv[i].c[1] = int_to_cards_2[hand_i].c2;
	  //hand_i++;
	  continue;
	}
      
      CardMask_OR(hand1, hand, board);
      hv1 = Hand_EVAL_N(hand1, 7);
      tmp_hv[hand_i].hv = hv1;
      tmp_hv[hand_i].hand = hand_i;
      tmp_hv[hand_i].c1 = int_to_cards_2[hand_i].c1;
      tmp_hv[hand_i].c2 = int_to_cards_2[hand_i].c2;
    }
  
  qsort(tmp_hv, HANDS, sizeof(struct hand_hv), compare_hv2);

  cur_hv = 0;
  hv_count = -1;
  for (hand_i = 0; hand_i < HANDS; hand_i++)
    {
      if (cur_hv != tmp_hv[hand_i].hv)
	{
	  hv_count++;
	  cur_hv = tmp_hv[hand_i].hv;
	}
      retval[hand_i].hv = hv_count;
      retval[hand_i].sample_i = tmp_hv[hand_i].hand;
      retval[hand_i].c[0] = tmp_hv[hand_i].c1;
      retval[hand_i].c[1] = tmp_hv[hand_i].c2;
    }

  //assert(cur_hv2 == HANDS-1);

}


void get_river_hand_hv2_all(int flop, int turn, int river, struct hand_hv2 retval[SAMPLES])
{
  int i, hand_i, last_change, j;
  uint16_t cur_hv2;
  //double w,t;
  CardMask board, hand, hand1;
  HandVal hv1, cur_hv;
  struct hand_hv tmp_hv[SAMPLES];

  board = int_to_cardmask_3[flop];

  CardMask_SET(board, turn);
  CardMask_SET(board, river);
  i = 0;
  hand_i = 0;
  while (i < SAMPLES)
    {
      hand = int_to_cardmask_2[hand_i];
      if (CardMask_ANY_SET(board, hand))
	{
	  //	  tmp_hv[i].hv = 0;
	  //tmp_hv[i].sample_i = i;
	  //tmp_hv[i].c[0] = int_to_cards_2[hand_i].c1;
	  //tmp_hv[i].c[1] = int_to_cards_2[hand_i].c2;
	  hand_i++;
	  continue;
	}
      
      CardMask_OR(hand1, hand, board);
      hv1 = Hand_EVAL_N(hand1, 7);
      tmp_hv[i].hv = hv1;
      tmp_hv[i].hand = i;
      tmp_hv[i].c1 = int_to_cards_2[hand_i].c1;
      tmp_hv[i].c2 = int_to_cards_2[hand_i].c2;
      hand_i++;
      i++;
    }
  
  qsort(tmp_hv, SAMPLES, sizeof(struct hand_hv), compare_hv2);
  cur_hv2 = -1;
  cur_hv = -1;
  last_change = 0;
  for (i = 0; i < SAMPLES; i++)
    {
      if (cur_hv != tmp_hv[i].hv)
	{
	  cur_hv2 += (i-last_change);
	  for (j = last_change; j < i; j++)
	    {
	      retval[j].hv = cur_hv2;
	      retval[j].sample_i = tmp_hv[j].hand;
	      retval[j].c[0] = tmp_hv[j].c1;
	      retval[j].c[1] = tmp_hv[j].c2;
	    }
	  //cur_hv2--;
	  //retval[i].hv = cur_hv2;
	  //retval[i].hand = tmp_hv[i].hand;

	  cur_hv = tmp_hv[i].hv;
	  last_change = i;
	}
    }
  
  cur_hv2 += (i-last_change);
  for (j = last_change; j < i; j++)
    {
      retval[j].hv = cur_hv2;
      retval[j].sample_i = tmp_hv[j].hand;
      retval[j].c[0] = tmp_hv[j].c1;
      retval[j].c[1] = tmp_hv[j].c2;  
    }
  assert(cur_hv2 == SAMPLES-1);

}


void get_river_hand_hv_all_cardmask(CardMask board, struct hand_hv *retval)
{
  int i;
  //double w,t;
  CardMask hand, hand1;
  HandVal hv1;
  
  for (i = 0; i < HANDS;i++)
    {
      hand = int_to_cardmask_2[i];
      if (CardMask_ANY_SET(board, hand))
	{
	  retval[i].hv = 0;
	  retval[i].hand = i;
	  retval[i].c1 = 0xff;
	  retval[i].c2 = 0xff;
	  continue;
	}

      CardMask_OR(hand1, hand, board);
      hv1 = Hand_EVAL_N(hand1, 7);
      retval[i].hv = hv1;
      retval[i].hand = i;
      retval[i].c1 = int_to_cards_2[i].c1;
      retval[i].c2 = int_to_cards_2[i].c2;
    }
  
  qsort(retval, HANDS, sizeof(struct hand_hv), compare_hv2);
}



void get_river_wtl_all(int flop, int turn, int river, struct wtl retval[HANDS])
{
  int curhv_count,tot_count,i,j,prev_hv_change = 0;
  //double w,t;
  CardMask board;
  HandVal curhv;
  struct hand_hv hv[HANDS];
  //struct wtl hands[1326];
  int cardcount[52];
  int cardcount_curhv[52];
  board = int_to_cardmask_3[flop];
  assert(!CardMask_CARD_IS_SET(board, turn) && !CardMask_CARD_IS_SET(board, river) && turn != river);
  /* if (CardMask_CARD_IS_SET(board, turn) || CardMask_CARD_IS_SET(board, river) || turn == river) */
  /*   { */
  /*     for (i = 0; i < HANDS; i++) */
  /* 	{ */
  /* 	  retval[i].w = -1; */
  /* 	  retval[i].t = -1; */
  /* 	  retval[i].l = -1; */
  /* 	} */
  /*     return; */
  /*   } */
  CardMask_SET(board, turn);
  CardMask_SET(board, river);

  get_river_hand_hv_all_cardmask(board, hv);

  memset(cardcount, 0, sizeof(cardcount));
  memset(cardcount_curhv, 0, sizeof(cardcount_curhv));
  curhv_count = 0;
  curhv = 0;
  tot_count = 0;
  for (i = 0; i < HANDS;i++)
    {
      if (hv[i].c1 == 0xff)
	{
	  //  printf("Nomitaihmetta\n");
	  retval[hv[i].hand].w = -1;
	  retval[hv[i].hand].t = -1;
	  retval[hv[i].hand].l = -1;
	  prev_hv_change = i+1;
	  continue;
	}
      
      if (hv[i].hv != curhv)
	{
	  //odds[hv[i].hand] += curhv_n - cardcount_curhv[hv[i].c1] - cardcount_curhv[hv[i].c2];
	  for (j = prev_hv_change; j < i;j++)
	    {
	      retval[hv[j].hand].w = tot_count - cardcount[hv[j].c1] - cardcount[hv[j].c2];
	      retval[hv[j].hand].t = (curhv_count - cardcount_curhv[hv[j].c1] - cardcount_curhv[hv[j].c2] + 1);
	      //retval[hv[j].hand].l = HANDS - i;
	      retval[hv[j].hand].l = 990 - retval[hv[j].hand].w - retval[hv[j].hand].t;
		//assert(retval[hv[j].hand].l == 990 - odds[hv[j].hand].w - odds[hv[j].hand].t);
	    }
	  prev_hv_change = i;

	  for (j = 0; j < 52; j++)
	    cardcount[j] += cardcount_curhv[j];

	  tot_count+=curhv_count;

	  memset(cardcount_curhv, 0, sizeof(cardcount_curhv));	  
	  cardcount_curhv[hv[i].c1] = 1;
	  cardcount_curhv[hv[i].c2] = 1;
	  curhv_count = 1;
	  curhv = hv[i].hv;
	}
      else
	{
	  cardcount_curhv[hv[i].c1]++;
	  cardcount_curhv[hv[i].c2]++;
	  curhv_count++;
	}

    }
  for (j = prev_hv_change; j < i;j++)
    {
      retval[hv[j].hand].w = tot_count - cardcount[hv[j].c1] - cardcount[hv[j].c2];
      retval[hv[j].hand].t = (curhv_count - cardcount_curhv[hv[j].c1] - cardcount_curhv[hv[j].c2] + 1);
      //retval[hv[j].hand].l = HANDS - i;
      retval[hv[j].hand].l = 990 - retval[hv[j].hand].w - retval[hv[j].hand].t;
      //assert(retval[hv[j].hand].l == 990 - odds[hv[j].hand].w - odds[hv[j].hand].t);
    }
}
 
/*
void calc_changetable(double *f, int *raw, int types1, int types2)
{
  int i, j, tot;

  for (i = 0; i < types1;i++)
    {
      tot = 0;
      for (j = 0; j < types2; j++)
	{
	  tot += raw[i*types2+j];
	}
      if (tot == 0)
	{
	  for (j = 0; j < types2; j++)
	    {
	      f[i*types2+j] = 0;
	    }
	}
      else
	{
	  for (j = 0; j < types2; j++)
	    {
	      f[i*types2+j] = (double)((double)raw[i*types2+j])/((double)tot);
	    }
	}
    }
}
*/

/* int shtest = 0; */

/* int sharedlibtest_set(int value) */
/* { */
/*   shtest = value; */
/*   return value; */
/* } */

/* int sharedlibtest_get() */
/* { */
/*   return shtest; */
/* } */

/* void load_stuff() */
/* { */
/*   FILE *fp; */
/*   int tmpraw[MAXTYPES*MAXTYPES]; */

/*   fp = fopen("turn.types2", "rb"); */
/*   fread(prec_turn_types, sizeof(int), TYPES*SLOTS, fp); */
/*   fclose(fp); */
  
/*   printf("start preflop load\n"); */
/*   fp = fopen("preflop_to_flop", "rb"); */
/*   fread(tmpraw, sizeof(int), PREFLOPTYPES*FLOPTYPES, fp); */
/*   fclose(fp); */
/*   calc_changetable(hv_odds_preflop, tmpraw, PREFLOPTYPES,FLOPTYPES); */
/*   printf("start flop load\n"); */
/*   fp = fopen("flop_to_turn", "rb"); */
/*   fread(tmpraw, sizeof(int), FLOPTYPES*TURNTYPES, fp); */
/*   fclose(fp); */
/*   calc_changetable(hv_odds_flop, tmpraw, FLOPTYPES,TURNTYPES); */
/*   printf("start turn load\n"); */
/*   fp = fopen("turn_to_river", "rb"); */
/*   fread(tmpraw, sizeof(int), TURNTYPES*SLOTS, fp); */
/*   fclose(fp); */
/*   calc_changetable(hv_odds_turn, tmpraw, TURNTYPES,SLOTS); */
 
/*   memset(hv_odds_start, 0, sizeof(hv_odds_start)); */
/*   for (i = 0; i < HANDS; i++) */
/*     { */
/*       hv_odds_start[get_preflop_slot(i)] += 1.0/(double)HANDS; */
/*     }  */
/*   /\*  for (i = 0; i < FLOPTYPES; i++) */
/*     { */
/*       printf("%f\n", hv_odds_turn[i]); */
/*       }*\/ */
/* } */

 /*int startup()
{

  srand(time(NULL));

  printf("precalc_cards\n");
  precalc_cards();
  printf("load stuff\n");
  //load_stuff();
  printf("startup done\n");
  return 0;
}
 */
