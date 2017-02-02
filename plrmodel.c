#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <search.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>


//#include <valgrind/memcheck.h>

#include "defs.h"
#include "bit.h"
#include "plrmodel.h"
#include "handval.h"
#include "types.h"
#include "zconv.h"
extern CardMask int_to_cardmask_3[22100];
extern CardMask int_to_cardmask_2[1326];




void save_unique_roots(struct unique_root *r, int fdn, int branched)
{
  /* save shit */
  printf("saving idx %i\n", r->root_idx);
  write(fdn, r, sizeof(struct unique_root));
  write(fdn, r->bets, r->n_plr*sizeof(double));
  write(fdn, r->type_types_order[0], r->n_type_types[0]*sizeof(int));
  write(fdn, r->type_types_order[1], r->n_type_types[1]*sizeof(int));      


  if (r->next[0] != NULL)
    {
      save_unique_roots(r->next[0],fdn, branched);
      if (r->to_act == 2 && r->next[1] != NULL)
	{
	  save_unique_roots(r->next[1], fdn, 1);
	}
    }
  else
    {
      if (r->gamestate < 3 && branched == 0)
	{
	  save_unique_roots(r->next[1], fdn, 0);
	}
    }
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

  i = find_closest_type(n->t, n->types_bmap, global_type_i, n->len);
  return  bitcount_before_bitmap(n->types_bmap, i);
}

void pmn_unlock_struct(struct plrmodel_node *n)
{
  unlock_plrmodel_node_read(&n->struct_lock);
  
  if (n->prev != NULL)
    {
      pmn_unlock_struct(n->prev);
    }
}

struct plrmodel_node *get_first_matching_situ(struct plrmodel_node *n, int16_t *st, double expand_odds, gsl_rng *rng, int lock)
{
  int i;
  
  /* if (update_visits != -1) */
  /*   { */
  /*     n->visits[update_visits]++; */
  /* if (set_valid && n->t->public) */
  /*   set_bit(n->valid_bmap, st[n->t->id]); */
  /*   } */
      
  if (n->t->public == 0)
    {
      return n;
    }
  else
    {
      int16_t new_type;
      new_type = st[n->t->id];

      if (expand_odds > 0 && n->len < n->t->n_types && n->t->id != 0 /* ettei potsizee expandata */ && !is_bit_set(n->types_bmap, new_type))
	if (gsl_rng_uniform(rng) < expand_odds/*/n->pub_node_count*/)
	  {
	    int16_t closest_slot_i;
	    lock_plrmodel_node_write(&n->struct_lock);
	    if (!is_bit_set(n->types_bmap, new_type))
	      {
		closest_slot_i = get_closest_local_type(n, new_type);
		copy_slot_to_type(n, new_type, closest_slot_i);
	      }
	    //set_bit(n->expand_bmap, st[n->t->id]);
	    unlock_plrmodel_node_write(&n->struct_lock);
	  }
      if (lock)
	lock_plrmodel_node_read(&n->struct_lock);
      i = get_closest_local_type(n, st[n->t->id]);
      
      //get_n_closest_local_type(n, st[n->t->id], &i2, 1);
      //assert(i == i2);
      /* if (n->t->style == 2) */
      /* 	i = find_closest_type_diffs(n->t->diffs_order, n->types_bmap, n->t->n_types, st[n->t->id]); */
      /* else */
      /* 	i = find_closest_type_linear_int(n->types_bmap, n->t->n_types, st[n->t->id]); */
      /* i = bitcount_before_bitmap(n->types_bmap, i); */
      
      return get_first_matching_situ(n->next_list[i], st, expand_odds, rng, lock);
    }
  return NULL;
}

struct plrmodel_node *get_first_matching_situ_weighed_random(struct plrmodel_node *n, int16_t *st, double expand_odds, gsl_rng *rng, int lock)
{
  int i;
  
  /* if (update_visits != -1) */
  /*   { */
  /*     n->visits[update_visits]++; */
  /* if (set_valid && n->t->public) */
  /*   set_bit(n->valid_bmap, st[n->t->id]); */
  /*   } */
      
  if (n->t->public == 0)
    {
      return n;
    }
  else
    {
      int16_t new_type, next_type;
      int16_t closest_types[10];
      double weights[10];
      double tot_weight = 0;
      int get_n_types = 10;
      int types_found;
      double cutoff, sum;
      new_type = st[n->t->id];

      if (expand_odds > 0 && n->len < n->t->n_types && n->t->id != 0 /* ettei potsizee expandata */ && !is_bit_set(n->types_bmap, new_type))
	if (gsl_rng_uniform(rng) < expand_odds/*/n->pub_node_count*/)
	  {
	    int16_t closest_slot_i;
	    lock_plrmodel_node_write(&n->struct_lock);
	    if (!is_bit_set(n->types_bmap, new_type))
	      {
		closest_slot_i = get_closest_local_type(n, new_type);
		copy_slot_to_type(n, new_type, closest_slot_i);
	      }
	    //set_bit(n->expand_bmap, st[n->t->id]);
	    unlock_plrmodel_node_write(&n->struct_lock);
	  }
      if (lock)
	lock_plrmodel_node_read(&n->struct_lock);
      i = get_closest_local_type(n, st[n->t->id]);
      if (n->t->id < 7)
	get_n_types = 1;
      else
	if (n->len < get_n_types)
	  get_n_types = n->len;
      //types_found = find_n_closest_type_valid_edge(n, st[n->t->id], closest_types, weights, get_n_types);
      types_found = find_n_closest_type_valid_edge(n->t, n->types_bmap, st[n->t->id], closest_types, weights, get_n_types);
      assert(types_found > 0);
      if (types_found > 1)
	{
	  for (i = 0; i < types_found; i++)
	    {
	      tot_weight += 1.0/weights[i];
	    }
	  cutoff = gsl_rng_uniform(rng)*tot_weight;
	  sum = 0;
	  for (i = 0; i < types_found; i++)
	    {
	      sum += 1.0/weights[i];
	      if (cutoff < sum)
		break;
	    }
	  assert(i < types_found);
	}
      else
	i = 0;
      
      next_type = type_to_slot(n->types_bmap, closest_types[i]);
      //assert(i == i2);
      /* if (n->t->style == 2) */
      /* 	i = find_closest_type_diffs(n->t->diffs_order, n->types_bmap, n->t->n_types, st[n->t->id]); */
      /* else */
      /* 	i = find_closest_type_linear_int(n->types_bmap, n->t->n_types, st[n->t->id]); */
      /* i = bitcount_before_bitmap(n->types_bmap, i); */
      
      return get_first_matching_situ_weighed_random(n->next_list[next_type], st, expand_odds, rng, lock);
    }
  return NULL;
}


