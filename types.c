#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "defs.h"
#include "handval.h"
#include "types.h"
#include "bit.h"
//#include "cuda.h"

#define INC 1.0/46.0


extern struct cards_3 int_to_cards_3[22100];
extern struct cards_2 int_to_cards_2[1326];
extern struct cards_1 int_to_cards_1[52];
extern int cards_to_int_4[52][52][52][52];
extern int cards_to_int_3[52][52][52];
extern int cards_to_int_2[52][52];
extern int cards_to_int_1[52];

extern CardMask int_to_cardmask_4[270725];
extern CardMask int_to_cardmask_3[22100];
extern CardMask int_to_cardmask_2[1326];

int cardc = HANDS;
int flopc = 0;



void print_type(float *t, int n_slots)
{
  int i, j;
  float max = 0;

  for (i = 0; i < n_slots; i++)
    {
      if (t[i] > max)
	max = t[i];
    }

  if (max == 0)
    {
      printf("max %f\n", max);
      return;
    }
  for (j = 0; j < n_slots; j++)
    {
      printf("%i", (int)((t[j]/max-0.000001)*10.0));
    }
  //printf("\n");
}



void gen_all_preflop_types(struct gameinfo *info, float *types)
{
  int i, tmphand, n_rtypes;
  int i1,i2,i3,i4,i5;

  //  static struct wtl tmp_wtl[HANDS];
  short int slots[HANDS];


  n_rtypes = info->n_rtypes[GS_PF];
  for (i1 = 51; i1 >= 0; i1--)
    for (i2 = i1 - 1; i2 >= 0; i2--)
      {
	for (i3 = i2 - 1; i3 >= 0; i3--)
	  {
	    printf("new_flop %i\n", cards_to_int_3[i1][i2][i3]);
	    for (i4 = i3 - 1; i4 >= 0; i4--)
	      for (i5 = i4 - 1; i5 >= 0; i5--)
		{
		  get_slots_for_river(info, slots, cards_to_int_3[i1][i2][i3], i4, i5);
		  //get_river_wtl_all(cards_to_int_3[i1][i2][i3], i4, i5, tmp_wtl);
		  for (i = 0; i < HANDS; i++)
		    {
		      if (slots[i] == -1)
			continue;
		      types[i*n_rtypes + slots[i]] += 1.0/1024.0;
		      //add_wtl_value_to_type(&(types[i*wl_slots*tie_slots]), wl_slots, tie_slots, wl_scale, tie_scale, &(tmp_wtl[i])); 
		      //types[i][tmp_slots[i]] += 1;
		    }
		}
	  }
	tmphand = random()%HANDS;
	//tmphand = 102;
	printf("type: %i ", tmphand);
	printf("%s\n", Deck_maskString(int_to_cardmask_2[tmphand]));
	print_type(&(types[tmphand*n_rtypes]), n_rtypes);
      } 
  for (i = 0; i < HANDS; i++)
    {
      //degauss_type(&(types[i*wl_slots*tie_slots]), wl_slots, tie_slots, wl_width, tie_width);
      degauss_type_with_diffs(&(types[i*n_rtypes]), info->diffs[GS_R], info->diffs_order[GS_R], info->gauss_width[GS_PF], n_rtypes);

    }
}

void gen_types_for_flop(struct gameinfo *info, float *types, int flop)
{
  int i,j, tmphand;
  int n_rtypes;
  short int slots[HANDS];

  n_rtypes = info->n_rtypes[GS_F];
  memset(types, 0, HANDS*n_rtypes*sizeof(float));
  for (i = 0;i < HANDS;i++)
    {
      if (CardMask_ANY_SET(int_to_cardmask_3[flop], int_to_cardmask_2[i]))
	continue;
      //      get_river_wtl_all(flop, int_to_cards_2[i].c1, int_to_cards_2[i].c2, tmp_wtl);
      get_slots_for_river(info, slots, flop,  int_to_cards_2[i].c1, int_to_cards_2[i].c2);
      for (j = 0; j < HANDS;j++)
	{
	  if (slots[j] == -1)
	    {
	      continue;
	    }
	  types[j*n_rtypes + slots[j]] += 1.0/1024.0;
	  
	  //add_wtl_value_to_type(&(types[j*n_rtypes]), wl_slots, tie_slots, wl_scale, tie_scale, &(tmp_wtl[j]));
	}
    }
  tmphand = random()%HANDS;
  while (CardMask_ANY_SET(int_to_cardmask_3[flop], int_to_cardmask_2[tmphand]))
    tmphand = random()%HANDS;
  printf("flop: %s ", Deck_maskString(int_to_cardmask_3[flop]));
  printf("hand: %s\n",  Deck_maskString(int_to_cardmask_2[tmphand]));
  //printf("%s\n", Deck_maskString(int_to_cardmask_2[tmphand]));
  print_type(&(types[tmphand*n_rtypes]), n_rtypes);
  printf("\n");

  for (i = 0;i < HANDS;i++)
    {
      degauss_type_with_diffs(&(types[i*n_rtypes]), info->diffs[GS_R], info->diffs_order[GS_R], info->gauss_width[GS_F], n_rtypes);
    }
  //printf("%s\n", Deck_maskString(int_to_cardmask_2[tmphand]));
  print_type(&(types[tmphand*n_rtypes]), n_rtypes);
  printf("\n");
}

void gen_types_for_turn(struct gameinfo *info, float *types, int flop, int turn)
{
  int i,j;
  int n_rtypes;
  short int slots[HANDS];

  n_rtypes = info->n_rtypes[GS_T];
  memset(types, 0, HANDS*n_rtypes*sizeof(float));

  for (i = 0;i < 52;i++)
    {
      if (CardMask_CARD_IS_SET(int_to_cardmask_3[flop],i) || turn == i)
	{
	  continue;
	}
      //      get_river_wtl_all(flop, turn, i, tmp_wtl);
      get_slots_for_river(info, slots, flop, turn, i);

      //      printf("%i\n", i);
      for (j = 0; j < HANDS;j++)
	{
	  if (slots[j] == -1)
	    {
	      continue;
	    }
	  types[j*n_rtypes + slots[j]] += 1;

	  //printf("%i %i %i\n", tmp_wtl[j].w,tmp_wtl[j].t,tmp_wtl[j].l);
	  //add_wtl_value_to_type(&(types[j*info->rtype[2].wl_slots*info->rtype[2].tie_slots]), info->rtype[2].wl_slots, info->rtype[2].tie_slots, info->rtype[2].wl_scale, info->rtype[2].tie_scale, &(tmp_wtl[j]));
	  //break;
	}
    }
  for (i = 0;i < HANDS;i++)
    {
      degauss_type_with_diffs(&(types[i*n_rtypes]), info->diffs[GS_R], info->diffs_order[GS_R], info->gauss_width[GS_T], n_rtypes);
      //      degauss_type(&(types[i*info->rtype[2].wl_slots*info->rtype[2].tie_slots]), info->rtype[2].wl_slots, info->rtype[2].tie_slots, info->rtype[2].wl_width, info->rtype[2].tie_width);
    }
}


void gen_single_turn_type(struct gameinfo *info, float *type, int flop, int turn, int hand)
{
  int i;
  
  CardMask dead_cards;
  //struct wtl tmp_wtl;
  short int slot;
  float tmp_river_type[3];
  //float multi,multi2, tot;
  //  static char slotcount[SLOTS];
  //  int tmptype[SLOTS];
  memset(type, 0, sizeof(float)*info->n_rtypes[GS_T]);
  
  CardMask_OR(dead_cards, int_to_cardmask_3[flop], int_to_cardmask_2[hand]);
  CardMask_SET(dead_cards, turn);
  for (i = 0; i < 52;i++)
    {
      if (CardMask_CARD_IS_SET(dead_cards, i))
	{
	  continue;
	}
      
      gen_single_river_type(info, tmp_river_type, flop, turn, i, hand); 
      slot = get_slot(info->types[GS_R], tmp_river_type, info->n_types[GS_R], info->n_rtypes[GS_R]);
      type[slot] += 1;
      //get_river_wtl_single(flop, turn,i,hand, &tmp_wtl);
      
      //add_wtl_value_to_type(type, info->rtype[2].wl_slots, info->rtype[2].tie_slots, info->rtype[2].wl_scale, info->rtype[2].tie_scale, &tmp_wtl);
      
    }
  degauss_type_with_diffs(type, info->diffs[GS_R], info->diffs_order[GS_R], info->gauss_width[GS_T], info->n_rtypes[GS_T]);
  //degauss_type(type, info->rtype[2].wl_slots, info->rtype[2].tie_slots, info->rtype[2].wl_width, info->rtype[2].tie_width);
}

void gen_single_river_type(struct gameinfo *info, float *type, int flop, int turn, int river, int hand)
{
    struct wtl tmp_wtl;
    float tot;

    get_river_wtl_single(flop, turn,river,hand, &tmp_wtl);
    tot = (float)(tmp_wtl.w + tmp_wtl.l + tmp_wtl.t);
    //printf("tot: %f, %f, %f, %f\n", tot, (float)tmp_wtl.w,(float)tmp_wtl.t,(float)tmp_wtl.l);
    type[0] = ((float)tmp_wtl.w)/tot;
    type[1] = ((float)tmp_wtl.t)/tot/2.0; //tasuria merkityksettomammaksi. 2.0 vedetty hihasta
    type[2] = ((float)tmp_wtl.l)/tot;
}


