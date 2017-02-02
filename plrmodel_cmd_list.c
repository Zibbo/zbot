#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "defs.h"
#include "plrmodel.h"
#include "plrmodel_cmd_list.h"

/* CREATING CMD LISTS */

void *cmdmem_alloc(struct mem *m, unsigned int units)
{
  void *retval;
  while (m->mem_size - m->i < units)
    {
      m->mem_size += 1024;
      m->data = realloc(m->data, m->mem_size*m->unit_size);
    }
  retval = m->data + m->i * m->unit_size;
  m->i += units;
  return retval;
  //  memcpy((m->data + m->i * m->unit_size), data, units*m->unit_size);
}


void calc_rcf_odds_for_one_path_node(struct cmd_lists *cmd, struct memory_pointers *memp, struct path *p, int index, int len)
{
  double path_odds = 1.0;
  struct cmd_path_node_odds_data *pairs, *final_data;
  struct cmd_path_node_odds_header *final_header;
  //  struct path *tmp_path;
  int x, i, j, act, path_idx, combine_count, l;
  struct situ *s;

  

  pairs = calloc(1, sizeof(struct cmd_path_node_odds_data)*len);
  
  
  x = 0;
  while (p != NULL)
    {
      path_odds = 1.0;
      s = p->items[index].s;
      for (i = 0; i < index; i++)
	{
	  act = p->items[i].act;
	  path_idx = p->items[i].path_idx;
	  path_odds *= p->items[i].s->path_odds[act][path_idx];
	}
      act = p->items[i].act;
      path_idx = p->items[i].path_idx;
      pairs[x].act_odds_off = s->gdo_situ*3+act;
      pairs[x].struct_odds = path_odds;
      x++;
      p = p->next;
    }
  combine_count = 0;
  for (i = 0; i < len; i++)
    {
      if (pairs[i].struct_odds == 0)
	continue;
      for (j = i+1; j < len; j++)
	{
	  if (pairs[i].act_odds_off == pairs[j].act_odds_off && pairs[j].struct_odds != 0)
	    {
	      pairs[i].struct_odds += pairs[j].struct_odds;
	      pairs[j].struct_odds = 0;
	      combine_count++;
	    }
	}
    }

  //retval = calloc(1,sizeof(unsigned int)*2 + sizeof(struct cmd_path_node_odds_data)*(len-combine_count));
  
  //  final_pairs = (struct cmd_path_node_odds_data*)&retval[2];
  //final_pairs = cmd->pno.d[cmd->pno.doff];
  //final_header = cmd->pno.h[cmd->pno.hoff];
  final_header = cmdmem_alloc(&cmd->pno.h, 1);
  final_header->len = len-combine_count;
  final_header->data_off = cmd->pno.d.i;
  final_header->target_off = memp->tmp_data_f;
  //printf("path_node_odds target %i", memp->tmp_data_f);
  memp->tmp_data_f++;

  final_data = cmdmem_alloc(&cmd->pno.d, len-combine_count);
  l = 0;

  for (i = 0; i < len; i++)
    {
      if (pairs[i].struct_odds == 0)
	continue;
      final_data[l] = pairs[i];
      //printf("p: %i %f, ", final_data[i].act_odds_off, final_data[i].struct_odds);
      l++;
    }
  //printf("\n");
  if (l != len-combine_count)
    printf("RIKSRAKS %i %i %i\n", l, len, combine_count);

  free(pairs);
}
  
void calc_path_odds_from_node_odds(struct cmd_lists *cmd, struct memory_pointers *memp, unsigned int odds_start, int len, unsigned int first_node_act)
{
  
  struct cmd_path_odds_from_node_odds_header *h;

  h = (struct cmd_path_odds_from_node_odds_header *)cmdmem_alloc(&cmd->pofno.h, 1);
  h->len = len;
  h->data_off = odds_start;
  h->target = memp->gdo_paths;
  h->first_node_act = first_node_act;
  memp->gdo_paths++;
}

void calc_path_weight_and_potsize(struct cmd_lists *cmd, struct path *p, double structure_odds, unsigned int p_gdo, double action_cost)
{

  struct cmd_path_weight_and_potsize *h;


  if (p->items[p->len-1].s == NULL)
    return;

  h = cmdmem_alloc(&cmd->pwap, 1);
  h->start_situ_off = p->items[0].s->gdo_situ;
  h->end_situ_off = p->items[p->len-1].s->gdo_situ;
  h->path_off = p_gdo;
  h->structure_odds = structure_odds;
  h->action_cost = action_cost;
}

void calc_hand_hw(struct cmd_lists *cmd,  struct situ *src, struct situ *dest, double *hand_odds, unsigned int p_gdo, double structure_odds)
{
  int i, j;
  struct cmd_hand_hw *h;

  for (i = 0; i < src->n_hands; i++)
    {
      for (j = 0; j < dest->n_hands; j++)
	{
	  if (hand_odds[i*dest->n_hands+j] == 0)
	    continue;
	  h = cmdmem_alloc(&cmd->hhw, 1);
	  h->src_off = src->gdo_hands+i;
	  h->dest_off = dest->gdo_hands+j;
	  if (dest->parent->type == 200)
	    h->dest_off = h->dest_off|0x80000000;
	  h->path_off = p_gdo;
	  h->odds = hand_odds[i*dest->n_hands+j] * structure_odds;
	  //if (h->odds != 1.0)
	  //  printf("ALAMOLO %f\n", h->odds);
	}
    }
}

