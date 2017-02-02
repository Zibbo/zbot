#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>

#include "rgbr_interface.h"
#include "br_if_defs.h"
#include "bit.h"

#include "defs.h"


int cards_to_int_4[52][52][52][52];
int cards_to_int_3[52][52][52];
int cards_to_int_2[52][52];
int cards_to_int_1[52];
int cards_to_int_2_nosuit[13][13];

//struct cards_4 int_to_cards_4[270725];

struct cards_4 int_to_cards_4[270725];
struct cards_3 int_to_cards_3[22100];
struct cards_2 int_to_cards_2[1326];
struct cards_1 int_to_cards_1[52];
struct cards_2 int_to_cards_2_nosuit[78];

/* CardMask int_to_cardmask_4[270725]; */
/* CardMask int_to_cardmask_3[22100]; */
/* CardMask int_to_cardmask_2[1326]; */

int preflop_morph_mapping[1326];

int a2z[52];

void precalc_conversions()
{
  int c = 0, i, j, k, l;
  //CardMask tmpmask1, tmpmask2, tmpmask3;

  for (i = 0; i < 13; i++)
    {
      a2z[i*4] = 3*13+i;
      a2z[i*4+1] = 0*13+i;
      a2z[i*4+2] = 1*13+i;
      a2z[i*4+3] = 2*13+i;
    }
  //printf("func addr: %lx\n", &precalc_conversions);
  c = 0;
  for (i = 51; i >=0;i--)
    {
      for (j = i-1;j >= 0; j--)
        {
          for (k = j-1;k >= 0; k--)
            {
	      for (l = k-1;l >= 0; l--)
		{
		  cards_to_int_4[i][j][k][l] = c;
		  cards_to_int_4[i][j][l][k] = c;
		  cards_to_int_4[i][l][j][k] = c;
		  cards_to_int_4[i][k][j][l] = c;
		  cards_to_int_4[i][k][l][j] = c;
		  cards_to_int_4[i][l][k][j] = c;

		  cards_to_int_4[j][i][k][l] = c;
                  cards_to_int_4[j][i][l][k] = c;
                  cards_to_int_4[j][l][i][k] = c;
                  cards_to_int_4[j][k][i][l] = c;
                  cards_to_int_4[j][k][l][i] = c;
                  cards_to_int_4[j][l][k][i] = c;

		  cards_to_int_4[k][j][i][l] = c;
                  cards_to_int_4[k][j][l][i] = c;
                  cards_to_int_4[k][l][j][i] = c;
                  cards_to_int_4[k][i][j][l] = c;
                  cards_to_int_4[k][i][l][j] = c;
                  cards_to_int_4[k][l][i][j] = c;

		  cards_to_int_4[l][j][k][i] = c;
                  cards_to_int_4[l][j][i][k] = c;
                  cards_to_int_4[l][i][j][k] = c;
                  cards_to_int_4[l][k][j][i] = c;
                  cards_to_int_4[l][k][i][j] = c;
                  cards_to_int_4[l][i][k][j] = c;
		  
		  int_to_cards_4[c].c1 = i;
		  int_to_cards_4[c].c2 = j;
		  int_to_cards_4[c].c3 = k;
		  int_to_cards_4[c].c4 = l;
		  
		  /* CardMask_RESET(int_to_cardmask_4[c]); */
		  /* CardMask_SET(int_to_cardmask_4[c], i); */
		  /* CardMask_SET(int_to_cardmask_4[c], j); */
		  /* CardMask_SET(int_to_cardmask_4[c], k); */
		  /* CardMask_SET(int_to_cardmask_4[c], l); */

		  c++;
		}
	    }
	}
    }
  c = 0;

  for (i = 51; i >=0;i--)
    {
      for (j = i-1;j >= 0; j--)
	{
	  for (k = j-1;k >= 0; k--)
	    {
	      cards_to_int_3[i][j][k] = c;
	      cards_to_int_3[i][k][j] = c;
	      cards_to_int_3[k][i][j] = c;
	      cards_to_int_3[j][i][k] = c;
	      cards_to_int_3[j][k][i] = c;
	      cards_to_int_3[k][j][i] = c;

	      
	      int_to_cards_3[c].c1 = i;
	      int_to_cards_3[c].c2 = j;
	      int_to_cards_3[c].c3 = k;

	      //cards_to_int_3[i][j][k] = c;
	      
	      /* tmpmask1 = Deck_MASK(i); */
	      /* tmpmask2 = Deck_MASK(j); */
	      /* tmpmask3 = Deck_MASK(k); */
	      /* CardMask_RESET(int_to_cardmask_3[c]); */
	      /* CardMask_SET(int_to_cardmask_3[c], i); */
	      /* CardMask_SET(int_to_cardmask_3[c], j); */
	      /* CardMask_SET(int_to_cardmask_3[c], k); */

	      c++;
	    }
	}
    }
  
  c = 0;
  for (i = 51; i >=0;i--)
    {
      for (j = i-1;j >= 0; j--)
        {
          cards_to_int_2[i][j] = c;
	  cards_to_int_2[j][i] = c;


	  int_to_cards_2[c].c1 = i;
	  int_to_cards_2[c].c2 = j;
	  
	  cards_to_int_2[i][j] = c;

	  /* tmpmask1 = Deck_MASK(i); */
	  /* tmpmask2 = Deck_MASK(j); */

	  /* CardMask_RESET(int_to_cardmask_2[c]); */
	  /* CardMask_SET(int_to_cardmask_2[c], i); */
	  /* CardMask_SET(int_to_cardmask_2[c], j); */

	  c++;
	}
    }

  c = 0;
  for (i = 51; i >=0;i--)
    {
      cards_to_int_1[i] = c;
      cards_to_int_1[i] = c;
      int_to_cards_1[c].c1 = i;

      c++;
    }
  c = 0;
  for (i = 12; i >=0;i--)
    {
      for (j = i-1;j >= 0; j--)
        {
	  cards_to_int_2_nosuit[i][j] = c;
	  cards_to_int_2_nosuit[j][i] = c;
	  int_to_cards_2_nosuit[c].c1 = i;
	  int_to_cards_2_nosuit[c].c2 = j;
	  c++;
	}
    }
  c = 0;
  for (i = 51; i >=0;i--)
    {
      for (j = i-1;j >= 0; j--)
        {
	  if (i%13 == j%13)
	    preflop_morph_mapping[c] = i%13;
	  else if (i/13 == j/13)
	    preflop_morph_mapping[c] = 13+cards_to_int_2_nosuit[i%13][j%13];
	  else
	    preflop_morph_mapping[c] = 13+78+cards_to_int_2_nosuit[i%13][j%13];
	  c++;
	}
    }
}