void gen_random_turn_type(struct gameinfo *info, float *type)
{
  int c1,c2,c3,c4,h1,h2;
  c1 = random()%52;
  
  do{c2 = random()%52;}
  while (c2 == c1);
  do{c3 = random()%52;}
  while (c3 == c1 || c3 == c2);
  do{c4 = random()%52;}
  while (c4 == c1 || c4 == c2 || c4 == c3);
  do{h1 = random()%52;}
  while (h1 == c1 || h1 == c2 || h1 == c3 || h1 == c4);
  do{h2 = random()%52;}
  while (h2 == c1 || h2 == c2 || h2 == c3 || h2 == c4 || h2 == h1);
  gen_single_turn_type(info, type, cards_to_int_3[c1][c2][c3], c4,cards_to_int_2[h1][h2]);
  
}

void gen_random_river_type(struct gameinfo *info, float *type)
{
  int c1,c2,c3,c4,c5,h1,h2;
  //  struct wtl tmp_wtl;

  c1 = random()%52;
  do{c2 = random()%52;}
  while (c2 == c1);
  do{c3 = random()%52;}
  while (c3 == c1 || c3 == c2);
  do{c4 = random()%52;}
  while (c4 == c1 || c4 == c2 || c4 == c3);
  do{c5 = random()%52;}
  while (c5 == c1 || c5 == c2 || c5 == c3 || c5 == c4);

  do{h1 = random()%52;}
  while (h1 == c1 || h1 == c2 || h1 == c3 || h1 == c4 || h1 == c5);
  do{h2 = random()%52;}
  while (h2 == c1 || h2 == c2 || h2 == c3 || h2 == c4 || h2 == c5 || h2 == h1);
  
  gen_single_river_type(info, type, cards_to_int_3[c1][c2][c3], c4,c5, cards_to_int_2[h1][h2]);
  
}

void get_slots_for_river(struct gameinfo *info, short int *slots, int flop, int turn, int river)
{
  float tmp_type[3];
  struct wtl tmp_wtl[HANDS];
  float tot;
  int i;

  //  printf("getting all wtl\n");
  get_river_wtl_all(flop, turn, river, tmp_wtl);
  //printf("done getting all wtl\n");
  for (i = 0; i < HANDS; i++)
    {
      if (tmp_wtl[i].w == -1)
	{
	  slots[i] = -1;
	  continue;
	}
      tot = (float)(tmp_wtl[i].w + tmp_wtl[i].l + tmp_wtl[i].t);
      //printf("tot: %f, %f, %f, %f\n", tot, (float)tmp_wtl[i].w,(float)tmp_wtl[i].t,(float)tmp_wtl[i].l);
      tmp_type[0] = ((float)tmp_wtl[i].w)/tot;
      tmp_type[1] = ((float)tmp_wtl[i].t)/tot/2.0; //tasuria merkityksettomammaksi. 2.0 vedetty hihasta
      tmp_type[2] = ((float)tmp_wtl[i].l)/tot;
  
      slots[i] = get_slot(info->types[GS_R], tmp_type, info->n_types[GS_R], info->n_rtypes[GS_R]);
      //printf("%f %f %f %i\n", tmp_type[0], tmp_type[1], tmp_type[2], slots[i]);
    }
}

short int get_slots_wtl_stats_for_river(struct gameinfo *info, struct wtl_f *wtl_stats, int flop, int turn, int river)
{
  float tmp_type[3];
  struct wtl tmp_wtl[HANDS];
  short int slots[HANDS];
  HandVal hvs[HANDS];
  float tot, addcount;
  int i, j;
  short int slot;
  float *bt;

  //  printf("getting all wtl\n");
  get_river_wtl_all(flop, turn, river, tmp_wtl);
  get_river_handhv_all(flop, turn, river, hvs);
  //printf("done getting all wtl\n");
  for (i = 0; i < HANDS; i++)
    {
      if (tmp_wtl[i].w == -1)
	{
	  slots[i] = -1;
	  continue;
	}
      tot = (float)(tmp_wtl[i].w + tmp_wtl[i].l + tmp_wtl[i].t);
      //printf("tot: %f, %f, %f, %f\n", tot, (float)tmp_wtl[i].w,(float)tmp_wtl[i].t,(float)tmp_wtl[i].l);
      tmp_type[0] = ((float)tmp_wtl[i].w)/tot;
      tmp_type[1] = ((float)tmp_wtl[i].t)/tot/2.0; //tasuria merkityksettomammaksi. 2.0 vedetty hihasta
      tmp_type[2] = ((float)tmp_wtl[i].l)/tot;
  
      slots[i] = get_slot(info->types[GS_R], tmp_type, info->n_types[GS_R], info->n_rtypes[GS_R]);
      //printf("%f %f %f %i\n", tmp_type[0], tmp_type[1], tmp_type[2], slots[i]);
    }

  
  addcount = 0;
  for (i = 0; i < HANDS; i++)
    {
      if (hvs[i] == -1)
	continue;
      for (j = i+1; j < HANDS; j++)
	{
	  if (hvs[j] == -1)
	    continue;
	  if (CardMask_ANY_SET(int_to_cardmask_2[i], int_to_cardmask_2[j]))
	    {
	      continue;
	    }
	  addcount += 2.0/1024.0;
	  if (hvs[i] > hvs[j])
	    {
	      wtl_stats[info->n_types[GS_R]*slots[i] + slots[j]].w += 1.0/1024.0;
	      wtl_stats[info->n_types[GS_R]*slots[j] + slots[i]].l += 1.0/1024.0;
	    }
	  else if (hvs[i] < hvs[j])
	    {
	      wtl_stats[info->n_types[GS_R]*slots[i] + slots[j]].l += 1.0/1024.0;
	      wtl_stats[info->n_types[GS_R]*slots[j] + slots[i]].w += 1.0/1024.0;
	    }
	  else if (hvs[i] == hvs[j])
	    {
	      wtl_stats[info->n_types[GS_R]*slots[i] + slots[j]].t += 1.0/1024.0;
	      wtl_stats[info->n_types[GS_R]*slots[j] + slots[i]].t += 1.0/1024.0;
	    } 
	  else
	    {
	      printf("EI VOI KOSKAAAAAAAAN %i %i %i %i %i %i || %i %i %i %i %i\n",tmp_wtl[i].w,tmp_wtl[j].w,tmp_wtl[i].l,tmp_wtl[j].l,tmp_wtl[i].t,tmp_wtl[j].t, flop, turn, river, i, j);
	    }
 	}
    }

   bt = (float *)calloc(1, sizeof(float)*info->n_htfb[GS_R-1]*info->n_htfb[GS_R]);
   gen_board_type(info, bt, GS_R, flop, turn, river);
   slot = get_slot(info->b_types[GS_R], bt, info->n_btypes[GS_R], info->n_htfb[GS_R-1]*info->n_htfb[GS_R]);
   free(bt);
   //printf("addcount %f %i\n", addcount, sizeof(struct wtl_f));
   return slot;
}

float gauss(float height, float center, float width, float x)
{
  return height * exp( -( powf( (x-center),2.0) / (2.0*powf(width,2.0) ) ));
}

void degauss_type_with_diffs_gsl(float *type, float *diffs, short int *diffs_order, float width, int n_slots)
{
  int i,j,doi;
  float *tmp_t;
  float height, g, w, max_diff, diff;

  tmp_t = (float*)malloc(sizeof(float)*n_slots);
  memset(tmp_t, 0, sizeof(float)*n_slots);


  for (i = 0; i < n_slots; i++)
    {
      if (type[i] == 0)
	continue;
      max_diff = diffs[i*n_slots + diffs_order[i*n_slots + n_slots-2]];
      w = (max_diff*width);
      height = type[i];
      //printf("max_diff: %f w: %f height: %f, n_slots %i \n", max_diff, w, height, n_slots);
      tmp_t[i] += height;
      for (j = 0; j < n_slots; j++)
	{
	  doi = diffs_order[i*n_slots + j];
	  diff = diffs[i*n_slots + doi];
	  if (diff > w)
	    break;
	  g = gauss(height, 0, w/3.0, diff);
	  
	  tmp_t[doi] += g;
	}
    }
  memcpy(type, tmp_t, sizeof(float)*n_slots); 
  free(tmp_t);
}
  

void degauss_type_with_diffs(float *type, float *diffs, short int *diffs_order, float width, int n_slots)
{
  int i,j,doi;
  float *tmp_t;
  float height, g, w, max_diff, diff;

  tmp_t = (float*)malloc(sizeof(float)*n_slots);
  memset(tmp_t, 0, sizeof(float)*n_slots);


  for (i = 0; i < n_slots; i++)
    {
      if (type[i] == 0)
	continue;
      max_diff = diffs[i*n_slots + diffs_order[i*n_slots + n_slots-2]];
      w = (max_diff*width);
      height = type[i];
      //printf("max_diff: %f w: %f height: %f, n_slots %i \n", max_diff, w, height, n_slots);
      tmp_t[i] += height;
      for (j = 0; j < n_slots; j++)
	{
	  doi = diffs_order[i*n_slots + j];
	  diff = diffs[i*n_slots + doi];
	  if (diff > w)
	    break;
	  g = gauss(height, 0, w/3.0, diff);
	  
	  tmp_t[doi] += g;
	}
    }
  memcpy(type, tmp_t, sizeof(float)*n_slots); 
  free(tmp_t);
}



