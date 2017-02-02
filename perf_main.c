#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <poker_defs.h>
#include <inlines/eval.h>

//#include "defs.h"
#include "precalc_conversions.h"
//#include "handval.h"
#include "perf_main.h"
#include "zbtree.h"

/*extern int cards_to_int_4[52][52][52][52];
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

struct path_nfo
{
  double potsize;
  double tocall;
  double toraise;
  double stake[2];
  int32_t gamestate;
  int32_t seat;
  int32_t actions[3];
  int32_t path_i;
  int32_t bets;
  int8_t path[32];
};



struct node
{
  double *regs;
  int32_t *sm;
  int32_t n_regs;
  int8_t path[32];
  int8_t board[5];
};

struct board_data
{
  int32_t *flop;
  int32_t *turn[52];
  int32_t *river[52][52];
  struct hand_hv2 *hv[52][52];
};
*/

int compare_hv2(const void *m1, const void *m2) 
{
  return ((struct hand_hv *)m1)->hv - ((struct hand_hv *)m2)->hv;
}


struct hand_hv2 *get_river_hand_hv2(int8_t b[5])
{
  int i, hand_i, hv_count, flop;
  //uint16_t cur_hv2;
  //double w,t;
  CardMask board, hand, hand1;
  HandVal hv1, cur_hv;
  struct hand_hv tmp_hv[HANDS];
  struct hand_hv2 *retval = malloc(sizeof(struct hand_hv2)*HANDS);

  flop = cards_to_int_3[b[0]][b[1]][b[2]];

  board = int_to_cardmask_3[flop];

  CardMask_SET(board, b[3]);
  CardMask_SET(board, b[4]);
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
  return retval;
  //assert(cur_hv2 == HANDS-1);

}

CardMask cardmask_from_board(int8_t *board, int n_board)
{
  int i;
  CardMask b;
  CardMask_RESET(b);
  for (i = 0; i < n_board; i++)
    {
      CardMask_SET(b, board[i]);
    }
  return b;
}

struct next_cards *gen_next_cards_list(int8_t *board, int n_board)
{
  struct next_cards *l = malloc(sizeof(struct next_cards));
  int8_t *suits = get_suits(board, n_board);
  int8_t *mapping = get_suit_mapping(suits, n_board);
  int max_suit = get_max_suit(suits, n_board);
  CardMask b = cardmask_from_board(board, n_board);
  int i, c = 0;
  int next_suit = max_suit == 3?3:max_suit+1;
  l->len = (13*(next_suit+1))-n_board;
  l->cards = malloc(sizeof(int)*l->len);
  l->multi = malloc(sizeof(double)*l->len);
  
  
  for (i = 0; i < next_suit*13; i++)
    {
      if (CardMask_CARD_IS_SET(b, i))
	continue;
      l->cards[c] = i;
      l->multi[c] = 1.0;
      c++;
    }
  
  for (i = next_suit*13; i < (next_suit+1)*13; i++)
    {
      if (CardMask_CARD_IS_SET(b, i))
	continue;
      l->cards[c] = i;
      l->multi[c] = 4.0-(double)next_suit;
      c++;
    }
  
  
  assert(c == l->len);
  free(suits);
  free(mapping);
  return l;
}
  
void free_next_cards_list(struct next_cards *l)
{
  free(l->cards);
  free(l->multi);
  free(l);
}

struct hand_hv2 *get_hand_hv_for_board(int8_t *board, int n_board, struct board_data *bd)
{
  return bd->hv[board[3]][board[4]];
}


int32_t *get_sm_for_board(int8_t *board, int n_board, struct board_data *bd)
{
  switch(n_board)
    {
    case 3:
      return bd->flop;
      break;
    case 4:
      return bd->turn[board[3]];
      break;
    case 5:
      return bd->river[board[3]][board[4]];
      break;
    default:
      assert(0);
      return NULL;
      break;
    }
  return NULL;
}

struct board_data *gen_board_data(int8_t *flop)
{
  struct board_data *d = calloc(1,sizeof(struct board_data));
  struct next_cards *tl, *rl;
  int8_t board[5];
  int i,j;
  memcpy(board, flop, 3);
  d->flop = get_slot_mapping(flop, 3);
  tl = gen_next_cards_list(flop, 3);
  for (i = 0; i < tl->len; i++)
    {
      board[3] = tl->cards[i];
      d->turn[board[3]] = get_slot_mapping(board, 4);
      rl = gen_next_cards_list(board,4);
      for (j = 0; j < rl->len; j++)
	{
	  board[4] = rl->cards[j];
	  d->river[board[3]][board[4]] = get_slot_mapping(board, 5);
	  d->hv[board[3]][board[4]] = get_river_hand_hv2(board);
	}
      free(rl);
    }
  free(tl);
  return d;
}
      	
struct path_nfo *gen_start_path_nfo()
{
  struct path_nfo *nfo = calloc(1,sizeof(struct path_nfo));
  
  nfo->potsize = 1.5;
  nfo->tocall = 0.5;
  nfo->toraise = 1.5;
  nfo->stake[0] = 1.0;
  nfo->stake[1] = 0.5;
  nfo->gamestate = 0;
  nfo->blen = 0;
  nfo->seat = 1;
  nfo->actions[0] = 1;
  nfo->actions[1] = 1;
  nfo->actions[2] = 1;
  nfo->path_i = 0;
  nfo->bets = 1;
  return nfo;
}

int8_t *add_card_to_board(int8_t *board, int n_board, int8_t card)
{
  int8_t *new_board = malloc(sizeof(int8_t)*(n_board+1));
  memcpy(new_board, board, sizeof(int8_t)*n_board);
  new_board[n_board] = card;
  return new_board;
}

struct path_nfo *add_act_to_path_nfo(struct path_nfo *nfo, int32_t act)
{
  struct path_nfo *new_nfo = malloc(sizeof(struct path_nfo));
  memcpy(new_nfo, nfo, sizeof(struct path_nfo));
  
  switch(act)
    {
    case 0:
      new_nfo->potsize += new_nfo->toraise;
      new_nfo->stake[new_nfo->seat] += new_nfo->toraise;
      new_nfo->tocall = new_nfo->toraise-new_nfo->tocall;
      new_nfo->toraise = new_nfo->tocall*2;
      new_nfo->seat = (new_nfo->seat+1)%2;
      new_nfo->bets += 1;
      break;
    case 1:
      new_nfo->potsize += new_nfo->tocall;
      new_nfo->stake[new_nfo->seat] += new_nfo->tocall;
      new_nfo->tocall = 0;
      if ((new_nfo->bets == 0 && new_nfo->gamestate > 0 && new_nfo->seat == 0) || (new_nfo->bets == 1 && new_nfo->seat == 1 && new_nfo->gamestate == 0))
	new_nfo->seat = (new_nfo->seat+1)%2;
      else
	{
	  new_nfo->gamestate += 1;
	  new_nfo->seat = 0;
	  new_nfo->bets = 0;
	}
      new_nfo->toraise = ((double)(new_nfo->gamestate/2))+1.0;
      break;
    case 2:
      new_nfo->gamestate += 10;
      new_nfo->seat = (new_nfo->seat+1)%2;
      new_nfo->toraise = 0;
      new_nfo->tocall = 0;
      break;
    }
  if (new_nfo->gamestate >= 4)
    {
      new_nfo->actions[0] = 0;
      new_nfo->actions[1] = 0;
      new_nfo->actions[2] = 0;
    }
  else if (new_nfo->bets == 4)
    {
      new_nfo->actions[0] = 0;
      new_nfo->actions[1] = 1;
      new_nfo->actions[2] = 1;
    }
  else if (new_nfo->tocall == 0)
    {
      new_nfo->actions[0] = 1;
      new_nfo->actions[1] = 1;
      //new_nfo->actions[2] = 0;
      new_nfo->actions[2] = 1;
    }
  else
    {
      new_nfo->actions[0] = 1;
      new_nfo->actions[1] = 1;
      new_nfo->actions[2] = 1;
    }
  new_nfo->path[new_nfo->path_i] = act;
  new_nfo->path_i++;
  if (new_nfo->gamestate > 0 && new_nfo->gamestate < 4)
    new_nfo->blen = new_nfo->gamestate +2;
  
  return new_nfo;
}