void calc_ev(struct cmd_lists *cmd, struct situ *prev, struct situ *cur, double *hand_odds, unsigned int p_gdo, double structure_odds, double own_action_cost, double other_action_cost)
{
  int i, j;
  struct cmd_calc_ev *h;
  for (i = 0; i < prev->n_hands; i++)
    {
      for (j = 0; j < cur->n_hands; j++)
	{
	  if (hand_odds[i*cur->n_hands+j] == 0)
	    continue;
	  if (cur->parent->type == 200)
	    h = cmdmem_alloc(&cmd->cev_sd, 1);
	  else
	    h = cmdmem_alloc(&cmd->cev, 1);
	  h->prev_situ_off = prev->gdo_situ;
	  h->cur_situ_off = cur->gdo_situ;
	  h->prev_off = prev->gdo_hands+i;
	  h->cur_off = cur->gdo_hands+j;
	  h->path_off = p_gdo;
	  h->own_action_cost = own_action_cost;
	  h->other_action_cost = other_action_cost;
	  h->odds = hand_odds[i*cur->n_hands+j] * structure_odds;
	}
    }
}

void calc_ev_fold(struct cmd_lists *cmd, struct situ *prev, struct situ *cur, unsigned int p_gdo, double structure_odds, double own_action_cost, double other_action_cost)
{
  struct cmd_calc_ev_fold *h;
  //if (other_action_cost != 0)
  // printf("b tahan %f\n",other_action_cost);
  h = cmdmem_alloc(&cmd->cev_fold, 1);
  h->cur_situ_off = cur->gdo_situ;
  h->prev_situ_off = prev->gdo_situ;
  h->prev_hands_off = prev->gdo_hands;
  h->path_off = p_gdo;
  h->own_action_cost = own_action_cost;
  h->other_action_cost = other_action_cost;
  h->odds = structure_odds;
  h->prev_n_hands = prev->n_hands;
}
  

void calc_sd_value(struct gameinfo *info, struct cmd_lists *cmd, struct plrmodel_node *n)
{
  struct cmd_calc_sd_value_header *h;
  struct cmd_calc_sd_value_data_header *dh;
  struct cmd_calc_sd_value_data *d;
  struct situ  *s;
  int i, j;
  unsigned int data_header_off;

  data_header_off = cmd->csv.dh.i;
  dh = cmdmem_alloc(&cmd->csv.dh, 1);
  dh->n_plr = n->root->n_plr;
  dh->sd_data_off = cmd->csv.d.i;

  for (i = 0; i < dh->n_plr; i++)
    {
      s = n->next_list[i];
      d = cmdmem_alloc(&cmd->csv.d, 1);
      d->hands_off = s->gdo_hands;
      d->n_hands = s->n_hands;
      d->situ_off = s->gdo_situ;
      for (j = 0; j < s->n_hands; j++)
	{
	  h = cmdmem_alloc(&cmd->csv.h, 1);
	  h->target_hand_off = d->hands_off+j;
	  h->data_header_off = data_header_off;
	}
    }
}

/*   h->hand_data_off = cmd->csv.hd.i; */
/*   h->slot_data_off = cmd->csv.sd.i; */
  
/*   for (i = 0; i < h->n_plr; i++) */
/*     { */
/*       s = n->next_list[i]; */

/*       hd = cmdmem_alloc(&cmd->csv.hd, 1); */
/*       hd->hands_off = s->gdo_hands; */
/*       hd->n_hands = s->n_hands; */

/*       sd = cmdmem_alloc(&cmd->csv.sd, s->n_hands); */
/*       for (j = 0; j < s->n_hands; j++) */
/* 	{ */
/* 	  sd[j] = get_slot_i_from_slot(&(s->hand_slots[j])); */
/* 	} */
/*     } */

/*   //  for (i = 0; i < */
/*   for (situ_for_i = 0; situ_for_i < n->len; situ_for_i++) */
/*     { */
/*       situ_for = n->next_list[situ_for_i]; */
      

/*       for (situ_against_i = 0; situ_against_i < n->len; situ_against_i++) */
/*   	{ */
/*   	  if (situ_for_i == situ_against_i) */
/*   	    continue; */

/*   	  situ_against = n->next_list[situ_against_i]; */
	  
/*   	  for (own_hand_i = 0; i < situ_for->n_hands;own_hand_i++) */
/*   	    { */
/*   	      slot_for = get_slot_i_from_slot(situ_for->slots[own_hand_i]); */
/*   	      h = cmdmem_alloc(&cmd->csv.h, 1); */
/* 	      h->n_plr = n->len; */
/* 	      h->target_hand_off = situ_for->gdo_hands+own_hand_i; */
/* 	      h->hand_data_off = cmd->csv.hd.i; */
/* 	      h->slot_data_off = cmd->csv.sd.i; */
	      

/* 	      h->victory_odds_off = cmd->csv.d.i; */
/*   	      d = cmdmem_alloc(&cmd->csv.d, situ_against->n_hands); */
/*   	      h->opp_hands_off = situ_against->gdo_hands; */
/*   	      h->own_hands_off = situ_for->gdo_hands; */
/*   	      h->opp_n_hands = situ_against->n_hands; */
/*   	      victory_odds = d->data; */
/*   	      for (opp_hand_i = 0; i < situ_against->n_hands;opp_hand_i++) */
/*   		{ */
/*   		  slot_against = get_slot_i_from_slot(situ_for->slots[opp_hand_i]); */
		  