float get_difference(float *t1, float *t2, int n_slots)
{
  int i;
  float totdiff = 0;

  for (i = 0; i < n_slots;i++)
    {
      totdiff += fabs(t1[i]-t2[i]);
    }
  return totdiff;
}

float get_difference_pow2(float *t1, float *t2, int n_slots)
{
  int i;
  float totdiff = 0;

  for (i = 0; i < n_slots;i++)
    {
      totdiff += powf((t1[i]-t2[i]), 2);
    }
  return totdiff;
}


float get_difference_max(float *t1, float *t2, int n_slots, float max)
{
  int i;
  float totdiff = 0;

  for (i = 0; i < n_slots;i++)
    {
      totdiff += fabs(t1[i]-t2[i]);
      if (totdiff > max)
	break;
    }
  return totdiff;
}

float get_difference_max_pow2(float *t1, float *t2, int n_slots, float max)
{
  int i;
  float totdiff = 0;

  for (i = 0; i < n_slots;i++)
    {
      totdiff += powf((t1[i]-t2[i]), 2);
      if (unlikely(totdiff > max))
	break;
    }
  return totdiff;
}

float try_new_type(float *types, float *diffs, float *t, int n_types, int n_slots_per_type, float smallest, int replace_i)
{
  float tmp_diff, small=1000000000;
  float *new_diffs;
  int i;


  new_diffs = (float*)malloc(sizeof(float)*n_types);
  //  memset(new_diffs, 0, sizeof(float)*n_types);
  for (i = 0; i < n_types; i++)
    {
      if (i == replace_i)
	{
	  new_diffs[i] = 1000000000;
	  continue;
	}
      tmp_diff = get_difference(&(types[i*n_slots_per_type]), t, n_slots_per_type);
      if (tmp_diff <= smallest)
	{
	  free(new_diffs);
	  return 0.0;
	}
      new_diffs[i] = tmp_diff;
      if (tmp_diff < small)
	small = tmp_diff;
    }

  memcpy(&(diffs[replace_i*n_types]), new_diffs, sizeof(float)*n_types);
  memcpy(&(types[replace_i*n_slots_per_type]), t, sizeof(float)*n_slots_per_type);
  diffs[replace_i*n_types+replace_i] = 1000000000;
  for (i = 0; i < n_types; i++)
    {
      if (i == replace_i)
	continue;
      diffs[i*n_types+replace_i] = new_diffs[i];
    }

  free(new_diffs);
  return small;
}

void calc_all_diffs(float *types, float *diffs, int n_types, int n_slots_per_type)
{
  int i, j;


  for (j = 0; j < n_types; j++)
    {
      diffs[j*n_types + j] = 1000000000;
      for (i = j+1; i < n_types; i++)
	{	  
	  diffs[j*n_types + i] = get_difference(&(types[j*n_slots_per_type]), &(types[i*n_slots_per_type]), n_slots_per_type);
	  diffs[i*n_types + j] = diffs[j*n_types + i];
	}
    }
}

void calc_diffs_for_one_type(float *types, float *type, float *diff, int n_types, int n_slots_per_type)
{
  int i;

  for (i = 0; i < n_types; i++)
    {
      diff[i] = get_difference_pow2(type, &(types[i*n_slots_per_type]), n_slots_per_type);
    }
}


float calc_diffs(float *types, float *diffs, int n_types, int n_slots_per_type, int t)
{
  int i;
  float small = 1000000000;

  for (i = 0; i < n_types; i++)
    {
      if (i == t)
	{
	  diffs[t*n_types + t] = 1000000000;
	  continue;
	}
      diffs[t*n_types + i] = get_difference(&(types[t*n_slots_per_type]), &(types[i*n_slots_per_type]), n_slots_per_type);
      diffs[i*n_types + t] = diffs[t*n_types + i];
      if (diffs[i*n_types + t] == 0)
	{
	  printf("duplicate %i %i\n", t,i);
	}
      if (diffs[t*n_types + i] < small)
	small = diffs[t*n_types + i];
    }
  return small;
}

float get_board_combo_diff(struct gameinfo *info, float *maxdiffs, int16_t *combo_type1, int16_t *combo_type2, int gamestate)
{
  int i;
  float diff = 0;
  for (i = 0; i < gamestate;i++)
    {
      diff += info->b_diffs[i+1][combo_type1[i]*info->n_btypes[i+1]+combo_type2[i]]/maxdiffs[i+1];
      assert(info->b_diffs[i+1][combo_type1[i]*info->n_btypes[i+1]+combo_type2[i]] == info->b_diffs[i+1][combo_type2[i]*info->n_btypes[i+1]+combo_type1[i]]);
    }
  return diff;
}

void get_board_combo_diffs(struct gameinfo *info, float *maxdiffs, int16_t *combo_types, int16_t *combo_type, int gamestate, int n_combo_types, float *diffs)
{
  int i;
  for (i = 0; i < n_combo_types; i++)
    {
      diffs[i] = get_board_combo_diff(info, maxdiffs, &combo_types[i*gamestate], combo_type, gamestate);
    }
}

int16_t get_board_combo_slot(struct gameinfo *info, float *maxdiffs, int16_t *combo_types, int16_t *combo_type, int gamestate, int n_combo_types)
{
  float diff = 0, smallest = 1000000000;
  int i;
  int16_t s = -1;


  for (i = 0; i < n_combo_types;i++)
    {
      //prefetch_range(&(types[i*n_slots_per_type]),n_slots_per_type*sizeof(float));
      diff = get_board_combo_diff(info, maxdiffs, &combo_types[i*gamestate], combo_type, gamestate);
      //diff = get_difference_max_pow2(&(types[i*n_slots_per_type]), t, n_slots_per_type, smallest);
      if (diff < smallest)
	{
	  s = (int16_t)i;
	  smallest = diff;
	  //printf("new slot %i %f\n", s, smallest);
	}
    }
  //printf("slot: %i, smallest %f\n", s, smallest);
  return s;
}

short int get_slot(float *types, float *t, int n_types, int n_slots_per_type)
{
  float diff = 0, smallest = 1000000000;
  int i,j;
  short int s = -1;

  for (j = 0; j < n_slots_per_type; j++)
    if (t[j] != 0)
      break;
  if (j == n_slots_per_type)
    return -1;
  for (i = 0; i < n_types;i++)
    {
      //prefetch_range(&(types[i*n_slots_per_type]),n_slots_per_type*sizeof(float));
      
      diff = get_difference_max_pow2(&(types[i*n_slots_per_type]), t, n_slots_per_type, smallest);
      if (diff < smallest)
	{
	  s = (short int)i;
	  smallest = diff;
	  //printf("new slot %i %f\n", s, smallest);
	}
    }
  //printf("slot: %i, smallest %f\n", s, smallest);
  if (smallest == 0)
    {
      for (i = 0; i < n_slots_per_type;i++)
	if (t[i] != 0)
	  break;
      if (i == n_slots_per_type)
	{
	  printf("fuckup type\n");
	  return -1;
	}
    }
  return s;
}

void get_slots_for_all(short int *slots, float *slot_types, float *types, int n_slots, int n_types, int n_slots_per_type)
{
  int i;
  for (i = 0; i < n_slots; i++)
    {
      
      slots[i] = get_slot(types, &(slot_types[i*n_slots_per_type]), n_types, n_slots_per_type);
    }
}


float get_closest_match(float *diffs, int n_types, int *t1, int *t2)
{
  int i, j, small_list_idx = 0;
  float small = 1000000000, diff;
  
  int *small_list;
  *t1 = 0;
  *t2 = 0;
  small_list = (int*) malloc(sizeof(int)*n_types*2);
  for (i = 0; i < n_types; i++)
    {
      for(j = i+1; j < n_types;j++)
	{
	  diff = diffs[i*n_types + j];
	  if (diff < small)
	    {
	      small = diff;
	      small_list_idx = 0;
	      small_list[small_list_idx*2] = i;
	      small_list[small_list_idx*2+1] = j;
	      small_list_idx++;
	    }
	  else if (diff == small)
	    {
	      small_list[small_list_idx*2] = i;
	      small_list[small_list_idx*2+1] = j;
	      small_list_idx++;
	    }
	}
    }
  i = random()%small_list_idx;
  *t1 = small_list[i*2];
  *t2 = small_list[i*2+1];
  free(small_list);
  printf("c_small: %f %i %i %i\n", small, *t1, *t2, small_list_idx);
  return small;
}

float get_avg_diff(float *diff, int n_types)
{
  int i;
  float tot = 0;
  for (i = 0; i < n_types; i++)
    {
      if (diff[i] == 1000000000)
	continue;
      tot += diff[i];
    }
  return tot/(float)(n_types - 1);
}


void generate_mapping_from_diffs(float *diffs, int *mapping, int n_types, int reduce_to)
{
  int x,y, smallest_x, smallest_y, non_mapped_types;
  float smallest = 1000000000;


  non_mapped_types = n_types;
  
  while (non_mapped_types > reduce_to)
    {
      //      small = get_smallest_diff_for_reduce(diffs, mapping, n_types, &smallest_x, &smallest_y);
      smallest = 1000000000;
      smallest_x = -1;
      smallest_y = -1;
      for (x = 0; x < n_types;x++)
	{
	  if (mapping[x] != x)
	    continue;
	  for (y = 0; y < n_types; y++)
	    {
	      if (mapping[y] != y)
		continue;
	      if (diffs[x*n_types+y] < smallest)
		{
		  smallest_x = x;
		  smallest_y = y;
		  smallest = diffs[x*n_types+y];
		}
	    }
	}
  
      mapping[smallest_x] = smallest_y;
      non_mapped_types--;
      printf("non_mapped_types: %i\n", non_mapped_types);
    }
}