struct path_nfo *add_act_to_path_nfo_2bet_max(struct path_nfo *nfo, int32_t act)
{
  struct path_nfo *new_nfo = malloc(sizeof(struct path_nfo));
  memcpy(new_nfo, nfo, sizeof(struct path_nfo));
  assert(nfo->actions[act]);
  switch(act)
    {
    case 0:
      new_nfo->potsize += new_nfo->toraise;
      new_nfo->stake[new_nfo->seat] += new_nfo->toraise;
      new_nfo->tocall = new_nfo->toraise-new_nfo->tocall;
      new_nfo->toraise = new_nfo->tocall*2;
      new_nfo->seat = (new_nfo->seat+1)%2;
      new_nfo->bets += 1;
      break;
    case 1:
      new_nfo->potsize += new_nfo->tocall;
      new_nfo->stake[new_nfo->seat] += new_nfo->tocall;
      new_nfo->tocall = 0;
      if ((new_nfo->bets == 0 && new_nfo->gamestate > 0 && new_nfo->seat == 0) || (new_nfo->bets == 1 && new_nfo->seat == 1 && new_nfo->gamestate == 0))
	new_nfo->seat = (new_nfo->seat+1)%2;
      else
	{
	  new_nfo->gamestate += 1;
	  new_nfo->seat = 0;
	  new_nfo->bets = 0;
	}
      new_nfo->toraise = ((double)(new_nfo->gamestate/2))+1.0;
      break;
    case 2:
      new_nfo->gamestate += 10;
      new_nfo->seat = (new_nfo->seat+1)%2;
      new_nfo->toraise = 0;
      new_nfo->tocall = 0;
      break;
    }
  if (new_nfo->gamestate >= 4)
    {
      new_nfo->actions[0] = 0;
      new_nfo->actions[1] = 0;
      new_nfo->actions[2] = 0;
    }
  else if (new_nfo->bets == 4)
    {
      new_nfo->actions[0] = 0;
      new_nfo->actions[1] = 1;
      new_nfo->actions[2] = 1;
    }
  else if (new_nfo->tocall == 0)
    {
      new_nfo->actions[0] = 1;
      new_nfo->actions[1] = 1;
      new_nfo->actions[2] = 0;
      //new_nfo->actions[2] = 1;
    }
  else
    {
      new_nfo->actions[0] = 1;
      new_nfo->actions[1] = 1;
      new_nfo->actions[2] = 1;
    }
  new_nfo->path[new_nfo->path_i] = act;
  new_nfo->path_i++;
  if (new_nfo->gamestate > 0 && new_nfo->gamestate < 4)
    new_nfo->blen = new_nfo->gamestate +2;
  
  return new_nfo;
}


int8_t *sort_flop(int8_t *board, int n_board)
{
  int8_t *new_board = malloc(sizeof(int8_t)*n_board);
  int8_t tmp_card;
  memcpy(new_board, board, sizeof(int8_t)*n_board);
  
  if (new_board[1]%13 > new_board[0]%13 && new_board[1]%13 > new_board[2]%13)
    {
      tmp_card = new_board[0];
      new_board[0] = new_board[1];
      new_board[1] = tmp_card;
    }
  else if (new_board[2]%13 > new_board[0]%13 && new_board[2]%13 > new_board[1]%13)
    {
      tmp_card = new_board[0];
      new_board[0] = new_board[2];
      new_board[2] = tmp_card;
    }
  if (new_board[2]%13 > new_board[1]%13)
    {
      tmp_card = new_board[1];
      new_board[1] = new_board[2];
      new_board[2] = tmp_card;
    }
  return new_board;
}

int8_t *get_suits(int8_t *board, int n_board)
{
  int8_t *suits = malloc(sizeof(int8_t)*n_board);
  int i;

  for (i = 0; i < n_board; i++)
    {
      suits[i] = board[i]/13;
    }
  return suits;
}

int8_t *get_suit_mapping(int8_t *suits, int n_board)
{
  int next_suit = 0, i;
  int8_t *suit_mapping = malloc(sizeof(int8_t)*4);
  memset(suit_mapping, 0xff, sizeof(int8_t)*4);
  
  if (n_board >= 3) //FLOP
    {
      if (suits[0] == suits[1] && suits[1] == suits[2])
	{
	  suit_mapping[suits[0]] = 0;
	  next_suit = 1;
	}
      else if (suits[0] != suits[1] && suits[0] != suits[1] && suits[1] != suits[2])
	{
	  suit_mapping[suits[0]] = 0;
	  suit_mapping[suits[1]] = 1;
	  suit_mapping[suits[2]] = 2;
	  next_suit = 3;
	}
      else
	{
	  if (suits[0] == suits[1])
	    {
	      suit_mapping[suits[0]] = 0;
	      suit_mapping[suits[2]] = 1;
	    }
	  else if (suits[0] == suits[2])
	    {
	      suit_mapping[suits[0]] = 0;
	      suit_mapping[suits[1]] = 1;
	    }
	  else if (suits[1] == suits[2])
	    {
	      suit_mapping[suits[1]] = 0;
	      suit_mapping[suits[0]] = 1;
	    }
	  next_suit = 2;
	}
    }
  if (n_board >= 4)
    {
      if (suit_mapping[suits[3]] == -1)
	{
	  suit_mapping[suits[3]] = next_suit;
	  next_suit++;
	}
    }
  if (n_board == 5)
    {
      if (suit_mapping[suits[4]] == -1)
	{
	  suit_mapping[suits[4]] = next_suit;
	  next_suit++;
	}
    }
  for (i = 0; i < 4; i++)
    if (suit_mapping[i] == -1)
      {
	assert(next_suit < 4);
	suit_mapping[i] = next_suit;
      }
  return suit_mapping;
}

int get_max_suit(int8_t *suits, int n_board)
{
  int i, max = 0;
  for (i = 0; i < n_board; i++)
    {
      if (suits[i] > max)
	max = suits[i];
    }
  return max;
}

int8_t morph_card(int8_t c, int8_t *mapping)
{
  return mapping[c/13]*13+c%13;
}


int get_max_slot_from_sm(int32_t *sm)
{
  int max_slot = 0;
  int i;

  for (i = 0; i < HANDS; i++)
    {
      if (sm[i] > max_slot)
	max_slot = sm[i];
    }
  assert(max_slot < HANDS && max_slot >=0);
  return max_slot;
}

int32_t *get_slot_mapping(int8_t *board, int n_board)
{
  int i,j;
  int32_t *sm = malloc(sizeof(int32_t)*HANDS);
  CardMask b;

  CardMask_RESET(b);
  for (i = 0; i < n_board; i++)
    {
      CardMask_SET(b, board[i]);
    }

  if (n_board == 5)
    {
      struct hand_hv2 *hv;
      
      hv = get_river_hand_hv2(board);
      for (i = 0; i < HANDS; i++)
	{
	  if (hv[i].c[0] == -1)
	    {
	      sm[hv[i].sample_i] = -1;
	      continue;
	    }
	  sm[hv[i].sample_i] = hv[i].hv;
	}
      free(hv);
      //RIVER

    }
  else
    {
      int8_t *suits = get_suits(board, n_board);
      int8_t *mapping = get_suit_mapping(suits, n_board);
      int max_suit = get_max_suit(suits, n_board)+1;
      int8_t tmp_c;
      for (i = 0; i < HANDS; i++)
	{
	  int8_t new_c1, new_c2;
	  new_c1 = morph_card(int_to_cards_2[i].c1, mapping);
	  new_c2 = morph_card(int_to_cards_2[i].c2, mapping);
	  if (new_c1/13 == max_suit && new_c2/13 == max_suit)
	    {
	      if (int_to_cards_2[i].c1/13 != int_to_cards_2[i].c2/13)
		{
		  if (new_c1 >= new_c2)
		    new_c1+=13;
		  else
		    new_c2+=13;
		}
	    }
	  /* if (new_c2 > new_c1) */
	  /*   { */
	  /*     tmp_c = new_c2; */
	  /*     new_c2 = new_c1; */
	  /*     new_c1 = tmp_c; */
	  /*   } */
	  assert(new_c1 != new_c2);
	  assert(new_c1 >= 0 && new_c1 <= 51 && new_c2 >= 0 && new_c2 <= 51);
	  sm[i] = cards_to_int_2[new_c1][new_c2];
	  if (CardMask_ANY_SET(b, int_to_cardmask_2[sm[i]]))
	    sm[i] = -1;
	}
      int int_map[HANDS];
      int count = 0;
      memset(int_map, 0xff, sizeof(int_map));
      
      for (i = 0; i < HANDS; i++)
	{
	  if (sm[i] == -1)
	    continue;
	  if (int_map[sm[i]] != -1)
	    continue;
	  int_map[sm[i]] = count;
	  count++;
	}
      for (i = 0; i < HANDS; i++)
	{
	  if (sm[i] == -1)
	    continue;
	  sm[i] = int_map[sm[i]];
	}
      //      printf("BOARD %i %i:%i %i:%i %i:%i %i:
      /* if (n_board == 5 && board[3] == 0 && board[4] == 1) */
      /* 	{ */
      /* 	  for (i = 0; i < count; i++) */
      /* 	    { */
      /* 	      printf("%i:", i); */
      /* 	      for (j = 0; j < HANDS; j++) */
      /* 		{ */
      /* 		  if (sm[j] == i)// && (int_to_cards_2[j].c1 == 51 || int_to_cards_2[j].c2 == 51 ||int_to_cards_2[j].c1 == 51-13 ||int_to_cards_2[j].c2 == 51-13)) */
      /* 		    printf("%i %i:%i %i ||", int_to_cards_2[j].c1%13, int_to_cards_2[j].c2%13, int_to_cards_2[j].c1/13, int_to_cards_2[j].c2/13); */
      /* 		} */
      /* 	  printf("\n"); */
      /* 	    } */
      /* 	  printf("\n"); */
      /* 	  printf("\n"); */
      /* 	} */
      free(suits);
      free(mapping);
    }
  return sm;
}