void get_action_odds_from_file(int fdn, struct gameinfo *info, int16_t *pub_types, int16_t *priv_types, float *odds, int from)
{
  struct plrmodel_node n;
  int i, public, slot_i;
  //struct f3 retval;
  double tot;
  read(fdn,&i, sizeof(n.t->id));
  read(fdn,&public, sizeof(n.t->public));
  n.t = &info->type_types[public][i];
  n.types_bmap = alloc_bitfield(n.t->n_types);
  read(fdn, n.types_bmap, bitfield_bytesize(n.t->n_types));
  n.len = bitcount_before_bitmap(n.types_bmap, n.t->n_types);
  
  if (n.t->public == 0)
    {
      struct situ s;
      read(fdn, &s, sizeof(struct situ));

      for (i = 0; i < ACTS; i++)
	{
	  if (s.regs[i] != NULL)
	    {
	      s.regs[i] = malloc(sizeof(s.regs[i][0]) * n.len);
	      read(fdn, s.regs[i], sizeof(s.regs[i][0]) * n.len);
	    }
	  /* if (s.d_regs[i] != NULL) */
	  /*   { */
	  /*     s.d_regs[i] = malloc(sizeof(s.d_regs[i][0]) * n.len); */
	  /*     read(fdn, s.d_regs[i], sizeof(s.d_regs[i][0]) * n.len); */
	  /*   } */
	  if (s.avg_odds[i] != NULL)
	    {
	       s.avg_odds[i] = malloc(sizeof(s.avg_odds[i][0]) * n.len);
	       read(fdn, s.avg_odds[i], sizeof(s.avg_odds[i][0]) * n.len);
	    }
	  /* if (i < ACTS-1) */
	  /*   { */
	  /*     /\* if (s.hand_odds[i] != NULL) *\/ */
	  /*     /\* 	{ *\/ */
	  /*     /\* 	  s.hand_odds[i] = malloc(sizeof(s.hand_odds[i][0]) * n.len); *\/ */
	  /*     /\* 	  read(fdn, s.hand_odds[i], sizeof(s.hand_odds[i][0]) * n.len); *\/ */
	  /*     /\* 	} *\/ */
	  /*     if (s.ev[i] != NULL) */
	  /* 	{ */
	  /* 	  s.ev[i] = malloc(sizeof(s.ev[i][0]) * n.len); */
	  /* 	  read(fdn, s.ev[i], sizeof(s.ev[i][0]) * n.len); */
	  /* 	} */
	  /*     if (s.ev_count[i] != NULL) */
	  /* 	{ */
	  /* 	  s.ev_count[i] = malloc(sizeof(s.ev_count[i][0]) * n.len); */
	  /* 	  read(fdn, s.ev_count[i], sizeof(s.ev_count[i][0]) * n.len); */
	  /* 	} */
	  /*   } */
	}
      /* if (s.visits != NULL) */
      /* 	{ */
      /* 	  s.visits = malloc(sizeof(s.visits[0]) * n.len); */
      /* 	  read(fdn, s.visits, sizeof(s.visits[0]) * n.len); */
      /* 	} */
      n.next_list = (void*)&s;
      slot_i = get_closest_local_type(&n, priv_types[n.t->id]);
      tot = 0.0;
      //printf("DATA %f %f %f, %f %f %f, %f %f , %i %i\n", s.avg_odds[0][slot_i], s.avg_odds[1][slot_i], s.avg_odds[2][slot_i], s.regs[0][slot_i], s.regs[1][slot_i], s.regs[2][slot_i],s.ev[0][slot_i], s.ev[1][slot_i], s.ev_count[0][slot_i], s.ev_count[1][slot_i]);
      switch(from)
	{
	case 0:
	  for (i = 0; i < ACTS; i++)
	    {
	      odds[i] = s.avg_odds[i][slot_i];
	      tot += odds[i];
	    }
	  break;

	case 1:
	  for (i = 0; i < ACTS; i++)
	    {
	      odds[i] = s.regs[i][slot_i] > 0?s.regs[i][slot_i]:0;
	      tot += odds[i];
	    }
	  break;
	  
	/* case 2: */
	/*   for (i = 0; i < ACTS; i++) */
	/*     odds[i] = 0; */
	/*   act_i = 2; */
	/*   if (s.ev[0][slot_i] > 0 && s.ev[0][slot_i] > s.ev[1][slot_i]) */
	/*     act_i = 0; */
	/*   else if (s.ev[1][slot_i] > 0) */
	/*     act_i = 1; */
	/*   odds[act_i] = 1; */
	/*   tot = 1; */
	/*   break; */
	}
      if (tot > 0)
	for (i = 0; i < ACTS; i++)
	  odds[i] /= tot;
      else
	{
	  printf("NO DATA %f %f %f, %f %f %f\n", s.avg_odds[0][slot_i], s.avg_odds[1][slot_i], s.avg_odds[2][slot_i], s.regs[0][slot_i], s.regs[1][slot_i], s.regs[2][slot_i]);
	  odds[0] = 0.333333333333333;
	  odds[1] = 0.3333333333333333;
	  odds[2] = 0.3333333333333333; 
	}
      free(n.types_bmap);
      for (i = 0; i < sizeof(struct situ)/sizeof(void*); i++)
	if (n.next_list[i] != NULL)
	  free(n.next_list[i]);
      
    }
  else
    {
      uint64_t next_pos;
      int slot_i;
      slot_i = get_closest_local_type(&n, pub_types[n.t->id]);
      lseek(fdn, sizeof(uint64_t)*slot_i, SEEK_CUR);
      read(fdn, &next_pos, sizeof(uint64_t));
      lseek(fdn, next_pos, SEEK_SET);
      
      get_action_odds_from_file(fdn, info, pub_types, priv_types, odds, from);
    }
}





void reset_plrmodel_data(struct plrmodel_node *n, uint64_t flags)
{
  int i;

  
  
  if (flags&PM_RESET_TYPE_MAPPING)
    if (n->type_mapping != NULL)
      {
	free(n->type_mapping);
	n->type_mapping = NULL;
      }
    
  if (!n->t->public)
    {
      struct situ *s;
      if (n->next_list == NULL)
	n->next_list = calloc(1,sizeof(struct situ));
      s = (struct situ*)n->next_list;
      /* if (flags&SITU_RESET_HAND_ODDS) */
      /* 	for (i = 0; i < ACTS;i++) */
      /* 	  if (s->hand_odds[i] != NULL) */
      /* 	    memset(s->hand_odds[i], 0, sizeof(s->hand_odds[i][0])*n->len); */
      
      if (flags&SITU_RESET_REGS)
	{
	  for (i = 0; i < ACTS;i++)
	    {
	      if (s->regs[i] == NULL)
		{
		  s->regs[i] = malloc(sizeof(s->regs[i][0])*n->len);
		}
	      memset(s->regs[i], 0, sizeof(s->regs[i][0])*n->len);
	    }
	  if (n->root->next[0] == NULL)
	    {
	      for (i = 0; i < n->len; i++)
		{
		  s->regs[0][i] = -1000000;
		}
	    }
	  if (n->root->action_cost[1] == 0)
	    {
	      for (i = 0; i < n->len; i++)
		{
		  s->regs[2][i] = -1000000;
		}
	    }
	}

      if (flags&SITU_RESET_AVG_ODDS)
	for (i = 0; i < ACTS;i++)
	  {
	    if (s->avg_odds[i] == NULL)
	      {
		s->avg_odds[i] = malloc(sizeof(s->avg_odds[i][0])*n->len);
	      }
	    memset(s->avg_odds[i], 0, sizeof(s->avg_odds[i][0])*n->len);
	  }

    }
}