void add_start_end_pair_to_board_type(struct gameinfo *info, float *bt, int gs, int start_slot, int end_slot, float g_max)
{
  float start_max_diff, end_max_diff, start_min_diff, end_min_diff, start_g, end_g, start_diff, end_diff;
  int i, j, n_start_types, n_end_types, start_idx, end_idx;
  float *start_diffs, *end_diffs;
  short int *start_diffs_order, *end_diffs_order;

  n_start_types = info->n_htfb[gs-1];
  start_diffs = (float *)&(info->diffs[gs-1][start_slot*n_start_types]);
  start_diffs_order = (short int *) &(info->diffs_order[gs-1][start_slot*n_start_types]);
  start_max_diff = start_diffs[start_diffs_order[n_start_types-2]];
  start_min_diff = start_diffs[start_diffs_order[0]];

  n_end_types = info->n_htfb[gs];
  end_diffs = (float *)&(info->diffs[gs][end_slot*n_end_types]);
  end_diffs_order = (short int *) &(info->diffs_order[gs][end_slot*n_end_types]);
  end_max_diff = end_diffs[end_diffs_order[n_end_types-2]];
  end_min_diff = end_diffs[end_diffs_order[0]];
  //printf("debug1 %i\n", info->diffs_order[0][127*n_start_types + 127]);
  start_idx = start_slot;
  end_idx = end_slot;
  //fprintf(stderr, "DEBUG0 %i %i %i %i\n", start_idx, n_end_types, end_idx, start_idx*n_end_types + end_idx);
  bt[start_idx*n_end_types + end_idx] += g_max;  
  //fprintf(stderr, "DEBUG1\n");
  for (j = 0; j < n_end_types; j++)
    {
      end_idx = end_diffs_order[j];
      end_diff = end_diffs[end_idx];
      end_g = gauss(g_max, end_min_diff, end_max_diff/20.0, end_diff);
      //if (end_g < 0.01)
      if (end_g < g_max/100.0)
	break;
      bt[start_idx*n_end_types + end_idx] += end_g;
      
    }

  //printf("DEBUG2 %i %f %f\n", j, g_max, end_g);
  for (i = 0; i < n_start_types; i++)
    {
      start_idx = start_diffs_order[i];
      start_diff = start_diffs[start_idx];
      start_g = gauss(g_max, start_min_diff, start_max_diff/20.0, start_diff);
      //if (start_g < 0.01)
      if (start_g < g_max/100.0)
	break;
      bt[start_idx*n_end_types + end_slot] += start_g;
      
      for (j = 0; j < n_end_types; j++)
	{
	  end_idx = end_diffs_order[j];
	  end_diff = end_diffs[end_idx];
	  end_g = gauss(start_g, end_min_diff, end_max_diff/20.0, end_diff);
	  //if (end_g < 0.01)
	  if (end_g < g_max/100.0)
	    break;
	  //	  printf("debug8 %i\n", info->diffs_order[0][127*n_start_types + 127]);
	  bt[start_idx*n_end_types + end_idx] += end_g;
	  //printf("debug9 %i\n", info->diffs_order[0][127*n_start_types + 127]);
	}
      //printf("%i %i %f\n", i, j, start_g);
    }

  //  printf("%f %i %i %f %f\n", g_max, i, j, start_max_diff, end_max_diff);

}

void gen_board_type(struct gameinfo *info, float *bt, int gs, int flop, int turn, int river)
{
  // struct cards_3 flop_c;
  int i, j, turn_i, n_start_types, n_end_types;
  short int *start_slots, *end_slots;
  float g_max, tot;
  float *tmp_bt = NULL;
  float tot1, multi;
  CardMask tmpcm;
  //flop_c = int_to_cards_3[flop];
  
  switch (gs)
    {
    case 1:
      start_slots = info->slots[gs-1];
      end_slots = &(info->slots[gs][flop*HANDS]);
      break;
    case 2:
      turn_i = cards_to_int_4[int_to_cards_3[flop].c1][int_to_cards_3[flop].c2][int_to_cards_3[flop].c3][turn];
      CardMask_RESET(tmpcm);
      CardMask_SET(tmpcm, turn);
      //printf("%i %i %i %i %i %i\n",int_to_cards_3[flop].c1,int_to_cards_3[flop].c2,int_to_cards_3[flop].c3, flop, turn, turn_i);
      //printf("%s\n", Deck_maskString(int_to_cardmask_4[turn_i]));
      //printf("%s\n", Deck_maskString(int_to_cardmask_3[flop]));
      //printf("%s\n", Deck_maskString(tmpcm));

      
      start_slots = (short int*)&(info->slots[gs-1][flop*HANDS]);
      end_slots = (short int*)&(info->slots[gs][turn_i*HANDS]);
      break;
    case 3:
      turn_i = cards_to_int_4[int_to_cards_3[flop].c1][int_to_cards_3[flop].c2][int_to_cards_3[flop].c3][turn];
      start_slots = (short int*)&(info->slots[gs-1][turn_i*HANDS]);
      end_slots = (short int *) malloc(sizeof(short int)*HANDS);

      //printf("getting end slots %i %i %i\n", flop, turn,river);
      get_slots_for_river(info, end_slots, flop, turn, river);
      //printf("done geting slots\n");
      break;
    default:
      start_slots = NULL;
      end_slots = NULL;
      break;
    }
  n_start_types = info->n_htfb[gs-1];
  n_end_types = info->n_htfb[gs];

  tmp_bt = (float *)malloc(sizeof(float)*n_start_types*n_end_types);
  memset(tmp_bt, 0, sizeof(float)*n_start_types*n_end_types);
  memset(bt, 0, sizeof(float)*n_start_types*n_end_types);
  for (i = 0; i < HANDS; i++)
    {
      if (start_slots[i] == -1 || end_slots[i] == -1)
	{
	  if (start_slots[i] == -1 && end_slots[i] != -1)
	    printf("EI PITAS TANNE KOSKAAN %i %i\n", start_slots[i], end_slots[i]);
	  
	  continue;	
	 
	}
      tmp_bt[start_slots[i]*n_end_types + end_slots[i]] += 1.0;
    }
  tot = 0;
  for (i = 0; i < n_start_types; i++)
    {
      for (j = 0; j < n_end_types; j++)
	{
	  /*if (start_slots[i] == -1 || end_slots[i] == -1)
	    {
	      if (start_slots[i] == -1 && end_slots[i] != -1)
		printf("EI PITAS TANNE KOSKAAN\n");
	      else
		continue;	
		}*/

	  if ((g_max = tmp_bt[i*n_end_types+j]) != 0)
	    {
	      //printf("gmax: %f\n", g_max);
	      tot += g_max;
	      add_start_end_pair_to_board_type(info, bt, gs, i, j, g_max);
	    }
	}
    }
  //normalize type
  for (i = 0; i < n_start_types; i++)
    {
      tot1 = 0;
      for (j = 0; j < n_end_types; j++)
	{
	  tot1 += bt[i*n_end_types+j];
	}
      if (tot1 != 0)
	{
	  multi = 1.0/tot1;
	  for (j = 0; j < n_end_types; j++)
	    {
	      bt[i*n_end_types+j] *= multi;
	    }
	}
    }

  free(tmp_bt);
  if (gs == 3)
    free(end_slots);
}