int read_exactly(int fd, void *buf, int size)
{
  int received = 0, received_tot = 0;

  while (received_tot < size)
    {
      received = read(fd, &((char*)buf)[received_tot], size-received_tot);
      if (received == 0)
	{
	  printf("EOF\n");
	  assert(0);
	}
      if (received == -1)
        {
	  perror("plrmodel read");
	  assert(0);
	}
      //assert(received != -1);
      received_tot += received;
    }
  return received_tot;
}

struct plrmodel_node *load_plrmodel_tree(struct gameinfo *info, int fileno, struct unique_root *u)
{
  struct plrmodel_node *n, *next_n;
  int i, public;
  n = calloc(1, sizeof(struct plrmodel_node));
  
  assert(sizeof(i) == sizeof(n->t->id));

  read_exactly(fileno,&i, sizeof(n->t->id));
  assert(i >= 0 && i <= 13);
  read_exactly(fileno,&public, sizeof(n->t->public));
  assert(public == 0 || public == 1);
  n->t = &info->type_types[public][i];
  n->types_bmap = alloc_bitfield(n->t->n_types);
  read_exactly(fileno, n->types_bmap, bitfield_bytesize(n->t->n_types));
  n->valid_bmap = alloc_bitfield(n->t->n_types);
  n->type_mapping = NULL;
  n->type_mapping_alt = NULL;
  
  n->len = bitcount_before_bitmap(n->types_bmap, n->t->n_types);
  n->root = u;
  