/*   		  victory_odds[i] = info->hand_wtl_odds[slot_for*info->n_types[GS_R]*2+slot_against]; */
/*   		} */
	      
/*   	    } */
/*   	} */
/*     } */
/* } */


void calc_situ_odds(struct cmd_lists *cmd, struct situ *s)
{
  
  struct cmd_calc_situ_odds *h;
  h = cmdmem_alloc(&cmd->cso, 1);
  h->situ_off = s->gdo_situ;
  h->hands_off = s->gdo_hands;
  h->n_hands = s->n_hands;
}

/* EXECUTING CMD LISTS */

void exec_path_node_odds(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga)
{
  struct cmd_path_node_odds_header *h, *cur_h;
  struct cmd_path_node_odds_data *d, *cur_d;
  int i,j;
  double tot_so, tot_ao;

  h = cmd->pno.h.data;
  d = cmd->pno.d.data;
  
  for (i = 0; i < cmd->pno.h.i; i++)
    {
      cur_h = &h[i];
      
      tot_so = 0;
      tot_ao = 0;
      for (j = cur_h->data_off; j < cur_h->data_off + cur_h->len; j++)
	{
	  cur_d = &d[j];
	  
	  tot_ao += ga->situ_odds[cur_d->act_odds_off]*cur_d->struct_odds;
	  tot_so += cur_d->struct_odds;
	}
      ga->tmp_data_f[cur_h->target_off] = tot_ao/tot_so;
    }
}

void exec_path_odds_from_node_odds(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga)
{
  struct cmd_path_odds_from_node_odds_header *h;
  int i,j;
  double path_odds;

  h = (struct cmd_path_odds_from_node_odds_header *)cmd->pofno.h.data;

  for(i = 0; i < cmd->pofno.h.i; i++)
    {
      path_odds = 1.0;
      
      for (j = 0; j < h[i].len; j++)
	{
	  path_odds *= ga->tmp_data_f[h[i].data_off + j];
	}
      ga->path_odds[h[i].target] = path_odds;
      ga->path_first_act[h[i].target] = h[i].first_node_act;
    }
}

void exec_path_weight_and_potsize(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga)
{
  
  struct cmd_path_weight_and_potsize *h;
  unsigned int s_off, e_off, p_off;
  double so, ac, cur_pw, self_path_odds, other_path_odds;
  int i;
  
  h = cmd->pwap.data;

  for(i = 0; i < cmd->pwap.i; i++)
    {
      s_off = h[i].start_situ_off;
      e_off = h[i].end_situ_off;
      p_off = h[i].path_off;
      so = h[i].structure_odds;
      ac = h[i].action_cost;
      self_path_odds = ga->situ_odds[s_off*3+ga->path_first_act[p_off]]*so;
      other_path_odds = ga->path_odds[p_off];
      cur_pw = other_path_odds * self_path_odds;
      //      cur_pw = ga->path_odds[p_off]*ga->situ_odds[s_off*3+ga->path_first_act[p_off]]*so; // path weight
      ga->situ_weight_onestep[e_off] += cur_pw;
      ga->self_situ_weight[e_off] += self_path_odds*ga->self_situ_weight[s_off]; 
      ga->other_situ_weight[e_off] += other_path_odds*ga->other_situ_weight[s_off]; 
      if (cur_pw * ga->situ_weight[s_off] > 0)
	{
	  ga->situ_weight[e_off] += cur_pw*ga->situ_weight[s_off] ;
      //ga->potsize[s_off] /= ga->situ_weight[s_off];
	  ga->potsize[e_off] += (ga->potsize[s_off]/ ga->situ_weight[s_off] + ac)*cur_pw*ga->situ_weight[s_off];
	}
      assert(ga->potsize[e_off] >= 0 && ga->potsize[e_off] <= 48);
    }
}
	
void exec_correct_zero_potsize(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga)
{
  
  struct cmd_path_weight_and_potsize *h;
  unsigned int s_off, e_off, p_off;
  double so, ac;
  int i;
  
  h = cmd->pwap.data;

  for(i = 0; i < cmd->pwap.i; i++)
    {
      s_off = h[i].start_situ_off;
      e_off = h[i].end_situ_off;
      p_off = h[i].path_off;
      so = h[i].structure_odds;
      ac = h[i].action_cost;
      if (ga->situ_weight[e_off] > 0)
	continue;
      //cur_pw = ga->situ_weight[s_off]*ga->path_odds[p_off]*ga->situ_odds[s_off*3+ga->path_first_act[p_off]]; // path weight
      ga->situ_weight[e_off] -= 1.0;
      ga->potsize[e_off] += (ga->potsize[s_off]/ fabs(ga->situ_weight[s_off]) + ac);

    }
}

void exec_normalize_potsize(struct global_arrays *ga, int n_situs)
{
  int i;

  for (i = 0; i < n_situs;i++)
    if (ga->situ_weight[i] > 0)
      ga->potsize[i] /= ga->situ_weight[i];
    else
      {
	if (ga->situ_weight[i] == 0)
	  printf("FUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUCK\n");
	ga->potsize[i] /= fabs(ga->situ_weight[i]);
	ga->situ_weight[i] = 0;
      }
}