void recursive_reset_plrmodel_data(struct plrmodel_node *n, uint64_t flags)
{
  int i;

  if (n->t->public)
    {
      for (i = 0; i < n->len; i++)
	{
	  recursive_reset_plrmodel_data(n->next_list[i], flags);
	}
    }
  reset_plrmodel_data(n, flags);
}

uint64_t count_hands(struct plrmodel_node *n)
{
  int i;
  uint64_t hand_count = 0;
  if (n->t->public)
    {
      for (i = 0; i < n->len; i++)
	{
	  hand_count += count_hands(n->next_list[i]);
	}
    }
  else
    {
      hand_count = n->len;
    }
  return hand_count;
}

uint64_t count_zero_regs_hands(struct plrmodel_node *n)
{
  int i;
  uint64_t hand_count = 0;
  if (n->t->public)
    {
      for (i = 0; i < n->len; i++)
	{
	  hand_count += count_zero_regs_hands(n->next_list[i]);
	}
    }
  else
    {
      struct situ *s = (struct situ*)n->next_list;
      for (i = 0; i < n->len;i++)
	{
	  if (s->regs[0][i] + s->regs[1][i] +s->regs[2][i] == 0)
	    hand_count++;
	}
    }
  return hand_count;
}

uint64_t count_zero_avg_hands(struct plrmodel_node *n)
{
  int i;
  uint64_t hand_count = 0;
  if (n->t->public)
    {
      for (i = 0; i < n->len; i++)
	{
	  hand_count += count_zero_avg_hands(n->next_list[i]);
	}
    }
  else
    {
      struct situ *s = (struct situ*)n->next_list;
      for (i = 0; i < n->len;i++)
	{
	  if (s->avg_odds[0][i] + s->avg_odds[1][i] +s->avg_odds[2][i] == 0)
	    hand_count++;
	}
    }
  return hand_count;
}

uint64_t prune_zero_avg_hands(struct plrmodel_node *n)
{
  int i;
  uint64_t hand_count = 0;
  if (n->t->public)
    {
      for (i = 0; i < n->len; i++)
	{
	  hand_count += prune_zero_avg_hands(n->next_list[i]);
	}
    }
  else
    {
      struct situ *s = (struct situ*)n->next_list;
      int changed = 0;

      for (i = 0; i < n->len && n->len > 1;i++)
	{
	  if (s->avg_odds[0][i] + s->avg_odds[1][i] +s->avg_odds[2][i] == 0)
	    {
	      changed++;
	      remove_hand_from_pmn(n, i);
	      i--;
	    }
	  //	  hand_count++;
	}
      if (changed)
	{
	  if (n->type_mapping != NULL)
	    memset(n->type_mapping, 0xff, sizeof(int16_t)*n->t->n_types);
	}
      hand_count += changed;
    }
  return hand_count;
}

uint64_t prune_zero_regs_hands(struct plrmodel_node *n)
{
  int i;
  uint64_t hand_count = 0;
  if (n->t->public)
    {
      for (i = 0; i < n->len; i++)
	{
	  hand_count += prune_zero_regs_hands(n->next_list[i]);
	}
    }
  else
    {
      struct situ *s = (struct situ*)n->next_list;
      int changed = 0;

      for (i = 0; i < n->len && n->len > 1;i++)
	{
	  if (s->regs[0][i] + s->regs[1][i] +s->regs[2][i] == 0)
	    {
	      changed++;
	      remove_hand_from_pmn(n, i);
	      i--;
	    }
	  //	  hand_count++;
	  
	}
      if (changed)
	{
	  if (n->type_mapping != NULL)
	    memset(n->type_mapping, 0xff, sizeof(int16_t)*n->t->n_types);
	}
      hand_count += changed;
    }
  return hand_count;
}


uint32_t recount_pub_nodes(struct plrmodel_node *n)
{
  int i;
  if (n->t->public)
    {
      n->pub_node_count = 0;
      for (i = 0; i < n->len; i++)
	{
	  n->pub_node_count += recount_pub_nodes(n->next_list[i]);
	}
    }
  else
    {
      //n->pub_node_count = n->len;
      n->pub_node_count = 1;
    }
  return n->pub_node_count;
}


uint64_t recount_data_index(struct plrmodel_node *n, uint64_t data_index)
{
  int i;
  n->data_index = data_index;
  if (n->t->public)
    {
      for (i = 0; i < n->len; i++)
	{
	  data_index = recount_data_index(n->next_list[i], data_index);
	}
    }
  else
    {
      data_index += n->len;
    }
  return data_index;
}




void reduce_plrmodel_randomly(struct plrmodel_node *n, double delete_odds, gsl_rng *rng)
{
  int i;
  if (delete_odds == 0)
    return;
  if ((n->t->public && n->t->id > 6) || !n->t->public)
    {
      i = 0;
      lock_plrmodel_node_write(&n->struct_lock);
      while (i < n->len && n->len > 1)
	{
	  if (gsl_rng_uniform(rng) < delete_odds)
	    {
	      if (n->t->public)
		remove_pmn_from_pmn(n, i);
	      else
		{
		  remove_hand_from_pmn(n, i);
		  if (n->type_mapping != NULL)
		    memset(n->type_mapping, 0xff, sizeof(int16_t)*n->t->n_types);
		}
	    }
	  else
	    i++;
	}
      unlock_plrmodel_node_write(&n->struct_lock);
    }
}

/* void set_plrmodel_to_expand_bmap(struct plrmodel_node *n) */
/* { */
/*   int i, closest_slot_i; */

/*   i = -1; */
/*   while ((i = biterate(n->expand_bmap, i, n->t->n_types)) != -1) */
/*     { */
/*       if (!is_bit_set(n->types_bmap, i)) */
/* 	{ */
/* 	  closest_slot_i = get_closest_local_type(n, i); */
/* 	  copy_slot_to_type(n, i, closest_slot_i); */
/* 	} */
/*     } */
/* } */

void adjust_plrmodel_tree(struct plrmodel_node *n, double pub_delete_odds, double priv_delete_odds, gsl_rng *rng, uint64_t retval[3])
{
  int i;
  
  if (n->t->public)
    reduce_plrmodel_randomly(n, pub_delete_odds, rng);
  else
    reduce_plrmodel_randomly(n, priv_delete_odds, rng);
  //set_plrmodel_to_expand_bmap(n);
  //memset(n->expand_bmap, 0, bitfield_bytesize(n->t->n_types));
  
  if (n->t->public)
    {
      lock_plrmodel_node_read(&n->struct_lock);

      retval[2]+=1;
      for (i = 0; i < n->len;i++)
	adjust_plrmodel_tree(n->next_list[i], pub_delete_odds, priv_delete_odds, rng, retval);
      unlock_plrmodel_node_read(&n->struct_lock);
    }
  else
    {
      retval[1]+=1;
      retval[0]+=n->len;
    }
}