  if (n->t->public == 0)
    {
      struct situ *s = malloc(sizeof(struct situ));
      read_exactly(fileno, s, sizeof(struct situ));

      for (i = 0; i < ACTS; i++)
	{
	  if (s->regs[i] != NULL)
	    {
	      s->regs[i] = malloc(sizeof(s->regs[i][0]) * n->len);
	      read_exactly(fileno, s->regs[i], sizeof(s->regs[i][0]) * n->len);
	    }
	  /* if (s->d_regs[i] != NULL) */
	  /*   { */
	  /*     s->d_regs[i] = malloc(sizeof(s->d_regs[i][0]) * n->len); */
	  /*     read_exactly(fileno, s->d_regs[i], sizeof(s->d_regs[i][0]) * n->len); */
	  /*   } */
	  if (s->avg_odds[i] != NULL)
	    {
	      s->avg_odds[i] = malloc(sizeof(s->avg_odds[i][0]) * n->len);
	      read_exactly(fileno, s->avg_odds[i], sizeof(s->avg_odds[i][0]) * n->len);
	    }
	  if (i < ACTS-1)
	    {
	      /* if (s->hand_odds[i] != NULL) */
	      /* 	{ */
	      /* 	   s->hand_odds[i] = malloc(sizeof(s->hand_odds[i][0]) * n->len); */
	      /* 	   read_exactly(fileno, s->hand_odds[i], sizeof(s->hand_odds[i][0]) * n->len); */
	      /* 	} */
	      
	      if (s->ev[i] != NULL)
		{
		  s->ev[i] = malloc(sizeof(s->ev[i][0]) * n->len);
		  read_exactly(fileno, s->ev[i], sizeof(s->ev[i][0]) * n->len);
		}
	      if (s->ev_count[i] != NULL)
		{
		  s->ev_count[i] = malloc(sizeof(s->ev_count[i][0]) * n->len);
		  read_exactly(fileno, s->ev_count[i], sizeof(s->ev_count[i][0]) * n->len);
		}

	    }
	}
      /* if (s->visits != NULL) */
      /* 	{ */
      /* 	  s->visits = malloc(sizeof(s->visits[0]) * n->len); */
      /* 	  read_exactly(fileno, s->visits, sizeof(s->visits[0]) * n->len); */
      /* 	} */
      
      //s->parent = n;
      n->hand_count = 1;
      n->next_list = (void**)s;
    }
  else
    {
      uint64_t *tmpbuf = malloc(sizeof(uint64_t)*n->len);
      read_exactly(fileno, tmpbuf, sizeof(uint64_t)*n->len);
      
      //lseek(fileno, sizeof(uint64_t)*n->len, SEEK_CUR);
      n->next_list = malloc(sizeof(void*)*n->len);
      //      n->slot_structure = malloc(sizeof(struct slot)*n->len);
      //n->types_bmap = alloc_bitfield(n->max_len);
      //read_exactly(fileno,n->types_bmap, bitfield_bytesize(n->max_len));
      n->hand_count = 0;
      for (i = 0; i < n->len;i++)
	{
	  //assert(tmpbuf[i] == lseek(fileno, 0, SEEK_CUR));
	  next_n = load_plrmodel_tree(info, fileno, u);
	  next_n->prev = n;
	  next_n->slot_in_prev = i;
	  n->next_list[i] = next_n;
	  n->hand_count += next_n->hand_count;
	}
      free(tmpbuf);
    }
  return n;
}

struct unique_root *fold_node = NULL;
struct unique_root *root_nodes[5];

 
struct unique_root *load_unique_roots(int fdn, int branched)
{
  int i;
  struct unique_root *r;

  if (fold_node == NULL)
    {
      fold_node = (struct unique_root *) malloc(sizeof(struct unique_root));
      memset(fold_node, 0, sizeof(struct unique_root));
      fold_node->root_idx = 23;
      fold_node->n_plr = 1; 
      fold_node->to_act = 1; 
      fold_node->gamestate = 4; 
      fold_node->cur_seat = 0; 
    }
  
  if (root_nodes[4] == NULL)
    {
      root_nodes[4] = (struct unique_root *) malloc(sizeof(struct unique_root));
      memset(root_nodes[4], 0, sizeof(struct unique_root));
      root_nodes[4]->root_idx = 21;
      root_nodes[4]->n_plr = 2; 
      root_nodes[4]->to_act = 2; 
      root_nodes[4]->gamestate = 4; 
      root_nodes[4]->cur_seat = 0; 
    }
  
  
  r = (struct unique_root *)malloc(sizeof(struct unique_root));
  read(fdn, r, sizeof(struct unique_root));

  r->bets = malloc(r->n_plr*sizeof(double));
  read(fdn, r->bets, r->n_plr*sizeof(double));
  
  r->type_types_order[0] = malloc( r->n_type_types[0]*sizeof(int));
  read(fdn, r->type_types_order[0], r->n_type_types[0]*sizeof(int));
  r->type_types_order[1] = malloc( r->n_type_types[1]*sizeof(int));
  read(fdn, r->type_types_order[1], r->n_type_types[1]*sizeof(int));
  
  

  if (r->next[0] != NULL)
    {
      r->next[0] = load_unique_roots(fdn, branched);
      if (r->to_act == 1)
	r->next[1] = root_nodes[r->gamestate+1];
      else
	r->next[1] = load_unique_roots(fdn, 1);
    }
  else
    {
      if (r->gamestate < 3 && branched == 0)
	{
	  r->next[1] = load_unique_roots(fdn,0);
	  assert(r->next[1]->gamestate == r->gamestate+1);
	  root_nodes[r->gamestate+1] = r->next[1];
	}
      else
	{
	  r->next[1] = root_nodes[r->gamestate+1];
	}
    }
  