void exec_hand_hw(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga)
{
  double src_situ_weight, dest_situ_weight, src_situ_weight_onestep, dest_situ_weight_onestep, act_odds, path_odds;
  unsigned int dest_off, src_off, dest_situ_i, src_situ_i;
  struct cmd_hand_hw *h;
  double  *dest_hhw;
  unsigned int *dest_hts;
  int i;
  // unsigned int dest_off;

  h = cmd->hhw.data;

  for (i = 0; i < cmd->hhw.i; i++)
    {
      if (h[i].dest_off & 0x80000000)
	{
	  dest_hts = ga->sd_hand_to_situ;
	  dest_hhw = ga->sd_hand_hw;
	}
      else
	{
	  dest_hts = ga->hand_to_situ;
	  dest_hhw = ga->hand_hw;
	}
      dest_off = h[i].dest_off&0x7fffffff;
      src_off = h[i].src_off;
      dest_situ_i = dest_hts[dest_off];
      src_situ_i = ga->hand_to_situ[src_off];
      src_situ_weight = ga->situ_weight[src_situ_i];
      dest_situ_weight = ga->situ_weight[dest_situ_i];
      src_situ_weight_onestep = ga->situ_weight_onestep[src_situ_i];
      dest_situ_weight_onestep = ga->situ_weight_onestep[dest_situ_i];
      //if (h[i].dest_off == 0x80000000)
      //	printf("DESTI NOLLA 0000\n");

      if (ga->self_situ_weight[dest_situ_i] == 0 || ga->self_situ_weight[src_situ_i] == 0)
	continue;

      //      if (dest_situ_weight != 0 && src_situ_weight == 0)
      //	continue;
      //if (h[i].dest_off == 0x80000000)
      //	printf("DESTI NOLLA 1111\n");
      act_odds = ga->hand_odds[src_off*3+ga->path_first_act[h[i].path_off]];
      path_odds = ga->path_odds[h[i].path_off];
      if (ga->other_situ_weight[dest_situ_i] == 0)
	{
	  assert(ga->situ_weight[dest_situ_i] == 0);
	  
	  if (dest_situ_weight_onestep == 0)
	    {
	      //if (act_odds == 0)
	      //  act_odds = 1.0;
	      if (path_odds == 0)
		path_odds = 1.0;
	    }
	}
      path_odds *= h[i].odds;
      dest_hhw[dest_off] += ga->hand_hw[h[i].src_off] * act_odds * path_odds;
      /* if (0x80000000 & h[i].dest_off) //merkkibitti, että showdown node */
      /* 	{ */
      /* 	  //ga->sd_hand_hw[h[i].dest_off&0x7fffffff] += ga->hand_hw[h[i].src_off] * ga->path_odds[h[i].path_off] * ga->hand_odds[h[i].src_off*3+ga->path_first_act[h[i].path_off]]*h[i].odds; */
      /* 	  ga->sd_hand_hw[dest_off] += ga->hand_hw[h[i].src_off] * act_odds * path_odds; */
      /* 	  if (dest_off == 0) */
      /* 	    printf("DESTI NOLLA\n"); */
	  
      /* 	} */
      /* else */
      /* 	{ */
      /* 	  ga->hand_hw[dest_off] += ga->hand_hw[h[i].src_off] * act_odds * path_odds; */
      /* 	} */
      //if (ga->hand_hw[h[i].src_off] == 0)
      //	printf("TORTTTTOTOROO\n");
    }
}

void exec_ev_fold(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga)
{
  struct cmd_calc_ev_fold *h;
  int i,j;
  //  double fold_value;
  double cur_victory_odds, new_victory_odds, new_oadd;
  h = cmd->cev_fold.data;

  for (i = 0; i < cmd->cev_fold.i; i++)
    {
      cur_victory_odds = ga->situ_odds[h[i].cur_situ_off*3+2];
      //cur_oadd = ga->hand_odds[h[i].cur_off*3]*ga->other_add[h[i].cur_off*2] + ga->hand_odds[h[i].cur_off*3+1]*ga->other_add[h[i].cur_off*2+1];
      //cur_sadd = ga->hand_odds[h[i].cur_off*3]*ga->self_add[h[i].cur_off*2] + ga->hand_odds[h[i].cur_off*3+1]*ga->self_add[h[i].cur_off*2+1];
      new_oadd = (h[i].other_action_cost * cur_victory_odds)*ga->path_odds[h[i].path_off]*h[i].odds;
      //if (new_oadd != 0)
      //printf("OOOO OK??? %f %f %f %f %f\n", new_oadd, h[i].other_action_cost, cur_victory_odds,ga->path_odds[h[i].path_off],h[i].odds);
      //new_sadd = (h[i].own_action_cost * (1.0-cur_victory_odds) + cur_sadd)*ga->path_odds[h[i].path_off]*h[i].odds;
      new_victory_odds = cur_victory_odds * ga->path_odds[h[i].path_off]*h[i].odds;
      for (j = h[i].prev_hands_off; j < h[i].prev_hands_off + h[i].prev_n_hands; j++)
	{
	  ga->victory_odds[j*2+ga->path_first_act[h[i].path_off]] += new_victory_odds;
	  ga->other_add[j*2+ga->path_first_act[h[i].path_off]] += new_oadd;
	}
      //ga->self_add[h[i].prev_off*2+ga->path_first_act[h[i].path_off]] += new_sadd;

      
      //      fold_value = (ga->potsize[h[i].prev_situ_off]+h[i].other_action_cost+h[i].own_action_cost) * ga->situ_odds[h[i].cur_situ_off*3+2] * ga->path_odds[h[i].path_off] - h[i].own_action_cost*ga->path_odds[h[i].path_off];
      //fold_value = ((ga->potsize[h[i].prev_situ_off]+h[i].other_action_cost+h[i].own_action_cost) * ga->situ_odds[h[i].cur_situ_off*3+2] - h[i].own_action_cost )*ga->path_odds[h[i].path_off];
      //      fold_value = (ga->potsize[h[i].prev_situ_off]+h[i].other_action_cost) * ga->situ_odds[h[i].cur_situ_off*3+2]*ga->path_odds[h[i].path_off]*h[i].odds; // oac always 0?
      /* fold_value = ga->situ_odds[h[i].cur_situ_off*3+2]*((ga->potsize[h[i].cur_situ_off] + h[i].own_action_cost+ h[i].other_action_cost)/ga->potsize[h[i].cur_situ_off])*ga->path_odds[h[i].path_off]*h[i].odds;  */
      
      /* for (j = h[i].prev_hands_off; j < h[i].prev_hands_off + h[i].prev_n_hands; j++) */
      /* 	{ */
      /* 	  ga->hand_evs[j*2+ga->path_first_act[h[i].path_off]] += fold_value; */
      /* 	} */
      
    }
}