void gen_board_type_from_hi(struct type_type *t, struct hand_info *hi, float *bt)
{
  // struct cards_3 flop_c;
  int i, j, turn_i, n_start_types, n_end_types;
  short int *start_slots, *end_slots;
  float g_max, tot;
  float *tmp_bt = NULL;
  float tot1, multi;
  CardMask tmpcm;
  //flop_c = int_to_cards_3[flop];

  struct gameinfo *info = t->info;
  int flop = cards_to_int_3[hi->board[0]][hi->board[1]][hi->board[2]];
  int turn = hi->board[3];
  int river = hi->board[4];
  int gs = t->gamestate;

  switch (gs)
    {
    case 1:
      start_slots = info->slots[gs-1];
      end_slots = &(info->slots[gs][flop*HANDS]);
      break;
    case 2:
      turn_i = cards_to_int_4[int_to_cards_3[flop].c1][int_to_cards_3[flop].c2][int_to_cards_3[flop].c3][turn];
      CardMask_RESET(tmpcm);
      CardMask_SET(tmpcm, turn);
      //printf("%i %i %i %i %i %i\n",int_to_cards_3[flop].c1,int_to_cards_3[flop].c2,int_to_cards_3[flop].c3, flop, turn, turn_i);
      //printf("%s\n", Deck_maskString(int_to_cardmask_4[turn_i]));
      //printf("%s\n", Deck_maskString(int_to_cardmask_3[flop]));
      //printf("%s\n", Deck_maskString(tmpcm));

      
      start_slots = (short int*)&(info->slots[gs-1][flop*HANDS]);
      end_slots = (short int*)&(info->slots[gs][turn_i*HANDS]);
      break;
    case 3:
      turn_i = cards_to_int_4[int_to_cards_3[flop].c1][int_to_cards_3[flop].c2][int_to_cards_3[flop].c3][turn];
      start_slots = (short int*)&(info->slots[gs-1][turn_i*HANDS]);
      end_slots = (short int *) malloc(sizeof(short int)*HANDS);

      //printf("getting end slots %i %i %i\n", flop, turn,river);
      get_slots_for_river(info, end_slots, flop, turn, river);
      //printf("done geting slots\n");
      break;
    default:
      start_slots = NULL;
      end_slots = NULL;
      break;
    }
  n_start_types = info->n_htfb[gs-1];
  n_end_types = info->n_htfb[gs];

  tmp_bt = (float *)malloc(sizeof(float)*n_start_types*n_end_types);
  memset(tmp_bt, 0, sizeof(float)*n_start_types*n_end_types);
  memset(bt, 0, sizeof(float)*n_start_types*n_end_types);
  for (i = 0; i < HANDS; i++)
    {
      if (start_slots[i] == -1 || end_slots[i] == -1)
	{
	  if (start_slots[i] == -1 && end_slots[i] != -1)
	    printf("EI PITAS TANNE KOSKAAN %i %i\n", start_slots[i], end_slots[i]);
	  
	  continue;	
	 
	}
      tmp_bt[start_slots[i]*n_end_types + end_slots[i]] += 1.0;
    }
  tot = 0;
  for (i = 0; i < n_start_types; i++)
    {
      for (j = 0; j < n_end_types; j++)
	{
	  /*if (start_slots[i] == -1 || end_slots[i] == -1)
	    {
	      if (start_slots[i] == -1 && end_slots[i] != -1)
		printf("EI PITAS TANNE KOSKAAN\n");
	      else
		continue;	
		}*/

	  if ((g_max = tmp_bt[i*n_end_types+j]) != 0)
	    {
	      //printf("gmax: %f\n", g_max);
	      tot += g_max;
	      add_start_end_pair_to_board_type(info, bt, gs, i, j, g_max);
	    }
	}
    }
  //normalize type
  for (i = 0; i < n_start_types; i++)
    {
      tot1 = 0;
      for (j = 0; j < n_end_types; j++)
	{
	  tot1 += bt[i*n_end_types+j];
	}
      if (tot1 != 0)
	{
	  multi = 1.0/tot1;
	  for (j = 0; j < n_end_types; j++)
	    {
	      bt[i*n_end_types+j] *= multi;
	    }
	}
    }

  free(tmp_bt);
  if (gs == 3)
    free(end_slots);
}


int fill_diffs_lookup(struct type_type *t, float *type)
{
  float diffs[t->n_types], lowest_diff = 10000000000.0;
  float slot_div;
  int i, lowest_i = -1;
  int lookup_slot_size = bitfield_bytesize(t->n_types)/sizeof(uint64_t);
  int changed = 0;
  int slot;
  uint64_t *lookup;

  for (i = 0; i < t->n_types; i++)
    {
      diffs[i] = get_difference_pow2(&t->types[i*t->n_items_per_type], type, t->n_items_per_type);
      if (diffs[i] < lowest_diff)
	{
	  lowest_diff = diffs[i];
	  lowest_i = i;
	}
    }
  for (i = 0; i < t->n_types; i++)
    {
      slot_div = t->diffs[i*t->n_types + t->n_types-2]/10.0f;
      slot = diffs[i]/slot_div;
      if (slot > 9)
	slot = 9;
      lookup = &t->diffs_lookup[i*lookup_slot_size*10 + lookup_slot_size*slot];
      if (!is_bit_set(lookup, lowest_i))
	changed++;
      set_bit(lookup, lowest_i);
      
      if (diffs[i]/slot_div - floor(diffs[i]/slot_div) > 0.5 && slot < 9)
	slot++;
      else if (diffs[i]/slot_div - floor(diffs[i]/slot_div) < 0.5 && slot > 0)
	slot--;
      lookup = &t->diffs_lookup[i*lookup_slot_size*10 + lookup_slot_size*slot];
      if (!is_bit_set(lookup, lowest_i))
	changed++;
      set_bit(lookup, lowest_i);
      

    }
  return changed;
}

int16_t get_slot_tt(struct type_type *t, float *type)
{
  float slot_div, diff, smallest_diff = 10000000000.0;
  int lookup_slot_size = bitfield_wordsize(t->n_types);
  int slot;
  int16_t smallest_i = -1;
  uint64_t *lookup;
  int i = 0;
  
  uint64_t *bf = malloc(bitfield_bytesize(t->n_types));
  memset(bf, 0xff, bitfield_bytesize(t->n_types));
  i = -1;
  while ((i = biterate(bf, i, t->n_types)) != -1)
    {
      /* do shit */
      diff = get_difference_pow2(&t->types[i*t->n_items_per_type], type, t->n_items_per_type);
      if (diff < smallest_diff)
	{
	  smallest_diff = diff;
	  smallest_i = i;
	}
      slot_div = t->diffs[i*t->n_types + t->n_types-2]/10.0f;
      slot = diff/slot_div;
      if (slot > 9)
	slot = 9;
      lookup = &t->diffs_lookup[i*lookup_slot_size*10 + lookup_slot_size*slot];
      bitfield_and(bf, bf, lookup, t->n_types);
    }
  free(bf);
  return smallest_i;
}

void tt_call_gen_type(struct type_type *t, struct hand_info *hi, float *type)
{
  t->gen_type(t, hi, type);
}


void find_two_smallest(float *diffs, short int *diffs_order, int n_diff, int new_type)
{
  float s1 = 1000000000, s2 = 1000000000;
  int s1_i = -1, s2_i = -1;
  int i,j;


  for (j = 0; j < n_diff; j++)
    {
      if (j == new_type || diffs_order[j*n_diff] == new_type || diffs_order[j*n_diff + 1] == new_type || diffs[j*n_diff + diffs_order[j*n_diff + 1]] >= diffs[j*n_diff + new_type])
	{     
	  s1 = s2 = 1000000000;
	  s1_i = s2_i = -1;
	  for (i = 0; i < n_diff; i++)
	    {
	      if (i == j)
		continue;
	      //if (diffs[j*n_diff] > 1000000000)
	      //	printf("OHO %f\n", diffs[j*n_diff]);
	      if (diffs[j*n_diff + i] < s1 || s1_i == -1)
		{
		  s2 = s1;
		  s1 = diffs[j*n_diff + i];
		  s2_i = s1_i;
		  s1_i = i;
		}
	      else if (diffs[j*n_diff + i] < s2 || s2_i == -1)
		{
		  s2 = diffs[j*n_diff + i];
		  s2_i = i;
		}
	    }
	  diffs_order[j*n_diff] = s1_i;
	  diffs_order[j*n_diff + 1] = s2_i;
	}
      if (diffs[j*n_diff + diffs_order[j*n_diff]] > diffs[j*n_diff + diffs_order[j*n_diff + 1]] && diffs_order[j*n_diff + 1] != j)
	{
	  printf("JEPJEPJEPJEPJEPJE %i %i %i %f %f \n", j, diffs_order[j*n_diff],diffs_order[j*n_diff+1],diffs[j*n_diff + diffs_order[j*n_diff]], diffs[j*n_diff + diffs_order[j*n_diff + 1]]);
	}
    }
}
  
void add_values_to_gs_switch_and_odds(float *switch_table, float *odds_table, short int *hss, short int *hse, int types_end)
{
  int j;
  
  // printf("end %f\n", odds_table[50]);
  
  for (j = 0; j < HANDS; j++)
    {
      if (hse[j] == -1)
	continue;
      //printf("hss %i, hse %i\n", hss[j], hse[j]);
      if (switch_table != NULL)
	switch_table[hss[j]*types_end+hse[j]] += 1.0/1024.0;
      odds_table[hse[j]] += 1.0/1024.0;
    }
}


int get_bets_from_hand_info(struct hand_info *hi, int gamestate)
{
  int i = 0;
  int bets = 0;
  while (hi->acts[gamestate][i] != 0)
    {
      if (hi->acts[gamestate][i] == 0 || hi->acts[gamestate][i] == 0)
	bets++;
      i++;
    }
  return bets;
}

int get_last_act_from_hand_info(struct hand_info *hi, int gamestate)
{
  int seat, i = 0;

  if (gamestate == 0)
    seat = 1;
  else
    seat = 0;
  
  while (hi->acts[gamestate][i] != 0)
    {
      seat = (seat+1)%2;
      i++;
    }
  return seat;
}

struct unique_root *get_unique_state_from_hand_info(struct hand_info *hi, struct unique_root *root)
{
  int i = 0,j = 0;

  for (i = 0; i < 4; i++)
    {
      for (j = 0; j < hi->acts_i[i];j++)
	{
	  root = root->next[hi->acts[i][j]];
	}
    }
  return root;
}

void add_action_to_hi(struct hand_info *hi, int action)
{
  hi->acts[hi->cur_us->gamestate][hi->acts_i[hi->cur_us->gamestate]] = action;
  hi->acts_i[hi->cur_us->gamestate]++;
  hi->cur_us = hi->cur_us->next[action];
}