  r->next[2] = fold_node;
  return r;
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

int16_t get_potsize_slot_from_hi(const struct type_type *t, const struct hand_info *hi)
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


int16_t get_last_act_slot_from_hi(const struct type_type *t, const struct hand_info *hi)
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

int16_t get_bets_slot_from_hi(const struct type_type *t, const struct hand_info *hi)
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

int16_t get_board_slot_from_hi(const struct type_type *t, const struct hand_info *hi)
{
  int i;
  int flop, turn;
  //int16_t retval;
  
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
      assert(0);
      /* float *tmp_t; */
      /* assert(t->types != NULL); */
      /* tmp_t = calloc(1, sizeof(float)*t->n_items_per_type); */
      /* gen_board_type(t->info, tmp_t, t->gamestate, flop, hi->board[3], hi->board[4]); */
      /* //t->gen_type(t, hi, tmp_t); */
      /* retval = get_slot(t->types, tmp_t, t->n_types, t->n_items_per_type); */
      /* //retval = get_slot_tt(t, tmp_t); */
      /* free(tmp_t); */
      /* return retval; */
    }
}

int16_t get_hand_slot_from_hi(const struct type_type *t, const struct hand_info *hi)
{
  //int i;
  //int flop, turn, hand;
  //int16_t retval;
  return -1;
  /* if (hi->cur_us->gamestate < t->gamestate) */
  /*   return -1; */
  
  /* if (t->gamestate > 0) */
  /*   { */
  /*     for (i = 0; i < t->gamestate+2; i++) */
  /* 	assert(hi->board[i] >= 0 && hi->board[i] < 52); */
      
  /*   } */
  
  /* hand = cards_to_int_2[hi->hole_cards[0]][hi->hole_cards[1]]; */

  /* if (t->slots != NULL) */
  /*   { */
  /*     switch (t->gamestate) */
  /* 	{ */
  /* 	case 0: */
  /* 	  return t->slots[hand]; */
  /* 	  break; */
  /* 	case 1: */
  /* 	  flop = cards_to_int_3[hi->board[0]][hi->board[1]][hi->board[2]]; */
  /* 	  return t->slots[flop*HANDS + hand]; */
  /* 	  break; */
  /* 	case 2: */
  /* 	  turn = cards_to_int_4[hi->board[0]][hi->board[1]][hi->board[2]][hi->board[3]]; */
  /* 	  return t->slots[turn*HANDS + hand]; */
  /* 	  break; */
  /* 	default: */
  /* 	  assert(0); */
  /* 	  break; */
  /* 	} */
  /*   } */
  /* else */
  /*   { */
  /*     float *tmp_t; */
  /*     assert(t->types != NULL); */
  /*     tmp_t = calloc(1, sizeof(float)*t->n_items_per_type); */
  /*     flop = cards_to_int_3[hi->board[0]][hi->board[1]][hi->board[2]]; */
  /*     switch (t->gamestate) */
  /* 	{ */
  /* 	case 2: */
  /* 	  gen_single_turn_type(t->info, tmp_t, flop, hi->board[3], hand); */
  /* 	  break; */
  /* 	case 3: */
  /* 	  gen_single_river_type(t->info, tmp_t, flop, hi->board[3], hi->board[4], hand); */
  /* 	  break; */
  /* 	default: */
  /* 	  assert(0); */
  /* 	  break; */
  /* 	} */
  /*     retval = get_slot(t->types, tmp_t, t->n_types, t->n_items_per_type); */
  /*     free(tmp_t); */
  /*     return retval; */
  /*   } */
}

int16_t type_to_slot(uint64_t *types_bmap, int16_t type_i)
{
  return bitcount_before_bitmap(types_bmap, type_i);
}


int16_t slot_to_type(uint64_t *types_bmap, int16_t slot_i)
{
  return get_nth_set_bit_bitmap(types_bmap, slot_i);
}