void exec_ev(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga)
{
  struct cmd_calc_ev *h;
  int i;
  //double  cur_ev, prev_ev, action_cost_diff;
  double cur_victory_odds, new_victory_odds, new_oadd, new_sadd, cur_oadd, cur_sadd;
  h = cmd->cev.data;

  for (i = cmd->cev.i-1; i >= 0; i--)
    {
      
      //if (h[i].prev_situ_off == 2)
      //	printf("JO\n");
      cur_victory_odds = ga->hand_odds[h[i].cur_off*3]*ga->victory_odds[h[i].cur_off*2] + ga->hand_odds[h[i].cur_off*3+1]*ga->victory_odds[h[i].cur_off*2+1];
      cur_oadd = ga->hand_odds[h[i].cur_off*3]*ga->other_add[h[i].cur_off*2] + ga->hand_odds[h[i].cur_off*3+1]*ga->other_add[h[i].cur_off*2+1];
      cur_sadd = ga->hand_odds[h[i].cur_off*3]*ga->self_add[h[i].cur_off*2] + ga->hand_odds[h[i].cur_off*3+1]*ga->self_add[h[i].cur_off*2+1];
      new_oadd = (h[i].other_action_cost * cur_victory_odds + cur_oadd)*ga->path_odds[h[i].path_off]*h[i].odds;
      new_sadd = (h[i].own_action_cost * (1.0-cur_victory_odds) + cur_sadd)*ga->path_odds[h[i].path_off]*h[i].odds;
      new_victory_odds = cur_victory_odds * ga->path_odds[h[i].path_off]*h[i].odds;
      ga->victory_odds[h[i].prev_off*2+ga->path_first_act[h[i].path_off]] += new_victory_odds;
      ga->other_add[h[i].prev_off*2+ga->path_first_act[h[i].path_off]] += new_oadd;
      ga->self_add[h[i].prev_off*2+ga->path_first_act[h[i].path_off]] += new_sadd;
      /*
      //      cur_ev = (ga->hand_evs[h[i].cur_off*2]*ga->hand_odds[h[i].cur_off*3] + ga->hand_evs[h[i].cur_off*2+1]*ga->hand_odds[h[i].cur_off*3+1])*((ga->potsize[h[i].prev_situ_off] + h[i].other_action_cost + h[i].own_action_cost)/ga->potsize[h[i].cur_situ_off]);
      cur_ev = (ga->hand_evs[h[i].cur_off*2]*ga->hand_odds[h[i].cur_off*3] + ga->hand_evs[h[i].cur_off*2+1]*ga->hand_odds[h[i].cur_off*3+1]);
      //action_cost_diff = ga->potsize[h[i].cur_situ_off] - (ga->potsize[h[i].prev_situ_off] + h[i].other_action_cost);
      
      //prev_ev = (cur_ev - action_cost_diff) * ga->path_odds[h[i].path_off]*h[i].odds;
      //prev_ev = (cur_ev - h[i].own_action_cost) * ga->path_odds[h[i].path_off]*h[i].odds;
      prev_ev = cur_ev *((ga->potsize[h[i].cur_situ_off] + h[i].own_action_cost+ h[i].other_action_cost)/ga->potsize[h[i].cur_situ_off])* ga->path_odds[h[i].path_off]*h[i].odds;
      //fold_value = (ga->potsize[h[i].prev_situ_off]+h[i].action_cost) * ga->situ_odds[h[i].cur_situ_off*3+2] * ga->path_odds[h[i].path_off];
      ga->hand_evs[h[i].prev_off*2+ga->path_first_act[h[i].path_off]] += prev_ev;
      */
    }
}