int16_t copy_slot_to_type(struct plrmodel_node *n, int16_t type_i, int16_t slot_i)
{
  
  
  if (n->t->public)
    {
      struct plrmodel_node *n_copy, *child;
      child = n->next_list[slot_i];
      n_copy = copy_pmn(child, 0);
      return add_pmn_to_pmn(n, n_copy, type_i);
    }
  else
    {
      int16_t new_slot_i;
      new_slot_i = add_empty_hand_to_situ(n, type_i);
      if (new_slot_i <= slot_i)
	slot_i++;
      copy_situ_data(n, new_slot_i, slot_i);
      return new_slot_i;
    }
}


  
int walk_all_situs_callback(struct plrmodel_node *root_n, int( *callback)(struct plrmodel_node*))
{
  int i;

  if (root_n->t->public == 0)
    {
      return callback(root_n);
    }
  else
    {
      for (i = 0; i < root_n->len; i++)
	{
	  if (root_n->next_list[i] == NULL)
	    printf("EIEIEIERIKKIRKKII\n");
	  if (!walk_all_situs_callback(root_n->next_list[i], callback))
	    return 0;
	}
    }
  return 1;
}


      



void *insert_data(void *d, void *item, int item_len, int n_items, int pos)
{
  uint8_t *d_c;
  d = realloc(d, item_len*(n_items+1));
  d_c = d;
  memmove(&d_c[pos*item_len+item_len], &d_c[pos*item_len], item_len*(n_items-pos));
  if (item != NULL)
    memcpy(&d_c[pos*item_len], item, item_len);
  else
    memset(&d_c[pos*item_len], 0, item_len);
  return d;
}

void *pop_data(void *d, void *item, int item_len, int n_items, int pos)
{
  uint8_t *d_c = d;
  if (item != NULL)
    memcpy(item, &d_c[pos*item_len], item_len);
  n_items--;
  memmove( &d_c[pos*item_len], &d_c[pos*item_len+item_len], item_len*(n_items-pos));
  d = realloc(d, item_len*n_items);
  return d;
}

struct situ *copy_situ(struct plrmodel_node *n)
{
  struct situ *s_orig, *s_copy;
  int i;

  s_orig = (struct situ*)n->next_list;
  s_copy = calloc(1, sizeof(struct situ));

  for (i = 0; i < ACTS; i++)
    {
     	
      if (s_orig->regs[i] != NULL)
	{
	  s_copy->regs[i] = malloc(sizeof(s_orig->regs[0][0])*n->len);
	  memcpy(s_copy->regs[i], s_orig->regs[i], sizeof(s_orig->regs[0][0])*n->len);
	}
      if (s_orig->avg_odds[i] != NULL)
	{
	  s_copy->avg_odds[i] = malloc(sizeof(s_copy->avg_odds[0][0])*n->len);
	  memcpy(s_copy->avg_odds[i], s_orig->avg_odds[i], sizeof(s_copy->avg_odds[0][0])*n->len);
	}
    }
  return s_copy;
}

int count_pmn(struct plrmodel_node *n)
{
  int i, pmn_count = 1;

  if (n->t->public)
    for (i = 0; i < n->len; i++)
      pmn_count += count_pmn(n->next_list[i]);
  return pmn_count;
}


struct plrmodel_node *copy_pmn(struct plrmodel_node *n, uint64_t timestamp)
{
  struct plrmodel_node *copy_n;
  int i;

  copy_n = calloc(1, sizeof(struct plrmodel_node));
  copy_n->t = n->t;
  copy_n->len = n->len;
  copy_n->slot_in_prev = n->slot_in_prev ;
  //copy_n->hand_count = n->hand_count ;  
  copy_n->root = n->root ;
 
  //  memcpy(copy_n, n, sizeof(struct plrmodel_node));
  


  copy_n->types_bmap = alloc_bitfield(copy_n->t->n_types);
  memcpy(copy_n->types_bmap, n->types_bmap, bitfield_bytesize(copy_n->t->n_types));
  

  //copy_n->timestamp = timestamp;
  //memset(copy_n->visits, 0, sizeof(copy_n->visits));
 
  if (n->t->public)
    {
      //copy_n->valid_bmap = alloc_bitfield(copy_n->t->n_types);
      //memcpy(copy_n->valid_bmap, n->valid_bmap, bitfield_bytesize(copy_n->t->n_types));
      copy_n->next_list = malloc(sizeof(void*)*copy_n->len);
      for (i = 0; i < n->len; i++)
	{
	  copy_n->next_list[i] = copy_pmn(n->next_list[i], timestamp);
	  ((struct plrmodel_node *)copy_n->next_list[i])->prev = copy_n;
	}
    }      
  else
    {
      copy_n->next_list = (void**)copy_situ(n);
    }
  return copy_n;
}

void insert_pmn_to_next_list(struct plrmodel_node *d, struct plrmodel_node *n, int pos)
{
  int i;
  d->next_list = insert_data(d->next_list, &n, sizeof(void*), d->len, pos);
  //d->hand_count += n->hand_count;
  d->len++;
  for (i = pos; i < d->len; i++)
    ((struct plrmodel_node*)d->next_list[i])->slot_in_prev = i;
}




struct plrmodel_node *gen_pmn_based_on_type_type(struct type_type *t)
{
  struct plrmodel_node *new_n;
  new_n = calloc(1,sizeof(struct plrmodel_node));
  new_n->t = t;
  new_n->types_bmap = alloc_bitfield(t->n_types);
  //if (t->public)
  //  new_n->valid_bmap = alloc_bitfield(t->n_types);
  return new_n;
}

struct plrmodel_node *gen_pmn(struct gameinfo *info, struct plrmodel_node *n, struct unique_root *root, struct type_type *t)
{
  struct plrmodel_node *new_n;
  //  struct type_type *new_t;


  new_n = gen_pmn_based_on_type_type(t);
  new_n->root = root;
  new_n->prev = n;
  if (t->public == 0)
    new_n->next_list = calloc(1, sizeof(struct situ));
  return new_n;
}

void recursive_delete_pmn(struct plrmodel_node *n)
{
  int i;
  if (n->t->public)
    for (i = 0; i < n->len; i++)
      recursive_delete_pmn(n->next_list[i]);
  else
    free_situ(n->next_list);
  
  if (n->types_bmap != NULL)
    free(n->types_bmap);
  
  //if (n->valid_bmap != NULL)
  // free(n->valid_bmap);
  
  if (n->type_mapping != NULL)
    free(n->type_mapping);
  free(n);
}

void remove_pmn_from_pmn(struct plrmodel_node *n, int16_t slot_i)
{
  struct plrmodel_node *rem_n;
  int16_t type_i;

  type_i = get_nth_set_bit_bitmap(n->types_bmap, slot_i);
  clear_bit(n->types_bmap, type_i);
  
  rem_n = n->next_list[slot_i];
  n->next_list = pop_data(n->next_list, NULL, sizeof(void*), n->len, slot_i);
  //n->hand_count -= rem_n->hand_count;  
  n->len--;
  recursive_delete_pmn(rem_n);
}  