int find_closest_type_diffs(short int *diffs_order, uint64_t *types_bmap, int n_types, short int slot)
{

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

int find_closest_type(struct type_type *tt, uint64_t *types_bmap, int16_t slot)
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
      return find_closest_type_diffs(tt->diffs_order, types_bmap, tt->n_types, slot);
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
 
  

void generate_mapping_to_slots(uint64_t *types_bmap, int16_t *mapping, struct type_type *t)
{
  int i;

  for (i = 0; i < t->n_types; i++)
    {
      mapping[i] = find_closest_type(t, types_bmap, i);
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
  int i;
  int bets = 0;
  double potsize = 0;

  for (i = 0; i < path_len; i++)
    {
      if (path[i] == 0 && r->root_idx != 0)
	bets++;
      if (r->next[(int)path[i]]->gamestate != r->gamestate)
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
      potsize += r->action_cost[(int)path[i]];
      
      r = r->next[(int)path[i]];
      if (r->gamestate == 4)
	break;
    }
  types[0] = (int)round(potsize);
  return r;
}

void get_n_closest_local_type(struct plrmodel_node *n, int16_t global_type_i, int16_t *dest, int n_req)
{
  int i;

  find_n_closest_type(n->t, n->types_bmap, global_type_i, dest, n_req);
  for (i = 0; i < n_req; i++)
    dest[i] = bitcount_before_bitmap(n->types_bmap, dest[i]);
}

int16_t get_closest_local_type(struct plrmodel_node *n, int16_t global_type_i)
{
  int i;

  i = find_closest_type(n->t, n->types_bmap, global_type_i);
  return  bitcount_before_bitmap(n->types_bmap, i);
}

struct plrmodel_node *get_first_matching_situ(struct plrmodel_node *n, int16_t *st, int set_valid)
{
  int i;
  int16_t i2;
  
  /* if (update_visits != -1) */
  /*   { */
  /*     n->visits[update_visits]++; */
  if (set_valid && n->t->public)
    set_bit(n->valid_bmap, st[n->t->id]);
  /*   } */
  if (n->t->public == 0)
    {
      return n;
    }
  else
    {
      i = get_closest_local_type(n, st[n->t->id]);
      get_n_closest_local_type(n, st[n->t->id], &i2, 1);
      assert(i == i2);
      /* if (n->t->style == 2) */
      /* 	i = find_closest_type_diffs(n->t->diffs_order, n->types_bmap, n->t->n_types, st[n->t->id]); */
      /* else */
      /* 	i = find_closest_type_linear_int(n->types_bmap, n->t->n_types, st[n->t->id]); */
      /* i = bitcount_before_bitmap(n->types_bmap, i); */
      return get_first_matching_situ(n->next_list[i], st, set_valid);
    }
  return NULL;
}

void generate_mapping_from_global_to_local(struct plrmodel_node *n)
{
  n->type_mapping = malloc(sizeof(int16_t)*n->t->n_types);
  generate_mapping_to_slots(n->types_bmap, n->type_mapping, n->t);
}


void save_gameinfo(struct gameinfo *info, int fdn)
{
  int i;
  write(fdn, info, sizeof(struct gameinfo));
  
  for (i = 0; i < info->n_type_types[0]; i++)
    write(fdn, &info->type_types[0][i], sizeof(struct type_type));
  
  for (i = 0; i < info->n_type_types[1]; i++)
    write(fdn, &info->type_types[1][i], sizeof(struct type_type));
  
}

struct gameinfo *load_gameinfo(int fdn)
{
  size_t sz; 
  int i;
  FILE *fp;
  struct type_type *t;
  struct gameinfo *info = malloc(sizeof(struct gameinfo));
  read(fdn, info, sizeof(struct gameinfo));
  
  /* load types sltos etc */

  info->type_types[0] = malloc(sizeof(struct type_type)*info->n_type_types[0]);
  info->type_types[1] = malloc(sizeof(struct type_type)*info->n_type_types[1]);

  for (i = 0; i < info->n_type_types[0]; i++)
    {
      t = &info->type_types[0][i];
      read(fdn, t, sizeof(struct type_type));
      t->info = info;
      t->diffs = malloc(sizeof(float)*t->n_types*t->n_types);
      t->diffs_order = malloc(sizeof(int16_t)*t->n_types*t->n_types);
      /* if (t->types != NULL) */
      /* 	t->types = malloc(sizeof(float)*t->n_types*t->n_items_per_type); */
      
    }

  for (i = 0; i < info->n_type_types[1]; i++)
    {
      t = &info->type_types[1][i];
      read(fdn, t, sizeof(struct type_type));
      t->info = info;
      t->diffs = malloc(sizeof(float)*t->n_types*t->n_types);
      t->diffs_order = malloc(sizeof(int16_t)*t->n_types*t->n_types);
      /* if (t->types != NULL) */
      /* 	t->types = malloc(sizeof(float)*t->n_types*t->n_items_per_type); */
    }
  
  t = &info->type_types[0][0];
  t->get_slot = &get_hand_slot_from_hi;
  fp = fopen("biggame/hand_diffs_gs0_n128_r32.ftype", "rb");
  fread(t->diffs, sizeof(float),t->n_types*t->n_types, fp);
  fclose(fp);

  fp = fopen("biggame/hand_diffs_order_gs0_n128_r32.ftype", "rb");
  fread(t->diffs_order, sizeof(int16_t), t->n_types*t->n_types, fp);
  fclose(fp);

  /* fp = fopen("biggame/hand_types_gs0_n128_r32.ftype", "rb"); */
  /* fread(t->types, sizeof(float), t->n_types*t->n_types, fp); */
  /* fclose(fp); */

  fp = fopen("biggame/slots_hand_gs0_n128_r32.slots", "rb");
  fseek(fp, 0L, SEEK_END);
  sz = ftell(fp);
  t->slots = malloc(sz);
  fseek(fp, 0L, SEEK_SET);
  fread(t->slots, sz, 1, fp);
  fclose(fp);

  /**/

  t = &info->type_types[0][1];
  t->get_slot = get_hand_slot_from_hi;
  fp = fopen("biggame/hand_diffs_gs1_n1024_r64.ftype", "rb");
  fread(t->diffs, sizeof(float), t->n_types*t->n_types, fp);
  fclose(fp);

  fp = fopen("biggame/hand_diffs_order_gs1_n1024_r64.ftype", "rb");
  fread(t->diffs_order, sizeof(int16_t), t->n_types*t->n_types, fp);
  fclose(fp);

  /* fp = fopen("biggame/hand_types_gs1_n1024_r64.ftype", "rb"); */
  /* fread(t->types, sizeof(float), t->n_types*t->n_types, fp); */
  /* fclose(fp); */

  fp = fopen("biggame/slots_hand_gs1_n1024_r64.slots", "rb");
  fseek(fp, 0L, SEEK_END);
  sz = ftell(fp);
  t->slots = malloc(sz);
  fseek(fp, 0L, SEEK_SET);
  fread(t->slots, sz, 1, fp);
  fclose(fp);

  /**/

  t = &info->type_types[0][2];
  t->get_slot = get_hand_slot_from_hi;
  fp = fopen("biggame/hand_diffs_gs2_n512_r128.ftype", "rb");
  fread(t->diffs, sizeof(float), t->n_types*t->n_types, fp);
  fclose(fp);

  fp = fopen("biggame/hand_diffs_order_gs2_n512_r128.ftype", "rb");
  fread(t->diffs_order, sizeof(int16_t), t->n_types*t->n_types, fp);
  fclose(fp);

  /* fp = fopen("biggame/hand_types_gs2_n512_r128.ftype", "rb"); */
  /* fread(t->types, sizeof(float), t->n_types*t->n_types, fp); */
  /* fclose(fp); */

  fp = fopen("biggame/slots_hand_gs2_n512_r128.slots", "rb");
  fseek(fp, 0L, SEEK_END);
  sz = ftell(fp);
  t->slots = malloc(sz);
  fseek(fp, 0L, SEEK_SET);
  fread(t->slots, sz, 1, fp);
  fclose(fp);

  /**/

  t = &info->type_types[0][3];
  t->get_slot = get_hand_slot_from_hi;
  fp = fopen("biggame/hand_diffs_gs3_n512_r3.ftype", "rb");
  fread(t->diffs, sizeof(float), t->n_types*t->n_types, fp);
  fclose(fp);

  fp = fopen("biggame/hand_diffs_order_gs3_n512_r3.ftype", "rb");
  fread(t->diffs_order, sizeof(int16_t), t->n_types*t->n_types, fp);
  fclose(fp);

  /* fp = fopen("biggame/hand_types_gs3_n512_r3.ftype", "rb"); */
  /* fread(t->types, sizeof(float), t->n_types*t->n_types, fp); */
  /* fclose(fp); */

  fp = fopen("biggame/river_slots", "rb");
  fseek(fp, 0L, SEEK_END);
  sz = ftell(fp);
  //t->slots = malloc(sz);
  fseek(fp, 0L, SEEK_SET);
 
  t->slots = mmap(NULL, sz, PROT_READ, MAP_PRIVATE, fileno(fp), 0);
  if (t->slots == MAP_FAILED)
    {
      perror("mmap:");
      exit(1);
    }
  //fread(t->slots, sz, 1, fp);
  fclose(fp);


  
  t = &info->type_types[1][1];
  t->get_slot = get_last_act_slot_from_hi;
  
  t = &info->type_types[1][2];
  t->get_slot = get_last_act_slot_from_hi;
  
  t = &info->type_types[1][3];
  t->get_slot = get_last_act_slot_from_hi;
  
  t = &info->type_types[1][4];
  t->get_slot = get_bets_slot_from_hi;
  
  t = &info->type_types[1][5];
  t->get_slot = get_bets_slot_from_hi;

  t = &info->type_types[1][6];
  t->get_slot = get_bets_slot_from_hi;

  
  /**/

  t = &info->type_types[1][9];
  t->get_slot = get_board_slot_from_hi;

  fp = fopen("biggame/board_diffs_gs1_s1024_tb64_te128.ftype", "rb");
  fread(t->diffs, sizeof(float), t->n_types*t->n_types, fp);
  fclose(fp);

  fp = fopen("biggame/board_diffs_order_gs1_s1024_tb64_te128.ftype", "rb");
  fread(t->diffs_order, sizeof(int16_t), t->n_types*t->n_types, fp);
  fclose(fp);

  /* fp = fopen("biggame/board_types_gs1_s1024_tb64_te128.ftype", "rb"); */
  /* fread(t->types, sizeof(float), t->n_types*t->n_types, fp); */
  /* fclose(fp); */

  fp = fopen("biggame/board_slots_gs1_s1024_tb64_te128.slots", "rb");
  fseek(fp, 0L, SEEK_END);
  sz = ftell(fp);
  t->slots = malloc(sz);
  fseek(fp, 0L, SEEK_SET);
  fread(t->slots, sz, 1, fp);
  fclose(fp);

  /**/

  t = &info->type_types[1][8];
  t->get_slot = get_board_slot_from_hi;

  fp = fopen("biggame/board_diffs_gs2_s1024_tb128_te128.ftype", "rb");
  fread(t->diffs, sizeof(float), t->n_types*t->n_types, fp);
  fclose(fp);

  fp = fopen("biggame/board_diffs_order_gs2_s1024_tb128_te128.ftype", "rb");
  fread(t->diffs_order, sizeof(int16_t), t->n_types*t->n_types, fp);
  fclose(fp);

  /* fp = fopen("biggame/board_types_gs2_s1024_tb128_te128.ftype", "rb"); */
  /* fread(t->types, sizeof(float), t->n_types*t->n_types, fp); */
  /* fclose(fp); */

  fp = fopen("biggame/board_slots_gs2_s1024_tb128_te128.slots", "rb");
  fseek(fp, 0L, SEEK_END);
  sz = ftell(fp);
  t->slots = malloc(sz);
  fseek(fp, 0L, SEEK_SET);
  fread(t->slots, sz,1, fp);
  fclose(fp);

  /**/ 

  t = &info->type_types[1][7];
  t->get_slot = get_board_slot_from_hi;

  fp = fopen("biggame/board_diffs_gs3_s1024_tb128_te128.ftype", "rb");
  fread(t->diffs, sizeof(float), t->n_types*t->n_types, fp);
  fclose(fp);

  fp = fopen("biggame/board_diffs_order_gs3_s1024_tb128_te128.ftype", "rb");
  fread(t->diffs_order, sizeof(int16_t), t->n_types*t->n_types, fp);
  fclose(fp);

  /* fp = fopen("biggame/board_types_gs3_s1024_tb128_te128.ftype", "rb"); */
  /* fread(t->types, sizeof(float), t->n_types*t->n_types, fp); */
  /* fclose(fp); */

  fp = fopen("biggame/board_slots_gs3_s1024_tb128_te128.slots", "rb");
  fseek(fp, 0L, SEEK_END);
  sz = ftell(fp);
  t->slots = malloc(sz);
  fseek(fp, 0L, SEEK_SET);
  fread(t->slots, sz, 1, fp);
  fclose(fp);
  return info;

}



void load_model_for_unique_root(struct gameinfo *info, struct unique_root *us, char *path)
{
  char filename[256];
  int len, fdn,i;
  
  if (us->root_idx >= 2 && us->gamestate < 4 && us->model_tree == NULL)
    {
      strcpy(filename, path);
      len = strlen(filename);
      
      sprintf(&filename[len], "(%i, %i, %i, %.1f, %.1f).model_tree", us->gamestate, us->to_act, us->cur_seat, us->bets[0], us->bets[1]);
      printf("TRYING %s\n", filename);
      fdn = open(filename, O_RDONLY);
      
      us->model_tree = load_plrmodel_tree(info, fdn, us);
      close(fdn);
    }
  for (i = 0; i < ACTS; i++)
    {
      if (us->next[i] != NULL)
	load_model_for_unique_root(info, us->next[i], path);
    }

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

struct global_data
{
  struct gameinfo *i;
  struct unique_root *r;
};


void *rgbr_init_private_info( char *arg )
{
  struct global_data *d = malloc(sizeof(struct global_data));

  int fdn;
  //FILE *fp;
  
  precalc_conversions();

  /* load shit */  
  memset(root_nodes, 0, sizeof(root_nodes));
  fdn = open("unique_roots", O_RDONLY);
  d->r = load_unique_roots(fdn,0);
  close(fdn);

  
  fdn = open("gameinfo", O_RDONLY);
  d->i = load_gameinfo(fdn);
  close(fdn);
 
  load_model_for_unique_root(d->i, d->r, arg);
 
  return d;
}

int rgbr_get_action_probs( const void *private_info,
                           int round,
                           const int board_cards[ RGBR_MAX_BOARD_CARDS ], 
                           const int num_actions[ RGBR_NUM_ROUNDS ], 
                           int actions[ RGBR_NUM_ROUNDS ][ RGBR_MAX_ACTIONS_IN_ROUND ],
                           const int num_private_hands,
                           int private_hands[ RGBR_MAX_HANDS ][ RGBR_NUM_HOLE_CARDS ],
                           double probs[ RGBR_MAX_HANDS ][ RGBR_NUM_ACTIONS ] )
{
  int16_t pub_types[9];
  int16_t priv_types[4];
  int16_t *private_types;
  int16_t global_type, local_type;
  int i, gs, act_i, flop_i, turn_i, river_i, hand_i;
  struct global_data *d = (struct global_data *)private_info;  
  struct hand_info *hi = get_blank_hi(d->r);
  struct plrmodel_node *n;
  struct situ *s;
  double fmul, rmul, tot;


  add_action_to_hi(hi, 0);
  add_action_to_hi(hi, 0);
	 	 
  memset(pub_types, 0, sizeof(pub_types));
  memset(priv_types, 0, sizeof(priv_types));
  pub_types[0] = 0;

  for (gs = 0; gs < round; gs++)
    {
      for (act_i = 0; act_i < num_actions[gs]; act_i++)
	{
	  switch (actions[gs][act_i])
	    {
	    case 0:
	      add_action_to_hi(hi, 2);
	      break;
	    case 1:
	      add_action_to_hi(hi, 1);
	      break;
	    case 2:
	      add_action_to_hi(hi, 0);
	      break;
	    default:
	      fprintf(stderr, "invalid action %i\n", actions[gs][act_i]);
	      return 1;
	      break;
	    }
	}
    }
  if (round >0)
    for (i = 0; i < round+2; i++)
      {
	hi->board[i] = a2z[board_cards[i]];
      }
  
  switch(round)
    {
    case 3:
      pub_types[7] = -1;
      pub_types[6] = -1;
      pub_types[3] = -1;
    case 2:
      pub_types[8] = -1;
      pub_types[5] = -1;
      pub_types[2] = -1;
    case 1:
      pub_types[9] = -1;
      pub_types[4] = -1;
      pub_types[1] = -1;
    default:
      break;
    }
  

  set_st_from_hand_info(d->i, hi, pub_types, priv_types);
  n = get_first_matching_situ(hi->cur_us->model_tree, pub_types, 0);
  s = (struct situ*)n->next_list;

  assert(round == n->t->gamestate);

  switch(round)
    {
    case 0:
      private_types = n->t->slots;
      break;
    case 1:
      flop_i = cards_to_int_3[hi->board[0]][hi->board[1]][hi->board[2]];
      private_types = &n->t->slots[flop_i*HANDS];
      break;
    case 2:
      turn_i = cards_to_int_4[hi->board[0]][hi->board[1]][hi->board[2]][hi->board[3]];
      private_types = &n->t->slots[turn_i*HANDS];
      break;
    case 3:
      river_i = get_river_index(hi->board);
      //private_types = &n->t->slots[river_i*HANDS];
      private_types = n->t->slots;
      private_types += river_i*HANDS;
      break;
    default:
      break;
    }

  if (n->type_mapping == NULL)
    {
      generate_mapping_from_global_to_local(n);
    }

  if (n->root->action_cost[1] == 0)
    fmul = 0;
  else
    fmul = 1;
  
  if (n->root->next[0] == NULL)
    rmul = 0;
  else
    rmul = 1;

  printf("sizeof pointer %i\n", sizeof(private_types));

  for (i = 0; i < num_private_hands; i++)
    {
      hand_i = cards_to_int_2[a2z[private_hands[i][0]]][a2z[private_hands[i][1]]];
      global_type = private_types[hand_i];
      local_type = n->type_mapping[global_type];
      tot = s->avg_odds[0][local_type]*rmul+s->avg_odds[1][local_type]+s->avg_odds[2][local_type]*fmul;
      if (tot > 0)
	{
	  probs[i][0] = s->avg_odds[2][local_type]*fmul/tot;
	  probs[i][1] = s->avg_odds[1][local_type]/tot;
	  probs[i][2] = s->avg_odds[0][local_type]*rmul/tot;
	}
      else
	{
	  probs[i][0] = 0;
	  probs[i][0] = 1;
	  probs[i][0] = 0;
	}
    }
  return 0;
}
	  