void exec_ev_sd(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga)
{
  struct cmd_calc_ev *h;
  int i;
  //double  cur_ev, prev_ev, action_cost_diff;
  double cur_victory_odds, new_victory_odds, new_oadd, new_sadd;
  h = cmd->cev_sd.data;

  for (i = 0; i < cmd->cev_sd.i; i++)
    {
      cur_victory_odds = ga->sd_victory_odds[h[i].cur_off];
      //cur_oadd = ga->hand_odds[h[i].cur_off*3]*ga->other_add[h[i].cur_off*2] + ga->hand_odds[h[i].cur_off*3+1]*ga->other_add[h[i].cur_off*2+1];
      //cur_sadd = ga->hand_odds[h[i].cur_off*3]*ga->self_add[h[i].cur_off*2] + ga->hand_odds[h[i].cur_off*3+1]*ga->self_add[h[i].cur_off*2+1];
      new_oadd = (h[i].other_action_cost * cur_victory_odds)*ga->path_odds[h[i].path_off]*h[i].odds;
      new_sadd = (h[i].own_action_cost * (1.0-cur_victory_odds))*ga->path_odds[h[i].path_off]*h[i].odds;
      new_victory_odds = cur_victory_odds * ga->path_odds[h[i].path_off]*h[i].odds;
      ga->victory_odds[h[i].prev_off*2+ga->path_first_act[h[i].path_off]] += new_victory_odds;
      ga->other_add[h[i].prev_off*2+ga->path_first_act[h[i].path_off]] += new_oadd;
      ga->self_add[h[i].prev_off*2+ga->path_first_act[h[i].path_off]] += new_sadd;

      /*
      //cur_ev = ga->sd_hand_evs[h[i].cur_off]*((ga->potsize[h[i].prev_situ_off] + h[i].other_action_cost + h[i].own_action_cost)/ga->potsize[h[i].cur_situ_off]);
      cur_ev = ga->sd_hand_evs[h[i].cur_off];
      //action_cost_diff = ga->potsize[h[i].cur_situ_off] - (ga->potsize[h[i].prev_situ_off] + h[i].other_action_cost);
      
      //prev_ev = (cur_ev - action_cost_diff) * ga->path_odds[h[i].path_off]*h[i].odds;

      //      prev_ev = (cur_ev - h[i].own_action_cost) * ga->path_odds[h[i].path_off]*h[i].odds;
      prev_ev = cur_ev * ((ga->potsize[h[i].cur_situ_off] + h[i].own_action_cost+ h[i].other_action_cost)/ga->potsize[h[i].cur_situ_off]) * ga->path_odds[h[i].path_off]*h[i].odds;
      //fold_value = (ga->potsize[h[i].prev_situ_off]+h[i].action_cost) * ga->situ_odds[h[i].cur_situ_off*3+2] * ga->path_odds[h[i].path_off];
      ga->hand_evs[h[i].prev_off*2+ga->path_first_act[h[i].path_off]] += prev_ev;
      */
    }
}


void exec_sd_value(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga)
{
  struct cmd_calc_sd_value_header *h, *cur_h;
  struct cmd_calc_sd_value_data_header *dh, *cur_dh;
  struct cmd_calc_sd_value_data *d, *cur_d;
  short int th_slot, opp_hand_slot;
  double *wtl_sum, *wtl_odds, *tmp_wtl, tot, th_hw;
  int hi, si, opp_i, hands_i, i;
  double tot_value[2];
  double tot_value_tot[2];

  h = cmd->csv.h.data;
  dh = cmd->csv.dh.data;
  d = cmd->csv.d.data;
  
  tot_value[0] = 0;
  tot_value[1] = 0;
  tot_value_tot[0] = 0;
  tot_value_tot[1] = 0;
      
  for (hi = 0; hi < cmd->csv.h.i; hi++)
    {
      cur_h = &h[hi];
      cur_dh = &dh[cur_h->data_header_off];
      cur_d = &d[cur_dh->sd_data_off];
      wtl_sum = calloc(1, sizeof(double)*3*(cur_dh->n_plr-1));
      
      th_slot = ga->sd_hand_slots[cur_h->target_hand_off];
      th_hw = ga->sd_hand_hw[cur_h->target_hand_off];
      
      opp_i = 0;
      for (si = 0; si < cur_dh->n_plr; si++)
	{
	  if (cur_h->target_hand_off >= cur_d[si].hands_off && cur_h->target_hand_off < cur_d[si].hands_off+cur_d[si].n_hands)
	    continue;
	  for (hands_i = cur_d[si].hands_off; hands_i < cur_d[si].hands_off + cur_d[si].n_hands; hands_i++)
	    {
	      opp_hand_slot = ga->sd_hand_slots[hands_i];
	      wtl_odds = &(info->hand_wtl_odds[ th_slot*info->n_types[GS_R]*2*3 + opp_hand_slot*3]);
	      wtl_sum[opp_i*3] += wtl_odds[0]*ga->sd_hand_hw[hands_i];
	      wtl_sum[opp_i*3+1] += wtl_odds[1]*ga->sd_hand_hw[hands_i];
	      wtl_sum[opp_i*3+2] += wtl_odds[2]*ga->sd_hand_hw[hands_i];
	    }
	  opp_i++;
	}

      for (i = 0; i < cur_dh->n_plr-1; i++)
	{
	  tmp_wtl = &wtl_sum[i*3];
	  tot = tmp_wtl[0] + tmp_wtl[1] + tmp_wtl[2];
	  if (tot != 0)
	    {
	      tmp_wtl[0] /= tot;
	      tmp_wtl[1] /= tot;
	      tmp_wtl[2] /= tot;
	    }
	}
      
      if (cur_dh->n_plr != 2)
	printf("RIKKIPOIKKI LIIKAA PELAAJIA %i\n", cur_dh->n_plr);
      //potsize = ga->potsize[cur_d->situ_off];
      //ga->sd_hand_evs[cur_h->target_hand_off] = wtl_sum[0] + wtl_sum[1]/2.0; //HAX ONLY 2HANDED
      ga->sd_victory_odds[cur_h->target_hand_off] = wtl_sum[0] + wtl_sum[1]/2.0; //HAX ONLY 2HANDED
      if (ga->situ_weight[ga->sd_hand_to_situ[cur_h->target_hand_off]] > 0)
	{
	  tot_value[0] += (wtl_sum[0] + wtl_sum[1]/2.0) * th_hw;
	  tot_value_tot[0] += th_hw;
	  tot_value[1] += (0.5 - (wtl_sum[0] + wtl_sum[1]/2.0)) * th_hw;
	}
      free(wtl_sum);
      
    }
  printf("sd value %f %f %f %f\n", tot_value[1], tot_value[0], tot_value_tot[0], tot_value[0]/tot_value_tot[0]);
}