int node_cmp(const void *v1, const void *v2)
{
  const struct node *n1 = v1, *n2 = v2;
  int sum = n1->path_i - n2->path_i;
  if (sum == 0)
    {
      return memcmp(n1->path, n2->path, n1->path_i);
    }
  return sum;
}

      
struct node *gen_node(int8_t *path, int8_t path_i, int gamestate)
{
  struct node *n = malloc(sizeof(struct node));
  memcpy(n->path, path, path_i);
  n->path_i = path_i;
  
  switch(gamestate)
    {
    case 1:
      n->regs = calloc(1, sizeof(double*));
      n->odds = calloc(1, sizeof(double*));
      n->avg_odds = calloc(1, sizeof(double*));
      n->n_slots = calloc(1, sizeof(int32_t));
      break;
    case 2:
      n->regs = calloc(1, sizeof(double*)*52);
      n->odds = calloc(1, sizeof(double*)*52);
      n->avg_odds = calloc(1, sizeof(double*)*52);
      n->n_slots = calloc(1, sizeof(int32_t)*52);
      break;
    case 3:
      n->regs = calloc(1, sizeof(double*)*52*52);
      n->odds = calloc(1, sizeof(double*)*52*52);
      n->avg_odds = calloc(1, sizeof(double*)*52*52);
      n->n_slots = calloc(1, sizeof(int32_t)*52*52);
      break;
    }
  /* int max_slot = 0; */
  /* for (i = 0; i < HANDS; i++) */
  /*   { */
  /*     if (n->sm[i] > max_slot) */
  /* 	max_slot = n->sm[i]; */
  /*   } */
  /* n->n_slots = max_slot; */
  /* //n->regs = calloc(1, sizeof(double)*3*n->n_slots); */
  return n;
}



void gen_nodes(struct zbtree *ntree, struct board_data *bd, struct path_nfo *nfo, int8_t *b, gsl_rng *rng)
{
  struct node *n;
  struct path_nfo *new_nfo;
  void *retval;
  int i,j;
  int n_board = nfo->gamestate +2;
  double *regs, *odds, *avg_odds;
  int n_slots;
  n = zbtree_find(ntree, nfo, 0);
  if (n == NULL)
    {
      n = gen_node(nfo->path, nfo->path_i, nfo->gamestate);
      assert(n != NULL);
      retval = zbtree_search(ntree, n, 0);
      assert(retval == n);
    }
  int32_t *sm = get_sm_for_board(b, n_board, bd);
  int max_slot = get_max_slot_from_sm(sm);
  
  assert(max_slot < HANDS && max_slot >= 0);
  //printf("%i\n", max_slot);

  regs = calloc(1, sizeof(double)*(max_slot+1)*3);
  odds = calloc(1, sizeof(double)*(max_slot+1)*3);
  avg_odds = calloc(1, sizeof(double)*(max_slot+1)*3);

  n_slots = max_slot+1;
  for (i = 0; i < 3; i++)
    if (!nfo->actions[i])
      for(j = 0; j < n_slots; j++)
	regs[i*n_slots+j] = -10000000000.0;

  get_slot_odds(odds, regs, nfo->actions, n_slots, rng);
  
  switch(nfo->gamestate)
    {
    case 1:
      n->regs[0] = regs;
      n->odds[0] = odds;
      n->avg_odds[0] = avg_odds;
      n->n_slots[0] = n_slots;
      break;
    case 2:
      n->regs[b[3]] = regs;
      n->odds[b[3]] = odds;
      n->avg_odds[b[3]] = avg_odds;
      n->n_slots[b[3]] = n_slots;
      break;
    case 3:
      n->regs[b[3]*52+b[4]] = regs;
      n->odds[b[3]*52+b[4]] = odds;
      n->avg_odds[b[3]*52+b[4]] = avg_odds;
      n->n_slots[b[3]*52+b[4]] = n_slots;
      break;
    }
  
  for (i = 0; i < 3; i++)
    {
      if (nfo->actions[i])
	{
	  //new_nfo = add_act_to_path_nfo(nfo, i);
	  new_nfo = add_act_to_path_nfo_2bet_max(nfo, i);
	  if (new_nfo->gamestate <= 3)
	    {
	      if (new_nfo->gamestate != nfo->gamestate)
		{
		  struct next_cards *l = gen_next_cards_list(b, nfo->blen);
		  int8_t *new_board = add_card_to_board(b,nfo->gamestate+2,0);
		  int n_board = nfo->blen;
		  int j;
		  if (new_nfo->gamestate == 2)
		    printf("Start new turn\n");
		  for (j = 0; j < l->len; j++)
		    {
		      new_board[n_board] = l->cards[j];
		      gen_nodes(ntree, bd, new_nfo, new_board, rng);
		    }
		  free(l);
		  free(new_board);
		  // NEXT CARD LIST
		}
	      else
		{
		  gen_nodes(ntree, bd, new_nfo, b, rng);
		}
	    }
	  free(new_nfo);
	}
    }
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

double *vector_mul_pure(double *src1, double *src2, int len)
{
  int i;
  double *dest = malloc(sizeof(double)*len);
  for (i = 0; i < len;i++)
    {
      dest[i] = src1[i] * src2[i];
    }
  return dest;
}

void vector_mul_scalar(double *dest, double *src1, double src2, int len)
{
  int i;
  for (i = 0; i < len;i++)
    {
      dest[i] = src1[i] * src2;
    }
}

double *vector_mul_scalar_pure(double *src1, double src2, int len)
{
  int i;
  double *dest = malloc(sizeof(double)*len);

  for (i = 0; i < len;i++)
    {
      dest[i] = src1[i] * src2;
    }
  return dest;
}

void vector_mul_add(double *dest, double *src1, double *src2, int len)
{
  int i;
  for (i = 0; i < len;i++)
    {
      dest[i] += src1[i] * src2[i];
    }
}

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

double *get_fold_ev_br(double *hw, double *ev, int32_t *local_types, double potsize, double stake, int win)
{
  int i;
  double cardcount[52];
  double tot_ev = 0;
  memset(cardcount, 0, sizeof(cardcount));
  //ev = malloc(sizeof(double)*SAMPLES);
  
  /* double tmp_slot_hw[HANDS]; */
  /* for (i = 0; i < HANDS;i++) */
  /*   { */
  /*     tmp_slot_hw[i] = -1.0; */
  /*   } */
  /* for (i = 0; i < HANDS;i++) */
  /*   { */
  /*     if (local_types[i] == -1) */
  /* 	continue; */
  /*     if (tmp_slot_hw[local_types[i]] >= 0) */
  /* 	assert(tmp_slot_hw[local_types[i]] == hw[i]); */
  /*     else */
  /* 	tmp_slot_hw[local_types[i]] = hw[i]; */
  /*   } */


  for (i = 0; i < HANDS; i++)
    {
      if (local_types[i] == -1)
	assert(hw[i] == 0);
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
	  if (local_types[i] == -1  || local_types[i] == 0xffff)
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
	  if (local_types[i] == -1 || local_types[i] == 0xffff)
	    {
	      ev[i] = 0;
	      continue;
	    }
	  
	  ev[i] = -((tot_ev - cardcount[int_to_cards_2[i].c1] - cardcount[int_to_cards_2[i].c2] + hw[i]) * stake);
	}
    }
    
  return ev;
}