int16_t add_pmn_to_pmn(struct plrmodel_node *n, struct plrmodel_node *new_n, int16_t type_i)
{
  int new_pos, i;
  assert(!is_bit_set(n->types_bmap, type_i));
  
  new_pos = bitcount_before_bitmap(n->types_bmap, type_i);
  new_n->slot_in_prev = new_pos;
  new_n->prev = n;  
  set_bit(n->types_bmap, type_i);
  
  n->next_list = insert_data(n->next_list, &new_n, sizeof(void*), n->len, new_pos);
  //n->hand_count += new_n->hand_count;
  n->len++;
  for (i = new_pos; i < n->len; i++)
    ((struct plrmodel_node*)n->next_list[i])->slot_in_prev = i;
  return new_pos;
  //insert_plrmodel_node_to_next_list(n, new_n, new_pos);
}


int16_t remove_hand_from_pmn(struct plrmodel_node *n, int16_t slot_i)
{
  int i;
  int16_t type_i;
  struct situ *s = (struct situ*)n->next_list;
  
  if (n->len == 1)
    return -1;
  assert(n->len > 1);
  
  type_i = get_nth_set_bit_bitmap(n->types_bmap, slot_i);
  clear_bit(n->types_bmap, type_i);
  
  for (i = 0; i < ACTS; i++)
    {
      if (s->regs[i] != NULL)
	s->regs[i] = pop_data(s->regs[i], NULL, sizeof(s->regs[i][0]), n->len, slot_i);
      /* if (s->d_regs[i] != NULL) */
      /* 	s->d_regs[i] = pop_data(s->d_regs[i], NULL, sizeof(s->d_regs[i][0]), n->len, slot_i); */
      if (s->avg_odds[i] != NULL)
	s->avg_odds[i] = pop_data(s->avg_odds[i], NULL, sizeof(s->avg_odds[i][0]), n->len, slot_i);

    }
  n->len--;
  return type_i;  
}
void copy_situ_data(struct plrmodel_node *n, int16_t ts, int16_t ss)
{
  int i;
  struct situ *s = (struct situ*)n->next_list;
  for (i = 0; i < ACTS; i++)
    {
      if (s->regs[i] != NULL)
	s->regs[i][ts] = s->regs[i][ss];
      /* if (s->d_regs[i] != NULL) */
      /* 	s->d_regs[i][ts] = s->d_regs[i][ss]; */
      if (s->avg_odds[i] != NULL)
	s->avg_odds[i][ts] = s->avg_odds[i][ss];
    }
}
  

int16_t add_empty_hand_to_situ(struct plrmodel_node *n, int16_t type_i)
{
  int slot, i;
  struct situ *s = (struct situ*)n->next_list;
  if (is_bit_set(n->types_bmap, type_i))
    return -1;
  slot = type_to_slot(n->types_bmap, type_i);
  //slot = bitcount_before_bitmap(n->types_bmap, type_i);
  set_bit(n->types_bmap, type_i);
  for (i = 0; i < ACTS; i++)
    {
      //if (s->hand_odds[i] != NULL || n->len == 0)
      //s->hand_odds[i] = insert_data(s->hand_odds[i], NULL, sizeof(s->hand_odds[i][0]), n->len, slot);
      if (s->regs[i] != NULL/* || n->len == 0*/)
	s->regs[i] = insert_data(s->regs[i], NULL, sizeof(s->regs[i][0]), n->len, slot);
      //if (s->d_regs[i] != NULL || n->len == 0)
      //	s->d_regs[i] = insert_data(s->d_regs[i], NULL, sizeof(s->d_regs[i][0]), n->len, slot);
      if (s->avg_odds[i] != NULL/* || n->len == 0*/)
	s->avg_odds[i] = insert_data(s->avg_odds[i], NULL, sizeof(s->avg_odds[i][0]), n->len, slot);
    }
  n->len++;
  
  return slot;
}

int16_t get_random_unset_type(struct plrmodel_node *n)
{
  int type_i;
  /* if (force_valid && n->valid_bmap != NULL && bitcount_before_bitmap(n->valid_bmap, n->t->n_types) <= n->len) */
  /*   return -1; */
  if (n->len >= n->t->n_types)
    return -1;
  do
    {
      type_i = zrandom_r()%n->t->n_types;
    }
  while (is_bit_set(n->types_bmap, type_i));
  return type_i;
}

void expand_type_max(struct plrmodel_node *n, int public, int id)
{
  int i, closest_slot_i;

  if (n->t->public == public && n->t->id == id)
    {
      for (i = 0; i < n->t->n_types;i++)
	{
	  if (public && id == 4 && i == 0)
	    continue;
	  if (!is_bit_set(n->types_bmap, i))
	    {
	      closest_slot_i = get_closest_local_type(n, i);
	      copy_slot_to_type(n, i, closest_slot_i);
	    }
	}
    }
  else if (n->t->public)
    {
      for(i = 0; i < n->len; i++)
	{
	  expand_type_max(n->next_list[i], public, id);
	}
    }
}
	    

int add_random_hand_to_situ(struct plrmodel_node *n)
{
  int type_i;
  type_i = get_random_unset_type(n);
  if (type_i != -1)
    add_empty_hand_to_situ(n, type_i);
  return type_i;
}

struct plrmodel_node *gen_minimal_plrmodel_tree(struct gameinfo *info, struct unique_root *root, uint64_t timestamp)
{
  struct plrmodel_node *cur_n = NULL, *new_n = NULL, *retval = NULL;
  struct type_type *t;

  int slot_i = 0, type_i, i, p;
  for (p = 1; p >= 0; p--)
    {
      for (i = 0; i < root->n_type_types[p]; i++)
	{
	  type_i = root->type_types_order[p][i];
	  t = &info->type_types[p][type_i];
	  new_n = gen_pmn(info, cur_n, root, t);
	  //new_n->timestamp = timestamp;
	  if (retval == NULL)
	    retval = new_n;
	  if (cur_n != NULL)
	    {
	      slot_i = zrandom_r()%(cur_n->t->n_types-1)+1;
	      new_n->slot_in_prev = slot_i;
	      add_pmn_to_pmn(cur_n, new_n, slot_i);
	    }
	  cur_n = new_n;
	}
    }
  slot_i = add_random_hand_to_situ(cur_n);
  return retval;
}



void print_plrmodel_tree(struct plrmodel_node *n, int tabs)
{
  int i, t;
  
  
  //printf("%i\t", n->hand_count);
  if (n->t->public)
    for (i = 0; i < n->len; i++)
      {
	print_plrmodel_tree(n->next_list[i], tabs+1);
	printf("\n");
	for (t = 0; t < tabs+1; t++)
	  printf("\t");

      }
  else
    {
      printf("len: %i", n->len);
    }
}


void save_plrmodel_struct(int fdn, struct plrmodel_node *n)
{
  int i;
  //uint64_t bytes_written = 0;
  write(fdn, &n->t->id, sizeof(n->t->id));
  write(fdn, &n->t->public, sizeof(n->t->public));
  write(fdn, &n->data_index, sizeof(n->data_index));
  write(fdn, n->types_bmap, bitfield_bytesize(n->t->n_types));
  
  if (n->t->public)
    {
      uint64_t next_offset, cur_pos;
      next_offset = lseek(fdn, 0, SEEK_CUR);
      write(fdn, n->next_list, sizeof(uint64_t)*n->len);
      for (i = 0; i < n->len;i++)
	{
	  cur_pos = lseek(fdn, 0, SEEK_CUR);
	  lseek(fdn, next_offset, SEEK_SET);
	  write(fdn, &cur_pos, sizeof(cur_pos));
	  next_offset = lseek(fdn, 0, SEEK_CUR);
	  lseek(fdn, cur_pos, SEEK_SET);
	  save_plrmodel_struct(fdn, n->next_list[i]);
	}
    }
      
}