void set_mutable_types_from_hand_info(struct gameinfo *info, struct hand_info *hi, int16_t *pub, int16_t *priv)
{
  int i;
    
  for (i = 1; i < 7; i++)
    {
      //fprintf(stderr, "getting st %i %i\n", i, info->type_types[1][i].get_slot);
      pub[i] = info->type_types[1][i].get_slot(&info->type_types[1][i], hi);
      //fprintf(stderr, "got st %i\n", i);
    }
  
}
  

void set_st_from_hand_info(struct gameinfo *info, struct hand_info *hi, int16_t *pub, int16_t *priv)
{
  int i;
  
  
  for (i = 0; i < info->n_type_types[1]; i++)
    {
      //fprintf(stderr, "getting st %i %i\n", i, info->type_types[1][i].get_slot);
      if (pub[i] == -1)
	pub[i] = info->type_types[1][i].get_slot(&info->type_types[1][i], hi);
      //fprintf(stderr, "got st %i\n", i);
    }

  for (i = 0; i < info->n_type_types[0]; i++)
    {
      if (priv[i] == -1)
	priv[i] = info->type_types[0][i].get_slot(&info->type_types[0][i], hi);
    }
  
}

struct hand_info *get_blank_hi(struct unique_root *us)
{
  struct hand_info *hi = calloc(1, sizeof(struct hand_info));
  
  hi->cur_us = us;
  hi->root_us = us;
  memset(hi->hole_cards, 0xff, sizeof(hi->hole_cards));
  memset(hi->board, 0xff, sizeof(hi->board));
  return hi;
}

int16_t __get_potsize_slot_from_hi(struct type_type *t, struct hand_info *hi)
{
  int i, j;
  double potsize = 0;
  struct unique_root *root = hi->root_us;
  for (i = 0; i < 4; i++)
    {
      for (j = 0; j < hi->acts_i[i];j++)
	{
	 
	  potsize += root->action_cost[hi->acts[i][j]];
	  root = root->next[hi->acts[i][j]];
	}
    }
  return (int16_t)round(potsize);
}

int16_t get_potsize_slot_from_hi(struct type_type *t, struct hand_info *hi)
{
  int i, j;
  double potsize = 0;
  struct unique_root *root = hi->root_us;
  for (i = 0; i < 4; i++)
    {
      for (j = 0; j < hi->acts_i[i];j++)
	{	 
	  //potsize += root->action_cost[hi->acts[i][j]];
	  if (root->next[hi->acts[i][j]]->gamestate > i)
	    potsize += root->n_plr*root->bets[root->n_plr-1];    
	  root = root->next[hi->acts[i][j]];
	}
    }
  return (int16_t)potsize;
}


int16_t get_last_act_slot_from_hi(struct type_type *t, struct hand_info *hi)
{
  int i, j;
  struct unique_root *root = hi->root_us;

  for (i = 0; i < 4; i++)
    {
      for (j = 0; j < hi->acts_i[i];j++)
	{
	  if (root->next[hi->acts[i][j]]->gamestate != root->gamestate)
	    if (root->gamestate == t->gamestate-1)
	      return root->cur_seat;
	  root = root->next[hi->acts[i][j]];
	}
    }
  return -1;
}

int16_t get_bets_slot_from_hi(struct type_type *t, struct hand_info *hi)
{
    
  int i = 0;
  int bets = 0;
  
  if (hi->acts_i[t->gamestate-1] == 0 || t->gamestate > hi->cur_us->gamestate)
    return -1;
  if (t->gamestate-1 == 0)
    bets = -1;
  for (i = 0; i < hi->acts_i[t->gamestate-1]; i++)
    {
      if (hi->acts[t->gamestate-1][i] == 0)
	bets++;
      //i++;
    }
  return bets;
}

int16_t get_board_slot_from_hi(struct type_type *t, struct hand_info *hi)
{
  int i;
  int flop, turn;
  int16_t retval;
  
  //  if (hi->cur_us->gamestate < t->gamestate)
  // return -1;
  
  for (i = 0; i < t->gamestate+2; i++)
    assert(hi->board[i] >= 0 && hi->board[i] < 52);

  flop = cards_to_int_3[hi->board[0]][hi->board[1]][hi->board[2]];
  if (t->slots != NULL)
    {
      switch (t->gamestate)
	{
	case 1:
	  return t->slots[flop];
	  break;
	case 2:
	  return t->slots[flop*52+51-hi->board[3]];
	  break;
	case 3:
	  turn =  cards_to_int_4[hi->board[0]][hi->board[1]][hi->board[2]][hi->board[3]];
	  return t->slots[turn*52+51-hi->board[4]];
	  break;
	default:
	  assert(0);
	  return -1;
	  break;
	}
    }
  else
    {
      float *tmp_t;
      assert(t->types != NULL);
      tmp_t = calloc(1, sizeof(float)*t->n_items_per_type);
      gen_board_type(t->info, tmp_t, t->gamestate, flop, hi->board[3], hi->board[4]);
      //t->gen_type(t, hi, tmp_t);
      retval = get_slot(t->types, tmp_t, t->n_types, t->n_items_per_type);
      //retval = get_slot_tt(t, tmp_t);
      free(tmp_t);
      return retval;
    }
}

int16_t get_board_combo_slot_from_hi(struct type_type *t, struct hand_info *hi)
{
  int i;
  int flop;
  int16_t retval;
  
  //if (hi->cur_us->gamestate < t->gamestate)
  //  return -1;
  
  for (i = 0; i < t->gamestate+2; i++)
    assert(hi->board[i] >= 0 && hi->board[i] < 52);

  flop = cards_to_int_3[hi->board[0]][hi->board[1]][hi->board[2]];
  if (t->slots != NULL)
    {
      switch (t->gamestate)
	{
	case 1:
	  return t->slots[flop];
	  break;
	case 2:
	  return t->slots[flop*52+hi->board[3]];
	  break;
	case 3:
	  
	  retval = t->slots[flop*52*52+ 52*hi->board[3] + hi->board[4]];
	  //assert(retval != -1);
	  return retval;
	  //turn =  cards_to_int_4[hi->board[0]][hi->board[1]][hi->board[2]][hi->board[3]];
	  //return t->slots[turn*52+51-hi->board[4]];
	  break;
	default:
	  assert(0);
	  return -1;
	  break;
	}
    }
  else
    {
      float *tmp_t;
      assert(0);
      assert(t->types != NULL);
      tmp_t = calloc(1, sizeof(float)*t->n_items_per_type);
      gen_board_type(t->info, tmp_t, t->gamestate, flop, hi->board[3], hi->board[4]);
      //t->gen_type(t, hi, tmp_t);
      retval = get_slot(t->types, tmp_t, t->n_types, t->n_items_per_type);
      //retval = get_slot_tt(t, tmp_t);
      free(tmp_t);
      return retval;
    }
}


int16_t get_hand_slot_from_hi(struct type_type *t, struct hand_info *hi)
{
  int i;
  int64_t flop, turn, hand, river_i;
  int16_t retval;

  if (hi->cur_us->gamestate < t->gamestate)
    return -1;
  
  if (t->gamestate > 0)
    {
      for (i = 0; i < t->gamestate+2; i++)
	assert(hi->board[i] >= 0 && hi->board[i] < 52);
      
    }
  
  hand = cards_to_int_2[hi->hole_cards[0]][hi->hole_cards[1]];

  if (t->slots != NULL)
    {
      switch (t->gamestate)
	{
	case 0:
	  return t->slots[hand];
	  break;
	case 1:
	  flop = cards_to_int_3[hi->board[0]][hi->board[1]][hi->board[2]];
	  return t->slots[flop*HANDS + hand];
	  break;
	case 2:
	  turn = cards_to_int_4[hi->board[0]][hi->board[1]][hi->board[2]][hi->board[3]];
	  return t->slots[turn*HANDS + hand];
	  break;
	case 3:
	  river_i = get_river_index(hi->board);
	  return t->slots[river_i*HANDS+hand];
	  break;
	default:
	  assert(0);
	  break;
	}
    }
  else
    {
      float *tmp_t;
      assert(t->types != NULL);
      tmp_t = calloc(1, sizeof(float)*t->n_items_per_type);
      flop = cards_to_int_3[hi->board[0]][hi->board[1]][hi->board[2]];
      switch (t->gamestate)
	{
	case 2:
	  gen_single_turn_type(t->info, tmp_t, flop, hi->board[3], hand);
	  break;
	case 3:
	  gen_single_river_type(t->info, tmp_t, flop, hi->board[3], hi->board[4], hand);
	  break;
	default:
	  assert(0);
	  break;
	}
      retval = get_slot(t->types, tmp_t, t->n_types, t->n_items_per_type);
      free(tmp_t);
      return retval;
    }
}

int16_t type_to_slot(uint64_t *types_bmap, int16_t type_i)
{
  return bitcount_before_bitmap(types_bmap, type_i);
}


int16_t slot_to_type(uint64_t *types_bmap, int16_t slot_i)
{
  return get_nth_set_bit_bitmap(types_bmap, slot_i);
}

int find_closest_type_diffs(float *diffs, uint64_t *types_bmap, int n_types, short int slot, int len)
{
  int last_i = -1;
  float smallest = 10000000;
  int smallest_i= -1;

  if (is_bit_set(types_bmap, slot))
    return slot;
  
  while ((last_i = biterate(types_bmap, last_i, n_types)) != -1)
    {
      if (diffs[slot*n_types+last_i] < smallest)
	{
	  smallest = diffs[slot*n_types+last_i];
	  smallest_i = last_i;
	}
    }
  return smallest_i;
}