double *get_showdown_ev_fast(double *slot_ev, double *slot_hw, int n_slots, double potsize, double stake)
{
  int i;

  double tot_hw = 0; 
  
  if (slot_ev == NULL)
    slot_ev = malloc(sizeof(double)*n_slots);
  
  for (i = 0; i < n_slots;i++)
    {
      slot_ev[i] = tot_hw+slot_hw[i]/2.0;
      tot_hw += slot_hw[i];
    }
   for (i = 0; i < n_slots;i++)
    {
      slot_ev[i] = slot_ev[i]*potsize - tot_hw*stake;
    }
 return slot_ev;
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


double *get_slot_odds(double *slot_odds, double *src, int32_t *valid_actions, int n_slots, gsl_rng *rng)
{
  int i;
  double tot;
  if (slot_odds == NULL)
    slot_odds = malloc(sizeof(double)*n_slots*3);
  
  for (i = 0; i < n_slots*3; i++)
    {
      if (src[i] > 0)
	slot_odds[i] = src[i];
      else
	slot_odds[i] = 0;
    }
  for (i = 0; i < n_slots; i++)
    {
      tot = slot_odds[i] + slot_odds[n_slots + i] + slot_odds[n_slots*2+i];
      if (likely(tot > 0))
	{	 
	  slot_odds[i] = slot_odds[i]/tot;
	  slot_odds[n_slots + i] = slot_odds[n_slots + i]/tot;
	  slot_odds[n_slots*2 + i] = slot_odds[n_slots*2 + i]/tot;
	}
      else
	{
	  slot_odds[i] = gsl_rng_uniform(rng)*valid_actions[0];
	  slot_odds[n_slots + i] = gsl_rng_uniform(rng)*valid_actions[1];
	  slot_odds[n_slots*2 + i] = gsl_rng_uniform(rng)*valid_actions[2];
	  tot = slot_odds[i] + slot_odds[n_slots + i] + slot_odds[n_slots*2+i];
	  assert(tot > 0);
	  slot_odds[i] = slot_odds[i]/tot;
	  slot_odds[n_slots + i] = slot_odds[n_slots + i]/tot;
	  slot_odds[n_slots*2 + i] = slot_odds[n_slots*2 + i]/tot;
	}
    }
  return slot_odds;
}

double *get_hand_odds_from_slot_odds(double *slot_odds, int32_t *mapping, int n_slots)
{
  int i,j;
  double tot;
  double *hand_odds = calloc(1,sizeof(double)*HANDS*3);

  for (j = 0; j < 3; j++)
    for (i = 0; i < HANDS; i++)
      {
	if (mapping[i] == -1 || mapping[i] == 0xffff)
	  {
	    hand_odds[j*HANDS + i] = 0;
	  }
	else
	  {
	    hand_odds[j*HANDS + i] = slot_odds[j*n_slots + mapping[i]];
	  }
      }
  return hand_odds;
}
double *slots_to_hands_avg(double *hands, double *slots, int32_t *mapping, int n_slots)
{
  int i;
  double hand_count[n_slots];
  memset(hand_count, 0, sizeof(hand_count));
  
  if (hands == NULL)
    hands = calloc(1, sizeof(double)*HANDS);
  
  for (i = 0; i < HANDS; i++)
    {
      if (mapping[i] == -1)
	continue;
      hand_count[mapping[i]]+=1.0;
    }
  
  for (i = 0; i < HANDS; i++)
    {
      if (mapping[i] == -1)
	continue;
      hands[i] = slots[mapping[i]]/hand_count[mapping[i]];
    }
  return hands;
}

double *slots_to_hands(double *hands, double *slots, int32_t *mapping)
{
  int i;
  if (hands == NULL)
    hands = calloc(1, sizeof(double)*HANDS);
  for (i = 0; i < HANDS; i++)
    {
      if (mapping[i] == -1)
	{
	  continue;
	}
      hands[i] = slots[mapping[i]];
    }
  return hands;
}

double *slots_to_hands_3(double *hands, double *slots, int32_t *mapping, int n_slots)
{
  int i,j;
  double tot;
  if (hands == NULL)
    hands = calloc(1,sizeof(double)*HANDS*3);

  for (j = 0; j < 3; j++)
    slots_to_hands(&hands[j*HANDS], &slots[j*n_slots], mapping);
  return hands;
}

double *hands_to_slots_avg(double *slots, double *hands, int32_t *mapping, int n_slots)
{
  int i;
  double hand_count[n_slots];
  memset(hand_count, 0, sizeof(hand_count));
  if (slots == NULL)
    slots = calloc(1,sizeof(double)*n_slots);
  for (i = 0; i < HANDS; i++)
    {
      if (mapping[i] == -1)
	continue;
      /* if (hand_count[mapping[i]]) */
      /* 	{ */
      /* 	  if (n_slots == 1081) */
      /* 	    assert(fabs((slots[mapping[i]]/hand_count[mapping[i]]) - hands[i]) < 0.000000001); */
      /* 	} */
      hand_count[mapping[i]]+=1.0;
      slots[mapping[i]] += hands[i];
    }
  for (i = 0; i < n_slots; i++)
    {
      slots[i]/=hand_count[i];
    }
  return slots;
}

double *hands_to_slots(double *slots, double *hands, int32_t *mapping, int n_slots)
{
  int i;

  if (slots == NULL)
    slots = calloc(1, sizeof(double)*n_slots);
  for (i = 0; i < HANDS; i++)
    {
      if (mapping[i] == -1)
	continue;
      slots[mapping[i]] += hands[i];
    }
  return slots;
}
double *hands_to_slots_3(double *slots, double *hands, int32_t *mapping, int n_slots)
{
  int j;
  if (slots == NULL)
    slots = calloc(1,sizeof(double)*n_slots*3);
  for (j = 0; j < 3; j++)
    hands_to_slots(&slots[j*n_slots], &hands[j*HANDS], mapping, n_slots);
  return slots;
}
	    
double *get_odds(double *regs, int32_t *mapping, int32_t *valid_actions, int n_slots, gsl_rng *rng)
{

  int i,j;
  double tot;
  double *slot_odds = get_slot_odds(NULL, regs, valid_actions, n_slots, rng);
  double *hand_odds = slots_to_hands_3(NULL, slot_odds, mapping, n_slots);
  free(slot_odds);
  return hand_odds;
}

double *get_odds_all_call(double *regs, int32_t *mapping, int32_t *valid_actions, int n_slots, gsl_rng *rng)
{

  int i,j;
  double tot;
  double *odds = calloc(1,sizeof(double)*HANDS*3);
  
  for (i = 0; i < HANDS; i++)
    {
      if (mapping[i] != -1)
	{
	  odds[HANDS + i] = 1.0;
	}
    }
  return odds;
}

double *get_odds_all_fold(double *regs, int32_t *mapping, int32_t *valid_actions, int n_slots, gsl_rng *rng)
{

  int i,j;
  double tot;
  double *odds = calloc(1,sizeof(double)*HANDS*3);
  
  for (i = 0; i < HANDS; i++)
    {
      if (mapping[i] != -1)
	{
	  odds[HANDS*2 + i] = 1.0;
	}
    }
  return odds;
}

double *get_odds_all_raise(double *regs, int32_t *mapping, int32_t *valid_actions, int n_slots, gsl_rng *rng)
{

  int i,j;
  double tot;
  double *odds = calloc(1,sizeof(double)*HANDS*3);
  
  for (i = 0; i < HANDS; i++)
    {
      if (mapping[i] != -1)
	{
	  odds[i] = 1.0;
	}
    }
  return odds;
}

/* void update_regs(double *regs, double *odds, double *ev[3], double *avg_ev, int32_t *mapping,  int32_t *valid_actions, int n_slots) */
/* { */
/*   int i, j; */
/*   for (j = 0; j < 3; j++) */
/*     { */
/*       if (!valid_actions[j]) */
/* 	continue; */
/*       for (i = 0; i < HANDS; i++) */
/* 	{ */
/* 	  if (mapping[i] == -1) */
/* 	    continue; */
/* 	  regs[j*n_slots + mapping[i]] += ev[j][i] - avg_ev[i]; */
/* 	} */
/*     } */
/* } */
void update_regs(double *regs, double *odds, double *ev, double *avg_ev, int32_t *valid_actions, int n_slots)
{
  int i, j;
  for (j = 0; j < 3; j++)
    {
      if (!valid_actions[j])
	continue;
      for (i = 0; i < n_slots; i++)
	{
	  regs[j*n_slots + i] += ev[j*n_slots + i] - avg_ev[i];
	}
    }
}


double *get_card_counts(double *hw)
{
  int i;
  double *card_count = calloc(1,sizeof(double)*HANDS);

  for (i = 0; i < HANDS; i++)
    {
      card_count[int_to_cards_2[i].c1] += hw[i];
      card_count[int_to_cards_2[i].c2] += hw[i];
    }
  return card_count;
}


double tot_hw_sum;
double tot_ev_sum;



double *update_cfr_against_perf(struct zbtree *ntree, struct board_data *bd, struct path_nfo *nfo, int8_t *b, int pov_seat, double *hw, gsl_rng *rng, uint64_t flags)
{
  struct node *n;
  struct path_nfo *new_nfo;
  double *regs = NULL, *odds = NULL, *avg_odds = NULL, *slot_odds = NULL, *hand_odds = NULL;
  int32_t n_slots = -1;
  int i, act, act_i, j;
  int n_board;
  int ev_size;
  double hw_sum;
  double *from_up_ev[3] = {NULL, NULL, NULL};
  double *to_down_ev;
  double *slots_ev, *slots_ev_avg;
  
  int32_t *sm;
  if (nfo->gamestate < 4)
    n_board = nfo->gamestate+2;
  else if (nfo->gamestate == 4)
    n_board = 5;
  else
    n_board = nfo->gamestate - 10 +2;
  to_down_ev = calloc(1,sizeof(double)*HANDS);
  if ((hw_sum=vector_sum(hw, HANDS)) == 0)
      return to_down_ev;
  sm = get_sm_for_board(b, n_board, bd);


  

  if (nfo->gamestate > 3)
    {
      assert(nfo->potsize <= 48.1);
      if (nfo->gamestate == 4)
  	{
	  if (flags&FAST_SHOWDOWN_EV)
	    {
	      for (i = 0; i < HANDS; i++)
		if (sm[i] == -1)
		  assert(hw[i] == 0);
	      n_slots = get_max_slot_from_sm(sm)+1;
	      double *tmp_slot_hw = hands_to_slots(NULL, hw, sm, n_slots);
	      //printf("hw diff %f     ", fabs(vector_sum(hw, HANDS) - vector_sum(tmp_slot_hw, n_slots)));	      
	      vector_mul_scalar(tmp_slot_hw, tmp_slot_hw, 990.0/1176.0, n_slots);
	      double *tmp_slot_ev = get_showdown_ev_fast(NULL, tmp_slot_hw, n_slots,nfo->potsize, nfo->stake[pov_seat]);
	      //printf("FAST %f       ", vector_sum(tmp_slot_ev, n_slots));
	      slots_to_hands_avg(to_down_ev, tmp_slot_ev, sm,n_slots);
	      
	      //printf("FAST %f       ", vector_sum(to_down_ev, HANDS));
	      free(tmp_slot_hw);
	      free(tmp_slot_ev);
	    }
	  struct hand_hv2 *hv = get_hand_hv_for_board(b, n_board, bd);
	  memset(to_down_ev, 0, sizeof(double)*HANDS);
	  get_showdown_ev_br(hw, to_down_ev, hv, nfo->potsize, nfo->stake[pov_seat]);
	  //if (flags&FAST_SHOWDOWN_EV) 
	  //printf("SLOW %.20f\n", vector_sum(to_down_ev, HANDS)); 
	  
	}
      else
  	{
  	  if (nfo->seat == pov_seat) //FOLD WIN
  	    get_fold_ev_br(hw, to_down_ev, sm, nfo->potsize, nfo->stake[pov_seat], 1);
  	  else
  	    get_fold_ev_br(hw, to_down_ev, sm, nfo->potsize, nfo->stake[pov_seat], 0);
	  /* double tmp_slot_hw[HANDS]; */
	  /* for (i = 0; i < HANDS;i++) */
	  /*   { */
	  /*     tmp_slot_hw[i] = -1.0; */
	  /*   } */
	  /* for (i = 0; i < HANDS;i++) */
	  /*   { */
	  /*     if (sm[i] == -1) */
	  /* 	continue; */
	  /*     if (tmp_slot_hw[sm[i]] >= 0) */
	  /* 	assert(tmp_slot_hw[sm[i]] == to_down_ev[i]); */
	  /*     else */
	  /* 	tmp_slot_hw[sm[i]] = to_down_ev[i]; */
	  /*   } */

  	}
      
      return to_down_ev;
    }
  

  n = zbtree_find(ntree, nfo, 0);
  if (n == NULL)
    {
      printf("NODE NOT FOUND\n");
      assert(0);
    }
  
  switch(nfo->blen)
    {
    case 3:
      regs = n->regs[0];
      avg_odds = n->avg_odds[0];
      slot_odds = n->odds[0];
      n_slots = n->n_slots[0];
      break;
    case 4:
      regs = n->regs[b[3]];
      avg_odds = n->avg_odds[b[3]];
      slot_odds = n->odds[b[3]];
      
      n_slots = n->n_slots[b[3]];
      break;
    case 5:
      regs = n->regs[b[3]*52+b[4]];
      avg_odds = n->avg_odds[b[3]*52+b[4]];
      slot_odds = n->odds[b[3]*52+b[4]];
      n_slots = n->n_slots[b[3]*52+b[4]];
      break;
    default:
      assert(0);
      break;
    }

  //printf("%i\n", max_slot);

  if (nfo->seat != pov_seat)
    {
      if (flags & NONPOV_ODDS_FROM_ODDS)
	hand_odds = slots_to_hands_3(NULL, slot_odds, sm, n_slots);
      //hand_odds = get_odds_all_call(NULL, sm, NULL, 0, NULL);
      else if (flags & NONPOV_ODDS_FROM_AVG_ODDS)
	{
	  double *tmp_slot_odds = get_slot_odds(NULL, avg_odds, nfo->actions, n_slots, rng);
	  hand_odds = slots_to_hands_3(NULL, tmp_slot_odds, sm, n_slots);
	  free(tmp_slot_odds);
	}
      else if (flags & NONPOV_ODDS_FROM_REGS)
	{
	  double *tmp_slot_odds = get_slot_odds(NULL, regs, nfo->actions, n_slots, rng);
	  hand_odds = slots_to_hands_3(NULL, tmp_slot_odds, sm, n_slots);
	  free(tmp_slot_odds);
	}
      else
	assert(0);
      
    }
      /* if (nfo->seat == pov_seat) */
  /*   odds = get_odds(regs, sm, nfo->actions, n_slots, rng); */
  /* else */
  /*   { */
  /*     if (nfo->gamestate == 3 && nfo->actions[2]) */
  /* 	odds = get_odds_all_fold(regs, sm, nfo->actions, n_slots, rng); */
  /*     /\* else if (nfo->actions[0]) *\/ */
  /*     /\* 	odds = get_odds_all_raise(regs, sm, nfo->actions, n_slots, rng); *\/ */
  /*     else */
  //odds = get_odds_all_call(regs, sm, nfo->actions, n_slots, rng); 
  /*   } */

  
  for (act = 0; act < 3; act++)
    {
      if (nfo->actions[act])
	{
	  double *new_hw;
	  double new_hw_sum;
	  if (nfo->seat != pov_seat)
	    {
	      new_hw = vector_mul_pure(&hand_odds[act*HANDS], hw, HANDS);
	    }
	  else
	    {
	      new_hw = hw;
	    }
	  
	  /* if (new_hw_sum == 0) */
	  /*   { */
	  /*     assert(nfo->seat != pov_seat); */
	  /*     from_up_ev[act] = calloc(1, sizeof(double)*HANDS); */
	  /*     free(new_hw); */
	  /*     continue; */
	  /*   } */
	  
	  //new_nfo = add_act_to_path_nfo(nfo, act);
	  new_nfo = add_act_to_path_nfo_2bet_max(nfo, act);
	  if (new_nfo->gamestate <= 3)
	    {
	      if (new_nfo->gamestate != nfo->gamestate)
		{

		  double *tmp_ev;
		  struct next_cards *l = gen_next_cards_list(b, nfo->blen);
		  double div = (1.0/(52-nfo->blen-2))*(((52.0-nfo->blen-2.0)*(51.0-nfo->blen-2.0)/2.0)/((52.0-new_nfo->blen-2.0)*(51.0-new_nfo->blen-2.0)/2.0));
		  double *new_hw_divided = vector_mul_scalar_pure(new_hw, div, HANDS);
		  int8_t *new_board = add_card_to_board(b,nfo->blen,0);
		  int n_board = nfo->gamestate+2;
		  int j;
		  from_up_ev[act] = calloc(1, sizeof(double)*HANDS);
		  //if (new_nfo->gamestate == 2)
		  // printf("Start new turn\n");
		  for (j = 0; j < l->len; j++)
		    {
		  	   
		      new_board[n_board] = l->cards[j];
		      tmp_ev = update_cfr_against_perf(ntree, bd, new_nfo, new_board, pov_seat, new_hw_divided, rng, flags);
		      if (l->multi[j] > 1.0)
			vector_mul_scalar(tmp_ev, tmp_ev, l->multi[j], HANDS);
		      vector_add(from_up_ev[act], tmp_ev, HANDS);
		      free(tmp_ev);
		    }
		  free_next_cards_list(l);
		  free(new_board);
		  free(new_hw_divided);
		}
	      else
		{
		  from_up_ev[act] = update_cfr_against_perf(ntree, bd, new_nfo, b, pov_seat, new_hw, rng, flags);
		}
	    }
	  else
	    {
	      from_up_ev[act] = update_cfr_against_perf(ntree, bd, new_nfo, b, pov_seat, new_hw, rng, flags);
	    }	      
	  if (nfo->seat != pov_seat)
	    {
	      free(new_hw);
	    }
	
	  free(new_nfo);
	}    
    }

  if (nfo->seat != pov_seat)
    {
      for (act = 0; act < 3; act++)
	{
	  if (nfo->actions[act])
	    {
	      vector_add(to_down_ev, from_up_ev[act], HANDS);
	     	  
	    }
	}
    }
  else
    {     
      slots_ev = calloc(1,sizeof(double)*n_slots*3);
      slots_ev_avg = calloc(1,sizeof(double)*n_slots);
      
      for (i = 0; i < 3; i++)
	{
	  if (nfo->actions[i])
	    hands_to_slots_avg(&slots_ev[i*n_slots], from_up_ev[i], sm, n_slots);		  
	}
      /* for (i = 0; i < 3; i++) */
      /* 	{ */
      /* 	  if (nfo->actions[i]) */
      /* 	    { */
      /* 	      double *tmp_ev = slots_to_hands(NULL, &slots_ev[i*n_slots], sm); */
      /* 	      for (j = 0; j < HANDS; j++) */
      /* 		assert(fabs(tmp_ev[j]-from_up_ev[i][j]) < 0.000000000001); */
      /* 	      free(tmp_ev); */
      /* 	    } */
      /* 	} */

      if (flags & REGS_DECAY)
	{
	  vector_mul_scalar(regs, regs, 0.95, n_slots*3);
	}
      if (flags & UPDATE_REGS)
	{
	  for (act = 0; act < 3; act++)
	    {
	      if (nfo->actions[act])
		{
		  vector_mul_add(slots_ev_avg, &slot_odds[act*n_slots], &slots_ev[act*n_slots], n_slots);
		}
	    } 
	  update_regs(regs, slot_odds, slots_ev, slots_ev_avg, nfo->actions, n_slots);
	}
      if (flags & POV_ODDS_FROM_REGS)
	get_slot_odds(slot_odds, regs, nfo->actions, n_slots, rng);
      else if (flags & POV_ODDS_FROM_OPT)
	{
	  memset(slot_odds, 0, sizeof(double)*n_slots*3);
	  if (nfo->actions[0] == 1 && nfo->actions[1] == 1&& nfo->actions[2] == 1)
	    for (i = 0; i < n_slots; i++)
	      {
		if (slots_ev[i] > slots_ev[n_slots + i])
		  act_i = 0;
		else
		  act_i = 1;
		if (slots_ev[2*n_slots + i] > slots_ev[act_i*n_slots + i])
		  act_i = 2;
		slot_odds[act_i*n_slots + i] = 1.0;
		//slots_ev[i] = slots_ev[act_i*n_slots + i];
	      }
	  else if (nfo->actions[0] == 0)
	    for (i = 0; i < n_slots; i++)
	      {
		if (slots_ev[n_slots + i] > slots_ev[2*n_slots + i])
		  act_i = 1;
		else
		  act_i = 2;
		slot_odds[act_i*n_slots + i] = 1.0;
		//slots_ev[i] = slots_ev[act_i*n_slots + i];
		
	      }
	  else if (nfo->actions[2] == 0)
	    for (i = 0; i < n_slots; i++)
	      {
		if (slots_ev[i] > slots_ev[n_slots + i])
		  act_i = 0;
		else
		  act_i = 1;
		slot_odds[act_i*n_slots + i] = 1.0;
		//slots_ev[i] = slots_ev[act_i*n_slots + i];
	      }
	}

      if (flags&UPDATE_AVG_ODDS)
	{
	  vector_add(avg_odds, slot_odds, n_slots*3);
	}

      if (flags&POV_EV_FROM_ODDS)
	{
	  if (1&&nfo->gamestate < 3)
	    {
	      memset(slots_ev_avg, 0, sizeof(double)*n_slots);
	      for (act = 0; act < 3; act++)
		{
		  if (nfo->actions[act])
		    {
		      vector_mul_add(slots_ev_avg, &slot_odds[act*n_slots], &slots_ev[act*n_slots], n_slots);
		    }
		} 
	      slots_to_hands(to_down_ev, slots_ev_avg, sm);
	    }
	  else
	    {
	      //memset(to_down_ev, 0, sizeof(to_down_ev[0])*HANDS);
	      assert(hand_odds == NULL);
	      hand_odds = slots_to_hands_3(NULL, slot_odds, sm, n_slots);
	      for (act = 0; act < 3; act++)
		{
		  if (nfo->actions[act])
		    {
		      vector_mul_add(to_down_ev, &hand_odds[act*HANDS], from_up_ev[act], HANDS);
		    }
		} 
	    }	     
	}
      else if (flags&POV_EV_FROM_OPT)
	{
	  if (nfo->gamestate == 3 && (flags&NO_RIVER_ABS))
	    {
	      int act_i;
	      //memset(to_down_ev, 0, sizeof(to_down_ev[0])*HANDS);
	      if (nfo->actions[0] == 1 && nfo->actions[1] == 1&& nfo->actions[2] == 1)
		for (i = 0; i < HANDS; i++)
		  {
		    if (sm[i] != -1)
		      {
			if (from_up_ev[0][i] > from_up_ev[1][i])
			  act_i = 0;
			else
			  act_i = 1;
			if (from_up_ev[2][i] > from_up_ev[act_i][i])
			  act_i = 2;
			to_down_ev[i] = from_up_ev[act_i][i];
		      }
		  }
	      else if (nfo->actions[0] == 0)
		{
		  for (i = 0; i < HANDS; i++)
		    {
		      if (sm[i] != -1)
			{
			  if (from_up_ev[1][i] > from_up_ev[2][i])
			    act_i = 1;
			  else
			    act_i = 2;
			  to_down_ev[i] = from_up_ev[act_i][i];
			}
		    }
		  
		}
	      else if (nfo->actions[2] == 0)
		{
		  for (i = 0; i < HANDS; i++)
		    {
		      if (sm[i] != -1)
			{
			  if (from_up_ev[0][i] > from_up_ev[1][i])
			    act_i = 0;
			  else
			    act_i = 1;
			  to_down_ev[i] = from_up_ev[act_i][i];
			}
		    }
		}
	      else
		assert(0);
	    }
	  else
	    {
	      //memset(slots_ev_avg, 0, sizeof(double)*n_slots);
	      if (nfo->actions[0] == 1 && nfo->actions[1] == 1&& nfo->actions[2] == 1)
		for (i = 0; i < n_slots; i++)
		  {
		    if (slots_ev[i] > slots_ev[n_slots + i])
		      act_i = 0;
		    else
		      act_i = 1;
		    if (slots_ev[2*n_slots + i] > slots_ev[act_i*n_slots + i])
		      act_i = 2;
		    //		slot_odds[act_i*n_slots + i] = 1.0;
		    slots_ev_avg[i] = slots_ev[act_i*n_slots + i];
		  }
	      else if (nfo->actions[0] == 0)
		for (i = 0; i < n_slots; i++)
		  {
		    if (slots_ev[n_slots + i] > slots_ev[2*n_slots + i])
		      act_i = 1;
		    else
		      act_i = 2;
		    //slot_odds[act_i*n_slots + i] = 1.0;
		    slots_ev_avg[i] = slots_ev[act_i*n_slots + i];
		    
		  }
	      else if (nfo->actions[2] == 0)
		for (i = 0; i < n_slots; i++)
		  {
		    if (slots_ev[i] > slots_ev[n_slots + i])
		      act_i = 0;
		    else
		      act_i = 1;
		    //slot_odds[act_i*n_slots + i] = 1.0;
		    slots_ev_avg[i] = slots_ev[act_i*n_slots + i];
		  }
	      slots_to_hands(to_down_ev, slots_ev_avg, sm);
	      
	    }
	}
      free(slots_ev);
      free(slots_ev_avg);
    }  

  /* if (nfo->seat == pov_seat) /\*DEBUG*\/ */
  /*   { */
  /*     double hands_cmp[HANDS]; */
  /*     double slots_cmp[n_slots]; */
  /*     memset(slots_cmp, 0, sizeof(slots_cmp)); */
  /*     memset(hands_cmp, 0, sizeof(hands_cmp)); */
  /*     hands_to_slots_avg(slots_cmp, to_down_ev, sm, n_slots); */
  /*     slots_to_hands(hands_cmp, slots_cmp, sm); */
  /*     for (i = 0; i < HANDS; i++) */
  /* 	assert(fabs(hands_cmp[i]-to_down_ev[i]) < 0.000000000001); */
  /*   } */
  for (act = 0; act < 3; act++)
    if (from_up_ev[act] != NULL)
      free(from_up_ev[act]);
  
  
  free(hand_odds);
  return to_down_ev;
}


/* double *get_max_ev(struct zbtree *ntree, struct board_data *bd, struct path_nfo *nfo, int8_t *b, int pov_seat, double *hw, gsl_rng *rng, int odds_from) */
/* { */
/*   struct node *n; */
/*   struct path_nfo *new_nfo; */
/*   double *regs = NULL, *odds = NULL, *avg_odds = NULL, *slot_odds = NULL, *hand_odds = NULL; */
/*   int32_t n_slots; */
/*   int i, act; */
/*   int n_board; */
/*   int ev_size; */
/*   double hw_sum; */

/*   if (nfo->gamestate < 4) */
/*     n_board = nfo->gamestate+2; */
/*   else if (nfo->gamestate == 4) */
/*     n_board = 5; */
/*   else */
/*     n_board = nfo->gamestate - 10 +2; */
/*   double *to_down_ev; */
/*   to_down_ev = calloc(1,sizeof(double)*HANDS); */
/*   if ((hw_sum=vector_sum(hw, HANDS)) == 0) */
/*       return to_down_ev; */
/*   int32_t *sm = get_sm_for_board(b, n_board, bd); */

/*   if (nfo->gamestate > 3) */
/*     { */
/*       assert(nfo->potsize <= 48.1); */
/*       if (nfo->gamestate == 4) */
/*   	{ */
/*   	  struct hand_hv2 *hv = get_hand_hv_for_board(b, n_board, bd); */
/*   	  get_showdown_ev_br(hw, to_down_ev, hv, nfo->potsize, nfo->stake[pov_seat]); */
/*   	} */
/*       else */
/*   	{ */
/*   	  if (nfo->seat == pov_seat) //FOLD WIN */
/*   	    get_fold_ev_br(hw, to_down_ev, sm, nfo->potsize, nfo->stake[pov_seat], 1); */
/*   	  else */
/*   	    get_fold_ev_br(hw, to_down_ev, sm, nfo->potsize, nfo->stake[pov_seat], 0); */
/*   	} */
/*       return to_down_ev; */
/*     } */
  

/*   n = zbtree_find(ntree, nfo, 0); */
/*   if (n == NULL) */
/*     { */
/*       printf("NODE NOT FOUND\n"); */
/*       assert(0); */
/*     } */
  
  
/*   //printf("%i\n", max_slot); */
/*   switch(nfo->gamestate) */
/*     { */
/*     case 1: */
/*       regs = n->regs[0]; */
/*       avg_odds = n->avg_odds[0]; */
/*       odds = n->avg_odds[0]; */
/*       n_slots = n->n_slots[0]; */
/*       break; */
/*     case 2: */
/*       regs = n->regs[b[3]]; */
/*       avg_odds = n->avg_odds[b[3]]; */
/*       odds = n->odds[b[3]]; */
      
/*       n_slots = n->n_slots[b[3]]; */
/*       break; */
/*     case 3: */
/*       regs = n->regs[b[3]*52+b[4]]; */
/*       avg_odds = n->avg_odds[b[3]*52+b[4]]; */
/*       odds = n->odds[b[3]*52+b[4]]; */
/*       n_slots = n->n_slots[b[3]*52+b[4]]; */
/*       break; */
/*     } */

/*   if (nfo->seat != pov_seat) */
/*     { */
/*       switch(odds_from) */
/* 	{ */
/* 	case 0: */
/* 	  slot_odds = get_slot_odds(NULL, regs, nfo->actions, n_slots, rng); */
/* 	  break; */
/* 	case 1: */
/* 	  slot_odds = get_slot_odds(NULL, avg_odds, nfo->actions, n_slots, rng); */
/* 	  break; */
/* 	case 2: */
/* 	  slot_odds = get_slot_odds(NULL, odds, nfo->actions, n_slots, rng); */
/* 	  break; */
/* 	} */
/*       hand_odds = slots_to_hands_3(NULL, slot_odds, sm, n_slots); */
/*       free(slot_odds); */
/*     } */
      
/*   /\* if (nfo->seat == pov_seat) *\/ */
/*   /\*   odds = get_odds(regs, sm, nfo->actions, n_slots, rng); *\/ */
/*   /\* else *\/ */
/*   /\*   { *\/ */
/*   /\*     if (nfo->gamestate == 3 && nfo->actions[2]) *\/ */
/*   /\* 	odds = get_odds_all_fold(regs, sm, nfo->actions, n_slots, rng); *\/ */
/*   /\*     /\\* else if (nfo->actions[0]) *\\/ *\/ */
/*   /\*     /\\* 	odds = get_odds_all_raise(regs, sm, nfo->actions, n_slots, rng); *\\/ *\/ */
/*   /\*     else *\/ */
/*   //odds = get_odds_all_call(regs, sm, nfo->actions, n_slot, rng);  */
/*   /\*   } *\/ */
/*   double *from_up_ev[3] = {NULL, NULL, NULL}; */
/*   assert(from_up_ev[0] == NULL); */
/*   assert(from_up_ev[1] == NULL); */
/*   assert(from_up_ev[2] == NULL); */
  
/*   for (act = 0; act < 3; act++) */
/*     { */
/*       if (nfo->actions[act]) */
/* 	{ */
/* 	  double *new_hw; */
/* 	  double new_hw_sum; */
/* 	  if (nfo->seat != pov_seat) */
/* 	    { */
/* 	      new_hw = vector_mul_pure(&hand_odds[act*HANDS], hw, HANDS); */
/* 	      new_hw_sum = vector_sum(new_hw, HANDS); */
/* 	    } */
/* 	  else */
/* 	    { */
/* 	      new_hw = hw; */
/* 	      new_hw_sum = hw_sum; */
/* 	    } */
	  
/* 	  if (new_hw_sum == 0) */
/* 	    { */
/* 	      assert(nfo->seat != pov_seat); */
/* 	      from_up_ev[act] = calloc(1, sizeof(double)*HANDS); */
/* 	      free(new_hw); */
/* 	      continue; */
/* 	    } */
	  
/* 	  //new_nfo = add_act_to_path_nfo(nfo, act); */
/* 	  new_nfo = add_act_to_path_nfo_2bet_max(nfo, act); */
/* 	  if (new_nfo->gamestate <= 3) */
/* 	    { */
/* 	      if (new_nfo->gamestate != nfo->gamestate) */
/* 		{ */

/* 		  double *tmp_ev; */
/* 		  struct next_cards *l = gen_next_cards_list(b, nfo->blen); */
/* 		  double div = (1.0/(52-nfo->blen-2))*(((52.0-nfo->blen-2.0)*(51.0-nfo->blen-2.0)/2.0)/((52.0-new_nfo->blen-2.0)*(51.0-new_nfo->blen-2.0)/2.0)); */
/* 		  double *new_hw_divided = vector_mul_scalar_pure(new_hw, div, HANDS); */
/* 		  int8_t *new_board = add_card_to_board(b,nfo->blen,0); */
/* 		  int n_board = nfo->gamestate+2; */
/* 		  int j; */
/* 		  from_up_ev[act] = calloc(1, sizeof(double)*HANDS); */
/* 		  if (new_nfo->gamestate == 2) */
/* 		    printf("Start new turn\n"); */
/* 		  for (j = 0; j < l->len; j++) */
/* 		    { */
		  	   
/* 		      new_board[n_board] = l->cards[j]; */
/* 		      tmp_ev = update_cfr_against_perf(ntree, bd, new_nfo, new_board, pov_seat, new_hw_divided, rng,set_opt); */
/* 		      if (l->multi[j] > 1.0) */
/* 			vector_mul_scalar(tmp_ev, tmp_ev, l->multi[j], HANDS); */
/* 		      vector_add(from_up_ev[act], tmp_ev, HANDS); */
/* 		      free(tmp_ev); */
/* 		    } */
/* 		  free_next_cards_list(l); */
/* 		  free(new_board); */
/* 		  free(new_hw_divided); */
				      
/* 		  //free(card_counts); */
/* 		  //free(new_hw_divided); */
/* 		  // NEXT CARD LIST */
/* 		} */
/* 	      else */
/* 		{ */
/* 		  from_up_ev[act] = update_cfr_against_perf(ntree, bd, new_nfo, b, pov_seat, new_hw, rng, set_opt); */
/* 		} */
/* 	    } */
/* 	  else */
/* 	    { */
/* 	      from_up_ev[act] = update_cfr_against_perf(ntree, bd, new_nfo, b, pov_seat, new_hw, rng, set_opt); */
/* 	    }	       */
/* 	  if (nfo->seat != pov_seat) */
/* 	    { */
/* 	      free(new_hw); */
/* 	    } */
	
/* 	  free(new_nfo); */
/* 	}     */
/*     } */

/*   if (nfo->seat != pov_seat) */
/*     { */
/*       for (act = 0; act < 3; act++) */
/* 	{ */
/* 	  if (nfo->actions[act]) */
/* 	    { */
/* 	      vector_add(to_down_ev, from_up_ev[act], HANDS); */
	     	  
/* 	    } */
/* 	} */
/*     } */
/*   else */
/*     { */
/*       if (nfo->seat == pov_seat) */
/* 	{ */
/* 	  int act_i; */
/* 	  if (nfo->actions[0] == 1 && nfo->actions[1] == 1&& nfo->actions[2] == 1) */
/* 	    for (i = 0; i < HANDS; i++) */
/* 	      { */
/* 		if (sm[i] != -1) */
/* 		  { */
/* 		    if (from_up_ev[0][i] > from_up_ev[1][i]) */
/* 		      act_i = 0; */
/* 		    else */
/* 		      act_i = 1; */
/* 		    if (from_up_ev[2][i] > from_up_ev[act_i][i]) */
/* 		      act_i = 2; */
/* 		    to_down_ev[i] = from_up_ev[act_i][i]; */
/* 		  } */
/* 	      } */
/* 	  else if (nfo->actions[0] == 0) */
/* 	    { */
/* 	      for (i = 0; i < HANDS; i++) */
/* 	  	{ */
/* 		  if (sm[i] != -1) */
/* 		    { */
/* 		      if (from_up_ev[1][i] > from_up_ev[2][i]) */
/* 			act_i = 1; */
/* 		      else */
/* 			act_i = 2; */
/* 		      to_down_ev[i] = from_up_ev[act_i][i]; */
/* 		    } */
/* 	  	} */

/* 	    } */
/* 	  else if (nfo->actions[2] == 0) */
/* 	    { */
/* 	      for (i = 0; i < HANDS; i++) */
/* 	  	{ */
/* 		  if (sm[i] != -1) */
/* 		    { */
/* 		      if (from_up_ev[0][i] > from_up_ev[1][i]) */
/* 			act_i = 0; */
/* 		      else */
/* 			act_i = 1; */
/* 		      to_down_ev[i] = from_up_ev[act_i][i]; */
/* 		    } */
/* 	  	} */
/* 	    } */
/* 	  else */
/* 	    assert(0); */
/* 	} */
/*     } */
/*   for (act = 0; act < 3; act++) */
/*     if (from_up_ev[act] != NULL) */
/*       free(from_up_ev[act]); */
  
  
/*   free(hand_odds); */
/*   return to_down_ev; */
/* } */

int main()
{
  precalc_conversions();
  int i,j;
  int8_t flop[3] = {35,19,51};
  //int8_t flop[3] = {0,24,48};
  int8_t *sorted_flop = sort_flop(flop, 3);
  int8_t *suits = get_suits(sorted_flop, 3);
  int8_t *suit_mapping = get_suit_mapping(suits,3);
  for (i = 0; i < 3; i++)
    {
      sorted_flop[i] = morph_card(sorted_flop[i], suit_mapping);
    }
  struct board_data *bd = gen_board_data(sorted_flop);
  struct path_nfo *nfo = gen_start_path_nfo();
  struct path_nfo *tmp_nfo;
  struct zbtree *ntree = zbtree_init(node_cmp);
  gsl_rng *rng = gsl_rng_alloc(gsl_rng_taus2);
  double *hw = calloc(1, sizeof(double)*HANDS);
  double *ev;
  double sum, sum2;
  uint64_t flags[2], max_ev_flags[2];


  //flags[0] = POV_ODDS_FROM_REGS|UPDATE_REGS|POV_EV_FROM_ODDS|NONPOV_ODDS_FROM_ODDS|UPDATE_AVG_ODDS|FAST_SHOWDOWN_EV;
  
  //flags[0] = POV_ODDS_FROM_REGS|UPDATE_REGS|POV_EV_FROM_ODDS|NONPOV_ODDS_FROM_ODDS|UPDATE_AVG_ODDS;
  //flags[1] = POV_ODDS_FROM_REGS|UPDATE_REGS|POV_EV_FROM_ODDS|NONPOV_ODDS_FROM_ODDS|UPDATE_AVG_ODDS;
  flags[0] = POV_ODDS_FROM_REGS|UPDATE_REGS|POV_EV_FROM_ODDS|NONPOV_ODDS_FROM_ODDS;  
  flags[1] = POV_ODDS_FROM_OPT|POV_EV_FROM_OPT|NONPOV_ODDS_FROM_ODDS;
    
  //flags[0] = POV_ODDS_FROM_REGS|UPDATE_REGS|POV_EV_FROM_ODDS|NONPOV_ODDS_FROM_ODDS;
  //flags[1] = POV_ODDS_FROM_REGS|UPDATE_REGS|POV_EV_FROM_ODDS|NONPOV_ODDS_FROM_ODDS;

  max_ev_flags[0] = POV_EV_FROM_OPT;
  max_ev_flags[1] = POV_EV_FROM_OPT;
  tmp_nfo = nfo;
  //  nfo = add_act_to_path_nfo(nfo, 0);
  nfo = add_act_to_path_nfo_2bet_max(nfo, 1);
  free(tmp_nfo);
  tmp_nfo = nfo;
  //nfo = add_act_to_path_nfo(nfo, 1);
  nfo = add_act_to_path_nfo_2bet_max(nfo, 1);
  free(tmp_nfo);
  gen_nodes(ntree, bd, nfo, sorted_flop, rng);

  for (i = 0; i < HANDS; i++)
    hw[i] = (1.0/1176.0)*(1176.0/1081.0);
  for (i = 0; i < 100000; i++)
    {
      int get_max_ev = i%11?0:1;
      int start_time = time(NULL);
      int stop_time;
      printf("start update %i\n",i);
      tot_hw_sum = 0;
      ev = update_cfr_against_perf(ntree, bd, nfo, sorted_flop, i%2, hw, rng, flags[i%2]);
      stop_time = time(NULL);
      sum = 0;
      sum2 = 0;
      	
      for (j = 0; j < HANDS; j++)
	{
	  sum+=ev[j];
	  /* if (get_max_ev) */
	  /*   sum2+=ev[HANDS+j]; */
	}
	  
      sum/= (double)1176.0;
      /* if (get_max_ev) */
      /* 	sum2/= (double)1176.0; */
      printf("stop update %i time: %i seat:%i sum:%f %f %f\n", i, stop_time-start_time, i%2, sum, sum2, tot_hw_sum);
      free(ev);
      if (!(i%50) && i > 0)
	{
	  sum = 0;
	  ev = update_cfr_against_perf(ntree, bd, nfo, sorted_flop, 0, hw, rng, POV_EV_FROM_OPT|NONPOV_ODDS_FROM_AVG_ODDS);
	  for (j = 0; j < HANDS; j++)
	    sum+=ev[j];
	  printf("max_ev avg %i %f\n", 0, sum/1176.0);
	  free(ev);
	  sum = 0;
	  ev = update_cfr_against_perf(ntree, bd, nfo, sorted_flop, 1, hw, rng, POV_EV_FROM_OPT|NONPOV_ODDS_FROM_AVG_ODDS);
	  for (j = 0; j < HANDS; j++)
	    sum+=ev[j];
	  printf("max_ev avg %i %f\n", 1, sum/1176.0);
	  free(ev);

	  sum = 0;
	  ev = update_cfr_against_perf(ntree, bd, nfo, sorted_flop, 0, hw, rng, POV_EV_FROM_OPT|NONPOV_ODDS_FROM_REGS);
	  for (j = 0; j < HANDS; j++)
	    sum+=ev[j];
	  printf("max_ev avg fast ev %i %f\n", 0, sum/1176.0);
	  free(ev);
	  sum = 0;
	  ev = update_cfr_against_perf(ntree, bd, nfo, sorted_flop, 1, hw, rng, POV_EV_FROM_OPT|NONPOV_ODDS_FROM_REGS);
	  for (j = 0; j < HANDS; j++)
	    sum+=ev[j];
	  printf("max_ev avg fast ev%i %f\n", 1, sum/1176.0);
	  free(ev);

	  sum = 0;
	  ev = update_cfr_against_perf(ntree, bd, nfo, sorted_flop, 0, hw, rng, POV_EV_FROM_OPT|NONPOV_ODDS_FROM_AVG_ODDS|NO_RIVER_ABS);
	  for (j = 0; j < HANDS; j++)
	    sum+=ev[j];
	  printf("max_ev avg no abs %i %f\n", 0, sum/1176.0);
	  free(ev);
	  sum = 0;
	  ev = update_cfr_against_perf(ntree, bd, nfo, sorted_flop, 1, hw, rng, POV_EV_FROM_OPT|NONPOV_ODDS_FROM_AVG_ODDS|NO_RIVER_ABS);
	  for (j = 0; j < HANDS; j++)
	    sum+=ev[j];
	  printf("max_ev avg_no abs %i %f\n", 1, sum/1176.0);
	  free(ev);

	}
    }
}