void exec_regret(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga, unsigned int n_hands, int n_plr)
{
  int i,j, situ_i;
  double *ho, he[2], old_ho[3], *hr, avg_ev, tot, potsize, hw, *vodds, *oadd, *sadd, hr_tot = 0, tot_ho_diff = 0, hr_change, sw;
  
  for (i = 0 ; i < n_hands; i++)
    {
      hw = ga->hand_hw[i];
      if (hw == 0)
	continue;
      situ_i = ga->hand_to_situ[i];
      if (situ_i < n_plr)
	continue;
      sw = ga->other_situ_weight[situ_i];
      if (sw == 0)
	continue;
      vodds = &ga->victory_odds[i*2];
      oadd = &ga->other_add[i*2];
      sadd = &ga->self_add[i*2];
      ho = &ga->hand_odds[i*3];
      potsize = ga->potsize[ga->hand_to_situ[i]];
      //if (potsize == 46)
      //printf("break here %f\n", potsize);
      he[0] = (potsize*  vodds[0] + oadd[0] - sadd[0]);
      he[1] = (potsize* vodds[1] + oadd[1] - sadd[1]);
      //      avg_ev = (/*POTSIZE* */  vodds[0] + oadd[0] - sadd[0])*ho[0] + (/*POTSIZE **/ vodds[1] + oadd[1] - sadd[1])*ho[1];
      
      
      //ho = &ga->hand_odds[i*3];
      //he = &ga->hand_evs[i*2];

      /*      if (he[0] > 0)
	max_ev = he[0];
      if (he[1] > max_ev)
	max_ev = he[0];
      */
      hr = &ga->hand_regs[i*3];
      avg_ev = ho[0]*he[0] + ho[1]*he[1];


      //(he[0] - max_ev)
      //      (he[0] - ho[0]*he[0]); 
      hr_change = (he[0] - avg_ev)*sw;
      if (hr_change > 0 || hr[0] > 0)
	hr[0] += hr_change;
      hr_change = (he[1] - avg_ev)*sw;
      if (hr_change > 0 || hr[1] > 0)
	hr[1] += hr_change;
      hr_change = (0 - avg_ev)*sw;
      if (hr_change > 0 || hr[2] > 0)
	hr[2] += hr_change;
      //hr_tot += fabs(he[0] - avg_ev)/fabs(hr[0]) + fabs(he[1] - avg_ev)/fabs(hr[1]) + fabs(0 - avg_ev)/fabs(hr[2]);
     
      if (hr[0] <= 0 && hr[1] <= 0 && hr[2] <= 0)
	{
	  hr[0] = (he[0] - avg_ev);
	  hr[1] = (he[1] - avg_ev);
	  hr[2] = (0 - avg_ev);
	}
      tot = 0;

      old_ho[0] = ho[0];
      old_ho[1] = ho[1];
      old_ho[2] = ho[2];
      for (j = 0; j < 3; j++)
	{
	  if (hr[j] >= 0)
	    {
	      ho[j] = hr[j];//+0.00005;
	      //tot += hr[j];
	    }
	  else
	    ho[j] = 0;//.00005;
	}
      //ho[2] = 0;
      tot = ho[0] + ho[1] + ho[2];
      if (tot != 0)
	{
	  for (j = 0; j < 3; j++)
	    {
	      ho[j] /= tot;
	    }
	}
      /*      else
	{
	  ho[0] = 0;
	  ho[1] = 1;
	  ho[2] = 0;
	  }*/
      //      if (fabs(ho[0]-old_ho[0])+ fabs(ho[1]-old_ho[1])+fabs(ho[2]-old_ho[2]) > 1.9 && fabs(ho[0]-old_ho[0])+ fabs(ho[1]-old_ho[1])+fabs(ho[2]-old_ho[2])<2.0)
      //printf("hodiff %f", fabs(ho[0]-old_ho[0])+ fabs(ho[1]-old_ho[1])+fabs(ho[2]-old_ho[2]));
      tot_ho_diff += fabs(ho[0]-old_ho[0])+ fabs(ho[1]-old_ho[1])+fabs(ho[2]-old_ho[2]);
    }
  printf("setting regrets, totreg %f %f\n", hr_tot, tot_ho_diff);
}