void save_plrmodel_byte_odds_from_avg(int fdn, struct plrmodel_node *n)
{
  int i, slot_i;
  if (n->t->public == 0)
    {
      double tot;
      //uint8_t byte_odds[ACTS-1];
      uint8_t byte_odds[n->len*2];
      struct situ *s = (struct situ*)n->next_list;
      for (slot_i = 0; slot_i < n->len; slot_i++)
	{
	  tot = s->avg_odds[0][slot_i] + s->avg_odds[1][slot_i] + s->avg_odds[2][slot_i];
	  if (tot > 0)
	    {
	      for (i = 0; i < ACTS-1; i++)
		byte_odds[slot_i*2+i] = (uint8_t)lround(255.0*s->avg_odds[i][slot_i]/tot);
	    }
	  else
	    {
	      byte_odds[slot_i*2] = 0xff;
	      //byte_odds[1] = 0xff;
	      byte_odds[slot_i*2+1] = 0xff;
	    }	  
	}
      write(fdn, byte_odds, sizeof(byte_odds));
    }
  else
    {
      for (i = 0; i < n->len;i++)
	{
	  save_plrmodel_byte_odds_from_avg(fdn, n->next_list[i]);
	}
    }
}

void save_plrmodel_byte_odds_from_regs(int fdn, struct plrmodel_node *n)
{
  int i, slot_i;
  if (n->t->public == 0)
    {
      double tot;
      //uint8_t byte_odds[ACTS-1];
      uint8_t byte_odds[n->len*2];
      struct situ *s = (struct situ*)n->next_list;
      for (slot_i = 0; slot_i < n->len; slot_i++)
	{
	  tot = (s->regs[0][slot_i]>0?s->regs[0][slot_i]:0) + (s->regs[1][slot_i]>0?s->regs[1][slot_i]:0) + (s->regs[2][slot_i]>0?s->regs[2][slot_i]:0);
	  if (tot > 0)
	    {
	      for (i = 0; i < ACTS-1; i++)
		byte_odds[slot_i*2+i] = (uint8_t)lround(255.0*(s->regs[i][slot_i]>0?s->regs[i][slot_i]:0)/tot);
	    }
	  else
	    {
	      byte_odds[slot_i*2] = 0xff;
	      //byte_odds[1] = 0xff;
	      byte_odds[slot_i*2+1] = 0xff;
	    }	  
	}
      write(fdn, byte_odds, sizeof(byte_odds));
    }
  else
    {
      for (i = 0; i < n->len;i++)
	{
	  save_plrmodel_byte_odds_from_regs(fdn, n->next_list[i]);
	}
    }
}


void save_plrmodel_avg_odds(int fdn, struct plrmodel_node *n)
{
  int i;
  if (n->t->public == 0)
    {
      struct situ *s = (struct situ*)n->next_list;
      for (i = 0; i < ACTS; i++)
	{
	  assert(s->avg_odds[i] != NULL);
	  write(fdn, s->avg_odds[i], sizeof(s->avg_odds[i][0]) * n->len);
	}
    }
  else
    {
      for (i = 0; i < n->len;i++)
	{
	  save_plrmodel_avg_odds(fdn, n->next_list[i]);
	}
    }
}


void save_plrmodel_regs(int fdn, struct plrmodel_node *n)
{
  int i;
  if (n->t->public == 0)
    {
      struct situ *s = (struct situ*)n->next_list;
      for (i = 0; i < ACTS; i++)
	{
	  assert(s->regs[i] != NULL);
	  write(fdn, s->regs[i], sizeof(s->regs[i][0]) * n->len);
	}
    }
  else
    {
      for (i = 0; i < n->len;i++)
	{
	  save_plrmodel_regs(fdn, n->next_list[i]);
	}
    }
}