int find_closest_type_diffs_order(short int *diffs_order, uint64_t *types_bmap, int n_types, short int slot)
{

  //prefetch_range(types_bmap, n_types/8);
  if (is_bit_set(types_bmap, slot))
    return slot;

  int i = 0;
  int16_t *local_diffs_order = &diffs_order[n_types*slot];

  while(!(is_bit_set(types_bmap, local_diffs_order[i])))
    {
      i++;
      //assert(i < n_types);
    }
  return local_diffs_order[i];
}


int find_closest_type_linear_int(uint64_t *types_bmap, int n_types, short int slot)
{
  short int lo = slot-1, hi = slot+1;

  if (is_bit_set(types_bmap, slot))
    return slot;
  
  while (1)
    {
      if (lo < 0)
	{
	  while (!is_bit_set(types_bmap, hi))
	    hi++;
	  return hi;
	}
      if (hi >= n_types)
	{
	  while (!is_bit_set(types_bmap, lo))
	    lo--;
	  return lo;
	}
      if (is_bit_set(types_bmap, lo))
	return lo;
      if (is_bit_set(types_bmap, hi))
	return hi;
      lo--;
      hi++;
    }

}

int find_closest_type_linear_float(uint64_t *types_bmap, int n_types, float slot)
{
  int int_slot = (int)roundf(slot);
  return find_closest_type_linear_int(types_bmap, n_types, int_slot);
}

int find_closest_type(struct type_type *tt, uint64_t *types_bmap, int16_t slot, int len)
{

  switch(tt->style)
    {
    case 0:
      return find_closest_type_linear_int(types_bmap, tt->n_types, slot);
      break;
    case 1:
      return find_closest_type_linear_int(types_bmap, tt->n_types, slot);
      break;
    case 2:
      /* res1 = find_closest_type_diffs(tt->diffs, types_bmap, tt->n_types, slot, len); */
      /* res2 = find_closest_type_diffs_order(tt->diffs_order, types_bmap, tt->n_types, slot); */
      /* assert(res1 == res2); */
      /* return res1; */
      if (tt->n_types/len > len)
      	return find_closest_type_diffs(tt->diffs, types_bmap, tt->n_types, slot, len);
      else
      	return find_closest_type_diffs_order(tt->diffs_order, types_bmap, tt->n_types, slot);
      break;
    default:
      return -1;
      break;
    }
}	


int16_t *find_n_closest_type_diffs(short int *diffs_order, uint64_t *types_bmap, int n_types, short int slot, int16_t *dest, int n_req)
{
  int i;
  int types_found = 0;
  //  int closest_found = 0;
  //uint64_t bit_set;
  int16_t *local_diffs_order = &diffs_order[n_types*slot];

  if (is_bit_set(types_bmap, slot))
    dest[types_found++] = slot;
    //return slot;
  i = 0;
  while(types_found < n_req)
    {
      assert(i < n_types);
      if (is_bit_set(types_bmap, local_diffs_order[i]))
	dest[types_found++] = local_diffs_order[i];
      i++;
    }
  return dest;
}


void find_n_closest_type_linear_int(uint64_t *types_bmap, int n_types, short int slot, int16_t *dest, int n_req)
{
  short int lo = slot-1, hi = slot+1;
  int types_found = 0;
  //int closest_found = 0;


  if (is_bit_set(types_bmap, slot))
    dest[types_found++] = slot;
  
  while (types_found < n_req)
    {
      if (lo < 0)
	{
	  while (types_found < n_req)
	    {
	      assert(hi < n_types);
	      if (is_bit_set(types_bmap, hi))
		dest[types_found++] = hi;
	      hi++;

	    }
	  break;
	}
      if (hi >= n_types)
	{
	  while (types_found < n_req)
	    {
	      assert(lo >= 0);
	      if (is_bit_set(types_bmap, lo))
		dest[types_found++] = lo;
	      lo--;

	    }
	  break;
	}
      assert(lo >= 0);
      assert(hi < n_types);
      
    
      if (is_bit_set(types_bmap, lo) && types_found < n_req)
	dest[types_found++] = lo;
      
      if (is_bit_set(types_bmap, hi) && types_found < n_req)
	dest[types_found++] = hi;
    
      hi++;    
      lo--;
    }

}

void find_n_closest_type(struct type_type *t, uint64_t *types_bmap, int16_t slot, int16_t *dest, int n_req)
{
  switch(t->style)
    {
    case 0:
      find_n_closest_type_linear_int(types_bmap, t->n_types, slot, dest, n_req);
      break;
    case 1:
      find_n_closest_type_linear_int(types_bmap, t->n_types, slot, dest, n_req);
      break;
    case 2:
      find_n_closest_type_diffs(t->diffs_order, types_bmap, t->n_types, slot, dest, n_req);
      break;
    default:
      break;
    }
}


int find_n_closest_type_diffs_valid_edge(short int *diffs_order, float *diffs, uint64_t *types_bmap, int n_types, short int slot, int16_t *dest, double *weights, int n_req)
{
  int i,j;
  int types_found = 0, types_tried = 0;
  //  int closest_found = 0;
  //uint64_t bit_set;
  int16_t *local_diffs_order = &diffs_order[n_types*slot];

  if (is_bit_set(types_bmap, slot))
    {
      dest[types_found] = slot;
      weights[types_found] = 0;
      types_found++;
      return types_found;
    }
  //return slot;
  i = 0;
  while(types_tried < n_req)
    {
      assert(i < n_types);
      if (is_bit_set(types_bmap, local_diffs_order[i]))
	{
	  for(j = 0; j < types_found; j++)
	    {
	      if (diffs[local_diffs_order[i]*n_types+slot] >= diffs[local_diffs_order[i]*n_types+dest[j]])
		break;
	    }
	  if (j == types_found)
	    {
	      dest[types_found] = local_diffs_order[i];
	      weights[types_found] = diffs[local_diffs_order[i]*n_types+slot];
	      types_found++;
	    }
	  types_tried++;
	}
      i++;
    }
  return types_found;
}

int find_n_closest_type_linear_int_valid_edge(uint64_t *types_bmap, int n_types, short int slot, int16_t *dest, double *weights, int n_req)
{
  short int lo = slot-1, hi = slot+1;
  int types_found = 0;
  //int closest_found = 0;


  if (is_bit_set(types_bmap, slot))
    {
      dest[types_found] = slot;
      weights[types_found] = 0;
      types_found++;
      return types_found;
    }
  while (hi < n_types)
    {
      if (is_bit_set(types_bmap, hi))
	{
	  dest[types_found] = hi;
	  weights[types_found] = abs(slot-hi);
	  types_found++;

	  break;
	}
      hi++;
    }
  while (lo >= 0)
    {
      if (is_bit_set(types_bmap, lo))
	{
	  dest[types_found] = lo;
	  weights[types_found] = abs(slot-lo);
	  types_found++;
	  break;
	}
      lo--;
    }
  return types_found;
}

int find_n_closest_type_valid_edge(struct type_type *t, uint64_t *types_bmap, int16_t slot, int16_t *dest, double *weights, int n_req)
{
  switch(t->style)
    {
    case 0:
      return find_n_closest_type_linear_int_valid_edge(types_bmap, t->n_types, slot, dest, weights, n_req);
      break;
    case 1:
      return find_n_closest_type_linear_int_valid_edge(types_bmap, t->n_types, slot, dest, weights, n_req);
      break;
    case 2:
      return find_n_closest_type_diffs_valid_edge(t->diffs_order, t->diffs, types_bmap, t->n_types, slot, dest, weights, n_req);
      break;
    default:
      break;
    }
  return -1;
}



double get_diff(struct type_type *t, int16_t slot1, int16_t slot2)
{
  switch(t->style)
    {
    case 0:
    case 1:
      return fabs((double)slot1 - (double)slot2);
      break;
    case 2:
      return t->diffs[slot1*t->n_types+slot2];
    default:
      break;
    }
  assert(0);
  return 1.0/0.0;
}


int16_t find_unset_type_with_smallest_combined_diff(struct type_type *t, uint64_t *types_bmap, uint64_t *valid_bmap, int16_t *cmp_types, int n_cmp_types)
{
  double smallest_diff = 100000000000, tmp_diff;
  int16_t retval = -1;
  int i, j;

  for (i = 0; i < t->n_types; i++)
    {
      if (!is_bit_set(types_bmap, i) && (valid_bmap == NULL || is_bit_set(valid_bmap, i)))
	{
	  tmp_diff = 0;
	  for (j = 0; j < n_cmp_types; j++)
	    tmp_diff += get_diff(t, i, cmp_types[j]);
	  if (tmp_diff < smallest_diff)
	    {
	      smallest_diff = tmp_diff;
	      retval = i;
	    }
	}
    }
  return retval;
}
 
  

void generate_mapping_to_slots(uint64_t *types_bmap, int16_t *mapping, struct type_type *t, int len)
{
  int i;


  for (i = 0; i < t->n_types; i++)
    {
      mapping[i] = find_closest_type(t, types_bmap, i, len);
      mapping[i] = bitcount_before_bitmap(types_bmap, mapping[i]);
    }
}