void exec_situ_odds(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga)
{
  struct cmd_calc_situ_odds *h;
  int i, hand_i, hi;
 
  double tot_odds[3], tot_odds2[3], *s_odds, *cur_odds, tot;

  h = cmd->cso.data;

  for (i = 0; i < cmd->cso.i; i++)
    {
      s_odds = &ga->situ_odds[h[i].situ_off*3];
      tot_odds[0] = 0;
      tot_odds[1] = 0;
      tot_odds[2] = 0;
      tot_odds2[0] = 0;
      tot_odds2[1] = 0;
      tot_odds2[2] = 0;
      
      for (hand_i = 0; hand_i < h[i].n_hands; hand_i++)
	{
	  hi = h[i].hands_off + hand_i;
	  cur_odds = &ga->hand_odds[hi*3];

	  tot_odds[0] += ga->hand_hw[hi] * cur_odds[0];
	  tot_odds[1] += ga->hand_hw[hi] * cur_odds[1];
	  tot_odds[2] += ga->hand_hw[hi] * cur_odds[2];
	  tot_odds2[0] += cur_odds[0];
	  tot_odds2[1] += cur_odds[1];
	  tot_odds2[2] += cur_odds[2];
	
	}
      tot = tot_odds[0]+tot_odds[1]+tot_odds[2];
      if (tot > 0)
	{
	  s_odds[0] = tot_odds[0]/tot;
	  s_odds[1] = tot_odds[1]/tot;
	  s_odds[2] = tot_odds[2]/tot;
	}
      else
	{
	  tot = tot_odds2[0]+tot_odds2[1]+tot_odds2[2];
	  if (tot > 0)
	    {
	      s_odds[0] = tot_odds2[0]/tot;
	      s_odds[1] = tot_odds2[1]/tot;
	      s_odds[2] = tot_odds2[2]/tot;
	    }
	  else 
	    {
	      s_odds[0] = 0;
	      s_odds[1] = 1;
	      s_odds[2] = 0;
	    }
	} 
    }
}
/* 	cur_sd = malloc(sizeof(short int *)*cur_h->n_plr); */
/* 	cur_hd = malloc(sizeof(double *)*cur_h->n_plr); */

/* 	hands_counted = 0; */
/* 	for (i = 0; i < cur_h->n_plr; i++) */
/* 	  { */
/* 	    cur_hd[i] = &hd[cur_h->hand_data_off+i]; */
/* 	    cur_sd[i] = &sd[cur_h->slot_data_off+hands_counted]; */
/* 	    hands_counted += cur_hd[i]->n_hands; */
/* 	    wtl_sum[i] = calloc(1, sizeof(double)*cur_hd[i]->n_hands*3); */
/* 	  } */

/* 	for (i1 = 0; i1 < h->n_plr; i1++) */
/* 	  { */
/* 	    for (i2 = i1+1; i2 < h->n_plr; i2++) */
/* 	      { */
/* 		for (hdi1 = 0; hdi1 < cur_hd[i1]->n_hands; hdi1++) */
/* 		  { */
/* 		    i1_hw = ga->hand_hw[cur_hd[i1].hand_off+hdi1]; */
/* 		    if (i1_hw <= 0) */
/* 		      continue; */
/* 		    for (hdi2 = 0; hdi2 < cur_hd[i2]->n_hands; hdi2++) */
/* 		      { */
/* 			i2_hw = ga->hand_hw[cur_hd[i2].hand_off+hdi2]; */
/* 			wtl_odds = &info->hand_wtl_odds[info->n_types[GS_R]*2*info->n_types[GS_R]*2 + cur_sd[i1][hdi1]*info->n_types[GS_R]*2 + cur_sd[i2][hdi2]]; */
/* 			tmp_wtl = &wtl_sum[i1][hdi1*3];  */
/* 			tmp_wtl[0] += wtl_odds[0]*i2_hw*i1_hw; */
/* 			tmp_wtl[1] += wtl_odds[1]*i2_hw*i1_hw; */
/* 			tmp_wtl[2] += wtl_odds[2]*i2_hw*i1_hw; */

/* 			tmp_wtl = &wtl_sum[i2][hdi2*3];  */
/* 			tmp_wtl[0] += wtl_odds[2]*i1_hw*i2_hw; */
/* 			tmp_wtl[1] += wtl_odds[1]*i1_hw*i2_hw; */
/* 			tmp_wtl[2] += wtl_odds[0]*i1_hw*i2_hw; */
/* 		      } */
/* 		  } */
/* 	      } */
/* 	  } */
/* 	//normalize */
/* 	for (i = 0; i < cur_h->n_plr; i++) */
/* 	  { */
/* 	    for (j = 0; j < cur_hd[i].n_hands; j++) */
/* 	      { */
/* 		tmp_wtl = &wtl_sum[i][j*3]; */
/* 		tot = tmp_wtl[0] + tmp_wtl[1] + tmp_wtl[2]; */
/* 		if (tot != 0) */
/* 		  { */
/* 		    tmp_wtl[0] /= tot; */
/* 		    tmp_wtl[1] /= tot; */
/* 		    tmp_wtl[2] /= tot; */
/* 		  } */
/* 		ga->hand_evs[(cur_hd[i].hands_off+j)*2] =  */
/* 	      } */
/* 	  } */
	
	    
/* } */