void save_plrmodel_tree(int fdn, struct plrmodel_node *n)
{
  int i;
  //uint64_t bytes_written = 0;
  write(fdn, &n->t->id, sizeof(n->t->id));
  write(fdn, &n->t->public, sizeof(n->t->public));
  write(fdn, n->types_bmap, bitfield_bytesize(n->t->n_types));
  if (n->t->public == 0)
    {
      uint64_t addr_int;
      struct situ *s = (struct situ*)n->next_list;
      //write(fdn, s, sizeof(struct situ));
      for (i = 0; i < sizeof(struct situ)/sizeof(void *); i++)
	{
	  if (n->next_list[i])
	    addr_int = 1;
	  else
	    addr_int = 0;
	  write(fdn, &addr_int, sizeof(addr_int));
	}
      for (i = 0; i < ACTS; i++)
	{
	  if (s->regs[i] != NULL)
	    write(fdn, s->regs[i], sizeof(s->regs[i][0]) * n->len);
	  /* if (s->d_regs[i] != NULL) */
	  /*   write(fdn, s->d_regs[i], sizeof(s->d_regs[i][0]) * n->len); */
	  if (s->avg_odds[i] != NULL)
	    write(fdn, s->avg_odds[i], sizeof(s->avg_odds[i][0]) * n->len);
	  

	  /* if (i < ACTS-1) */
	  /*   { */
	  /*     /\* if (s->hand_odds[i] != NULL) *\/ */
	  /*     /\* 	write(fdn, s->hand_odds[i], sizeof(s->hand_odds[i][0]) * n->len); *\/ */
	      
	  /*     if (s->ev[i] != NULL) */
	  /* 	write(fdn, s->ev[i], sizeof(s->ev[i][0]) * n->len); */
	  /*     if (s->ev_count[i] != NULL) */
	  /* 	write(fdn, s->ev_count[i], sizeof(s->ev_count[i][0]) * n->len); */


	  /*   } */

	}
      /* if (s->visits != NULL) */
      /* 	write(fdn, s->visits, sizeof(s->visits[0]) * n->len); */
      
    }
  else
    {
      uint64_t next_offset, cur_pos;
      next_offset = lseek(fdn, 0, SEEK_CUR);
      write(fdn, n->next_list, sizeof(uint64_t)*n->len);
      for (i = 0; i < n->len;i++)
	{
	  cur_pos = lseek(fdn, 0, SEEK_CUR);
	  lseek(fdn, next_offset, SEEK_SET);
	  write(fdn, &cur_pos, sizeof(cur_pos));
	  next_offset = lseek(fdn, 0, SEEK_CUR);
	  lseek(fdn, cur_pos, SEEK_SET);
	  save_plrmodel_tree(fdn, n->next_list[i]);
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

struct plrmodel_node *load_plrmodel_tree(struct gameinfo *info, int fdn, struct unique_root *u)
{
  struct plrmodel_node *n, *next_n;
  int i, public;
  n = calloc(1, sizeof(struct plrmodel_node));
  
  assert(sizeof(i) == sizeof(n->t->id));

  read_exactly(fdn,&i, sizeof(n->t->id));
  assert(i >= 0 && i <= 13);
  read_exactly(fdn,&public, sizeof(n->t->public));
  assert(public == 0 || public == 1);
  n->t = &info->type_types[public][i];
  n->types_bmap = alloc_bitfield(n->t->n_types);
  read_exactly(fdn, n->types_bmap, bitfield_bytesize(n->t->n_types));
  //n->valid_bmap = alloc_bitfield(n->t->n_types);
  n->type_mapping = NULL;
  //n->type_mapping_alt = NULL;
  
  n->len = bitcount_before_bitmap(n->types_bmap, n->t->n_types);
  n->root = u;
  

  if (n->t->public == 0)
    {
      struct situ *s = malloc(sizeof(struct situ));
      read_exactly(fdn, s, sizeof(struct situ));

      for (i = 0; i < ACTS; i++)
	{
	  if (s->regs[i] != NULL)
	    {
	      s->regs[i] = malloc(sizeof(s->regs[i][0]) * n->len);
	      read_exactly(fdn, s->regs[i], sizeof(s->regs[i][0]) * n->len);
	    }
	  /* if (s->d_regs[i] != NULL) */
	  /*   { */
	  /*     s->d_regs[i] = malloc(sizeof(s->d_regs[i][0]) * n->len); */
	  /*     read_exactly(fdn, s->d_regs[i], sizeof(s->d_regs[i][0]) * n->len); */
	  /*   } */
	  if (s->avg_odds[i] != NULL)
	    {
	      s->avg_odds[i] = malloc(sizeof(s->avg_odds[i][0]) * n->len);
	      read_exactly(fdn, s->avg_odds[i], sizeof(s->avg_odds[i][0]) * n->len);
	    }
	  /* if (i < ACTS-1) */
	  /*   { */
	  /*     /\* if (s->hand_odds[i] != NULL) *\/ */
	  /*     /\* 	{ *\/ */
	  /*     /\* 	   s->hand_odds[i] = malloc(sizeof(s->hand_odds[i][0]) * n->len); *\/ */
	  /*     /\* 	   read_exactly(fdn, s->hand_odds[i], sizeof(s->hand_odds[i][0]) * n->len); *\/ */
	  /*     /\* 	} *\/ */
	      
	  /*     if (s->ev[i] != NULL) */
	  /* 	{ */
	  /* 	  s->ev[i] = malloc(sizeof(s->ev[i][0]) * n->len); */
	  /* 	  read_exactly(fdn, s->ev[i], sizeof(s->ev[i][0]) * n->len); */
	  /* 	} */
	  /*     if (s->ev_count[i] != NULL) */
	  /* 	{ */
	  /* 	  s->ev_count[i] = malloc(sizeof(s->ev_count[i][0]) * n->len); */
	  /* 	  read_exactly(fdn, s->ev_count[i], sizeof(s->ev_count[i][0]) * n->len); */
	  /* 	} */

	  /*   } */
	}
      /* if (s->visits != NULL) */
      /* 	{ */
      /* 	  s->visits = malloc(sizeof(s->visits[0]) * n->len); */
      /* 	  read_exactly(fdn, s->visits, sizeof(s->visits[0]) * n->len); */
      /* 	} */
      
      //s->parent = n;
      //n->hand_count = 1;
      n->next_list = (void**)s;
    }
  else
    {
      uint64_t *tmpbuf = malloc(sizeof(uint64_t)*n->len);
      read_exactly(fdn, tmpbuf, sizeof(uint64_t)*n->len);
      
      //lseek(fdn, sizeof(uint64_t)*n->len, SEEK_CUR);
      n->next_list = malloc(sizeof(void*)*n->len);
      //      n->slot_structure = malloc(sizeof(struct slot)*n->len);
      //n->types_bmap = alloc_bitfield(n->max_len);
      //read_exactly(fdn,n->types_bmap, bitfield_bytesize(n->max_len));
      //n->hand_count = 0;
      for (i = 0; i < n->len;i++)
	{
	  //assert(tmpbuf[i] == lseek(fdn, 0, SEEK_CUR));
	  next_n = load_plrmodel_tree(info, fdn, u);
	  next_n->prev = n;
	  next_n->slot_in_prev = i;
	  n->next_list[i] = next_n;
	  //n->hand_count += next_n->hand_count;
	}
      free(tmpbuf);
    }
  return n;
}

void load_plrmodel_regs(struct plrmodel_node *n, int fdn)
{
  int i;

  if (n->t->public == 0)
    {
      struct situ *s = NULL;
      if (n->next_list == NULL)
	n->next_list = calloc(1,sizeof(struct situ));
      s = (struct situ *)n->next_list;

      for (i = 0; i < ACTS; i++)
	{
	  if (s->regs[i] == NULL)
	    s->regs[i] = malloc(sizeof(s->regs[i][0]) * n->len);

	  read_exactly(fdn, s->regs[i], sizeof(s->regs[i][0]) * n->len);
	}
    }
  else
    {
      for (i = 0; i < n->len;i++)
	{
	  load_plrmodel_regs(n->next_list[i], fdn);
	}
    }
}

void load_plrmodel_avg_odds(struct plrmodel_node *n, int fdn)
{
  int i;

  if (n->t->public == 0)
    {
      struct situ *s = NULL;
      if (n->next_list == NULL)
	n->next_list = calloc(1,sizeof(struct situ));
      s = (struct situ *)n->next_list;

      for (i = 0; i < ACTS; i++)
	{
	  if (s->avg_odds[i] == NULL)
	    s->avg_odds[i] = malloc(sizeof(s->avg_odds[i][0]) * n->len);

	  read_exactly(fdn, s->avg_odds[i], sizeof(s->avg_odds[i][0]) * n->len);
	}
    }
  else
    {
      for (i = 0; i < n->len;i++)
	{
	  load_plrmodel_avg_odds(n->next_list[i], fdn);
	}
    }
}

void load_plrmodel_byte_odds(struct plrmodel_node *n, int fdn)
{
  int i;

  if (n->t->public == 0)
    {
      struct situ *s = NULL;
      if (n->next_list == NULL)
	n->next_list = calloc(1,sizeof(struct situ));
      s = (struct situ *)n->next_list;
      if (s->byte_odds == NULL)
	s->byte_odds = malloc(sizeof(s->byte_odds[0]) * n->len*2);

      read_exactly(fdn, s->byte_odds, sizeof(s->byte_odds[0]) * n->len*2);
	
    }
  else
    {
      for (i = 0; i < n->len;i++)
	{
	  load_plrmodel_byte_odds(n->next_list[i], fdn);
	}
    }
}


void set_avg_odds_from_mmap(struct plrmodel_node *n, double *m)
{
  int i;

  if (n->t->public == 0)
    {
      struct situ *s = NULL;
      if (n->next_list == NULL)
	n->next_list = calloc(1,sizeof(struct situ));
      n->data_format = 1;
      s = (struct situ *)n->next_list;
      s->avg_odds[0] = &m[n->data_index * ACTS];
      s->avg_odds[1] = &m[n->data_index * ACTS + n->len];
      s->avg_odds[2] = &m[n->data_index * ACTS + n->len*2];
    }
  else
    {
      for (i = 0; i < n->len;i++)
	{
	  set_avg_odds_from_mmap(n->next_list[i], m);
	}
    }
}

void set_regs_from_mmap(struct plrmodel_node *n, double *m)
{
  int i;

  if (n->t->public == 0)
    {
      struct situ *s = NULL;
      if (n->next_list == NULL)
	n->next_list = calloc(1,sizeof(struct situ));
      n->data_format = 1;
      s = (struct situ *)n->next_list;
      s->regs[0] = &m[n->data_index * ACTS];
      s->regs[1] = &m[n->data_index * ACTS + n->len];
      s->regs[2] = &m[n->data_index * ACTS + n->len*2];
    }
  else
    {
      for (i = 0; i < n->len;i++)
	{
	  set_regs_from_mmap(n->next_list[i], m);
	}
    }
}

void set_byte_odds_from_mmap(struct plrmodel_node *n, uint8_t *m)
{
  int i;

  if (n->t->public == 0)
    {
      struct situ *s = NULL;
      if (n->next_list == NULL)
	n->next_list = calloc(1,sizeof(struct situ));
      n->data_format = 1;
      s = (struct situ *)n->next_list;
      s->byte_odds = &m[n->data_index * 2];
    }
  else
    {
      for (i = 0; i < n->len;i++)
	{
	  set_byte_odds_from_mmap(n->next_list[i], m);
	}
    }
}


struct plrmodel_node *load_plrmodel_struct(struct gameinfo *info, int fdn, struct unique_root *u)
{
  struct plrmodel_node *n, *next_n;
  int i, public;
  n = calloc(1, sizeof(struct plrmodel_node));
  
  assert(sizeof(i) == sizeof(n->t->id));

  read_exactly(fdn,&i, sizeof(n->t->id));
  assert(i >= 0 && i <= 13);
  read_exactly(fdn,&public, sizeof(n->t->public));
  assert(public == 0 || public == 1);
  read_exactly(fdn,&n->data_index, sizeof(n->data_index));
  n->t = &info->type_types[public][i];
  n->types_bmap = alloc_bitfield(n->t->n_types);
  read_exactly(fdn, n->types_bmap, bitfield_bytesize(n->t->n_types));
  //n->valid_bmap = alloc_bitfield(n->t->n_types);
  n->type_mapping = NULL;
  //n->type_mapping_alt = NULL;
  
  n->len = bitcount_before_bitmap(n->types_bmap, n->t->n_types);
  n->root = u;
  

  if (n->t->public)
    {
      uint64_t *tmpbuf = malloc(sizeof(uint64_t)*n->len);
      read_exactly(fdn, tmpbuf, sizeof(uint64_t)*n->len);
      
      n->next_list = malloc(sizeof(void*)*n->len);
      for (i = 0; i < n->len;i++)
	{
	  next_n = load_plrmodel_struct(info, fdn, u);
	  next_n->prev = n;
	  next_n->slot_in_prev = i;
	  n->next_list[i] = next_n;
	}
      free(tmpbuf);
    }
  return n;
}


void free_situ(void **s)
{
  int i;
  for (i = 0; i < sizeof(struct situ)/sizeof(void*); i++)
    if (s[i] != NULL)
      free(s[i]);
  free(s);
}

void free_plrmodel_tree(struct plrmodel_node *n)
{
  int i;

  if (n->types_bmap != NULL)
    free(n->types_bmap);
  /* if (n->valid_bmap != NULL) */
  /*   free(n->valid_bmap); */
  if (n->type_mapping != NULL)
    free(n->type_mapping);
  /* if (n->type_mapping_alt != NULL) */
  /*   free(n->type_mapping_alt); */
    
  if (n->t->public == 0)
    {
      if (n->data_format == 0)
	free_situ(n->next_list);
      else
	if (n->next_list != NULL)
	  free(n->next_list);
      n->next_list = NULL;
    }
  else
    {
      for (i = 0; i < n->len;i++)
	{
	  free_plrmodel_tree(n->next_list[i]);
	}
      
      if (n->next_list != NULL)
	free(n->next_list);
    }
  free(n);
}





  






int is_duplicate(int *d, int len)
{
  int i,j;

  for (i = 0; i < len; i++)
      for (j = i+1; j < len; j++)
	if (d[i] == d[j])
	  return 1;
  return 0;
}


	  
/* void pm_reset_ev(struct plrmodel_node *n) */
/* { */
/*   int i; */
/*   struct situ *s = (struct situ*)n->next_list; */


/*   for (i = 0; i < ACTS-1;i++) */
/*     if (s->ev[i] != NULL) */
/*       memset(s->ev[i], 0, sizeof(s->ev[i][0])*n->len); */
/* } */

void pm_reset_regs(struct plrmodel_node *n)
{
  int i;
  struct situ *s = (struct situ*)n->next_list;

  for (i = 0; i < ACTS;i++)
    if (s->regs[i] != NULL)
      memset(s->regs[i], 0, sizeof(s->regs[i][0])*n->len);
}

void pm_reset_d_regs(struct plrmodel_node *n)
{
  int i;
  struct situ *s = (struct situ*)n->next_list;

  for (i = 0; i < ACTS;i++)
    if (s->regs[i] != NULL)
      memset(s->regs[i], 0, sizeof(s->regs[i][0])*n->len);
}

void pm_reset_avg_odds(struct plrmodel_node *n)
{
  int i;
  struct situ *s = (struct situ*)n->next_list;

  for (i = 0; i < ACTS;i++)
    if (s->avg_odds[i] != NULL)
      memset(s->avg_odds[i], 0, sizeof(s->avg_odds[i][0])*n->len);
}

/* void pm_reset_visits(struct plrmodel_node *n) */
/* { */

/*   struct situ *s = (struct situ*)n->next_list; */

/*   if (s->visits != NULL) */
/*     memset(s->visits, 0, sizeof(s->visits[0])*n->len); */
/* } */


double tot_change;
double avg_sd_ev[2];
unsigned int avg_sd_ev_c[2];

/* void update_regrets(struct situ *s, int visit_limit) */
/* { */
/*   int hand_i; */
/*   double avg_ev; */
/*   for (hand_i = 0; hand_i < s->parent->len; hand_i++) */
/*     { */
/*       if (s->hd[hand_i].visits > visit_limit) */
/* 	{ */
/* 	  avg_ev = s->hand_odds[hand_i*3] * s->hd[hand_i].ev[0] +s->hand_odds[hand_i*3+1] * s->hd[hand_i].ev[1];  */
/* 	  s->hd[hand_i].regs[0] += s->hd[hand_i].ev[0] - avg_ev; */
/* 	  s->hd[hand_i].regs[1] += s->hd[hand_i].ev[1] - avg_ev; */
/* 	  s->hd[hand_i].regs[2] += 0 - avg_ev; */
/* 	  s->hd[hand_i].visits = 0; */
/* 	} */
/*     } */
/* }	   */