void generate_alt_mapping_to_slots(uint64_t *types_bmap, int16_t *mapping, struct type_type *t)
{
  int i;
  int16_t two_closest_type[2];

  for (i = 0; i < t->n_types; i++)
    {
      find_n_closest_type(t, types_bmap, i, two_closest_type, 2);
      mapping[i] = bitcount_before_bitmap(types_bmap, two_closest_type[1]);
    }
}

struct unique_root *set_mutable_types_from_path(int16_t *types, char *path, int path_len, struct unique_root *r)
{
  int i, act_i;
  int bets = 0;
  double potsize = 0;

  for (i = 0; i < path_len; i++)
    {
      act_i = path[i];
      if (act_i >= 48)
	act_i -= 48;
      if (act_i == 0 && r->root_idx != 0)
	bets++;
      if (r->next[(int)act_i]->gamestate != r->gamestate)
	{
	  //set types
	  if (r->gamestate < 3)
	    {
	      types[1+r->gamestate] = r->cur_seat;
	      types[4+r->gamestate] = bets;
	    }
	  //add potsize
	  bets = 0;
	}
      potsize += r->action_cost[(int)act_i];
      
      r = r->next[(int)act_i];
      if (r->gamestate == 4)
	break;
    }
  types[0] = (int)round(potsize);
  return r;
}


int cmp(const void *i1, const void *i2)
{
  return *((int8_t*)i2) - *((int8_t*)i1);
}

int get_skip(int start, int cur, int items)
{
  int i, f = 1, mul1, mul2;
  
  for (i = 2; i <= items; i++)
    f*=i;
  mul1 = start;
  for (i = start + 1 - items; i < start; i++)
    {
      mul1*=i;
    }
  mul2 = cur;
  for (i = cur + 1 - items; i < cur; i++)
    {
      mul2*=i;
    }
  return mul1/f - mul2/f;
}


int get_river_index(int8_t *board)
{
  int8_t b[5];
  int sum, start, i;
  memcpy(&b, board, sizeof(b));
  
  qsort(&b, 5, 1, &cmp);
  
  sum = 0;
  start = 52;

  for (i = 0; i < 5; i++)
    {
      sum += get_skip(start, b[i]+1, 5-i);
      start = b[i];
    }
  return sum;
}


void gen_types_data(struct gameinfo *info, int8_t board[5], struct types_data *td)
{
  //struct types_data *td = calloc(1, sizeof(struct types_data));
  //int16_t *preflop_types = NULL, *flop_types = NULL, *turn_types = NULL, *river_types = NULL;
  int64_t flop_i, turn_i, river_i;
  struct hand_info hi;
  int i,j,t,s;
  int16_t *types[4];
  int16_t n_t[4];
  int16_t *tc[4];
  int16_t tc_tot[4];

  memcpy(hi.board, board, sizeof(board));
  td->public_types[7] = info->type_types[1][7].get_slot(&info->type_types[1][7], &hi);
  td->public_types[8] = info->type_types[1][8].get_slot(&info->type_types[1][8], &hi);
  td->public_types[9] = info->type_types[1][9].get_slot(&info->type_types[1][9], &hi);


  types[0] = info->type_types[0][0].slots;

  n_t[0] = info->type_types[0][0].n_types;
  tc[0] = calloc(n_t[0],sizeof(int16_t));
  tc_tot[0] = 0;

  flop_i = cards_to_int_3[board[0]][board[1]][board[2]];
  types[1] = &info->type_types[0][1].slots[flop_i*HANDS];
  
  n_t[1] = info->type_types[0][1].n_types;
  tc[1] = calloc(n_t[1],sizeof(int16_t));
  tc_tot[1] = 0;

  turn_i = cards_to_int_4[board[0]][board[1]][board[2]][board[3]];
  types[2] = &info->type_types[0][2].slots[turn_i*HANDS];
  n_t[2] = info->type_types[0][2].n_types;
  tc[2] = calloc(n_t[2],sizeof(int16_t));
  tc_tot[2] = 0;

  river_i = get_river_index(board);
  types[3] = &info->type_types[0][3].slots[river_i*HANDS];
  n_t[3] = info->type_types[0][3].n_types;
  tc[3] = calloc(n_t[3],sizeof(int16_t));
  tc_tot[3] = 0;

  j = 0;  
  for (i = 0; i < HANDS; i++)
    {    
      if (types[3][i] == -1)
	continue;
      for (t = 0; t < 4; t++)
	{
	  td->private_types[t][j] = types[t][i];
	  if (tc[t][types[t][i]] == 0)
	    tc_tot[t]++;
	  tc[t][types[t][i]]++;
	}
      /* td->private_types[0][j] = preflop_types[i]; */
      /* if (pft_c[preflop_types[i]] == 0) */
      /* 	pft_c_tot++; */
      /* pft_c[preflop_types[i]]++; */

      /* td->private_types[1][j] = flop_types[i]; */
      /* if (ft_c[flop_types[i]] == 0) */
      /* 	ft_c_tot++; */
      /* ft_c[flop_types[i]]++; */

      /* td->private_types[2][j] = turn_types[i]; */
      /* if (tt_c[turn_types[i]] == 0) */
      /* 	tt_c_tot++; */
      /* tt_c[turn_types[i]]++; */

      /* td->private_types[3][j] = river_types[i]; */
      /* if (rt_c[river_types[i]] == 0) */
      /* 	rt_c_tot++; */
      /* rt_c[river_types[i]]++; */

      j++;
    }
  assert(j == SAMPLES);

  get_river_hand_hv2_all(flop_i, board[3], board[4], td->vals);
  
  for (t = 0; t < 4; t++)
    {
      td->tc[t] = calloc(tc_tot[t], sizeof(struct type_count));
      td->n_types[t] = tc_tot[t];
      j = 0;
      for (i = 0; i < n_t[t]; i++)
	{
	  if (tc[t][i] == 0)
	    continue;
	  assert(j < tc_tot[t]);

	  td->tc[t][j].type = i;
	  td->tc[t][j].count = tc[t][i];
	  td->tc[t][j].mapping = calloc(tc[t][i], sizeof(int16_t));
	  j++;
	  
	}
      
      for (i = 0; i < tc_tot[t]; i++)
	{
	  j = 0;
	  for (s = 0; s < SAMPLES;s++)
	    {
	      if (td->private_types[t][s] == td->tc[t][i].type)
		{
		  td->tc[t][i].mapping[j] = s;
		  j++;
		}
	      if (j == td->tc[t][i].count)
		break;
	    }
	  assert (j == td->tc[t][i].count);
	}
    }
  for (t = 0; t < 4; t++)
    {
      free(tc[t]);
    }
}

void free_type_count_from_types_data(struct types_data *td)
{
  int t, i;

  for (t = 0; t < 4; t++)
    {
      for (i = 0; i < td->n_types[t]; i++)
	free(td->tc[t][i].mapping);
      free(td->tc[t]);
    }
}

 
int get_board_type(struct gameinfo *info, int gamestate, int8_t *board)
{
  int flop = cards_to_int_3[board[0]][board[1]][board[2]];
  switch(gamestate)
    {
    case 0:
      return 0;
      break;
    case 1:
      return info->b_slots[1][flop];
      break;
    case 2:
      return info->b_slots[2][flop*52+board[3]];
      break;
    case 3:
      return info->b_slots[3][flop*52*52+ 52*board[3] + board[4]];
      break;
    default:
      return 0;
      break;
    }
}
      
int16_t get_hand_type(struct gameinfo *info, int gamestate, int8_t *board, int hand)
{
  int64_t flop, turn, river_i;
  switch (gamestate)
    {
    case 0:
      return info->slots[0][hand];
      break;
    case 1:
      flop = cards_to_int_3[board[0]][board[1]][board[2]];
      return info->slots[1][flop*HANDS + hand];
      break;
    case 2:
      turn = cards_to_int_4[board[0]][board[1]][board[2]][board[3]];
      return info->slots[2][turn*HANDS + hand];
      break;
    case 3:
      river_i = get_river_index(board);
      return info->slots[3][river_i*HANDS+hand];
      break;
    default:
      assert(0);
      break;
    }
}

int16_t *get_hand_types(struct gameinfo *info, int gamestate, int8_t *board)
{
  int64_t flop, turn, river_i;
  struct hand_info hi;

  switch (gamestate)
    {
    case 0:
      assert(info->type_types[0][0].slots == info->slots[0]);
      return info->slots[0];
      break;
    case 1:
      flop = cards_to_int_3[board[0]][board[1]][board[2]];
      assert(&info->type_types[0][1].slots[flop*HANDS] == &info->slots[1][flop*HANDS]);
      return &info->slots[1][flop*HANDS];
      break;
    case 2:
      turn = cards_to_int_4[board[0]][board[1]][board[2]][board[3]];
      assert( &info->slots[2][turn*HANDS] == &info->type_types[0][2].slots[turn*HANDS]);
      return &info->slots[2][turn*HANDS];
      break;
    case 3:
      river_i = get_river_index(board);
      assert(&info->slots[3][river_i*HANDS] == &info->type_types[0][3].slots[river_i*HANDS]);
      return &info->slots[3][river_i*HANDS];
      break;
    default:
      assert(0);
      break;
    }

}

void get_hand_hv2(int8_t *board, struct hand_hv2 *vals)
{
  get_river_hand_hv2_all_hands(cards_to_int_3[board[0]][board[1]][board[2]], board[3], board[4], vals);
}
