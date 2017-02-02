#include <string.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "plrmodel.h"
#include "plrmodel_paths.h"



int path_cmp(const void *key1, const void *key2)
{
  struct path *path1, *path2;
  int sum = 0, i;//, val1 = 0, val2 = 0;

  path1 = (struct path*) key1;
  path2 = (struct path*) key2;
  
  if (path1->len == path2->len)
    {
      for (i = 0; i < path1->len; i++)
	{	
	  if ((sum = memcmp((void*)&(path1->items[i].s), (void*)&(path2->items[i].s), sizeof(struct situ*))) != 0)
	    return sum;
	}
      return 0;
    }
  if (path1->len > path2->len)
    return 1;
  return -1;
}

int path_cmp_unique_nodes(const void *key1, const void *key2)
{
  struct path *path1, *path2;
  int sum = 0, i;

  path1 = (struct path*) key1;
  path2 = (struct path*) key2;
  
  if (path1->len == path2->len)
    {
      for (i = 0; i < path1->len; i++)
	{
	  if (path1->items[i].s != NULL && path2->items[i].s != NULL)
	    {
	      if ((sum = memcmp((void*)path1->items[i].s->parent->root, (void*)path2->items[i].s->parent->root, sizeof(path1->items[i].s->parent->root))) != 0)
		return sum;
	    }
	  else
	    {
	      if (path1->items[i].s != NULL || path2->items[i].s != NULL)
		{
		  printf("ASRRKIRROPRIKKI\n");
		}
	     
	    }
	}
       return 0;
    }
  if (path1->len > path2->len)
    return 1;
  return -1;
}

int path_cmp_first_and_last(const void *key1, const void *key2)
{
  struct path *path1, *path2;
  int sum = 0, i;

  path1 = (struct path*) key1;
  path2 = (struct path*) key2;
  
  if (path1->len == path2->len)
    {
      if ((sum = memcmp((void*)&(path1->items[0].s), (void*)&(path2->items[0].s), sizeof(struct situ*))) != 0)
	return sum;
      for (i = 1; i < path1->len; i++)
	{
	  if (path1->items[i].s != NULL && path2->items[i].s != NULL)
	    {
	      if ((sum = memcmp((void*)path1->items[i].s->parent->root, (void*)path2->items[i].s->parent->root, sizeof(path1->items[i].s->parent->root))) != 0)
		return sum;
	    }
	  else
	    {
	      if (path1->items[i].s != NULL || path2->items[i].s != NULL)
		{
		  printf("ASRRKIRROPRIKKI\n");
		}
	      
	    }
	}
      return 0;
    }
  if (path1->len > path2->len)
    return 1;
  return -1;
}

struct path *path_copy(struct path *p)
{
  struct path *new_p = (struct path*)calloc(1,sizeof(struct path)); 
  
  memcpy(new_p, p, sizeof(struct path));
  if (p->items != NULL)
    {
      if (p->len != new_p->len || p->len == 0)
	printf("TOTAOSDTOSDOTOSDT %i %i\n", p->len, new_p->len);
      new_p->items = (struct path_item*)calloc(1,sizeof(struct path_item)*new_p->len);
      memcpy(new_p->items, p->items, sizeof(struct path_item)*new_p->len);
    }
  return new_p;
}

struct path *path_add_to_copy(struct path *p, struct situ *s, char act, short path_idx)
{
  struct path *new_p = (struct path*)calloc(1,sizeof(struct path));   

  memcpy(new_p, p, sizeof(struct path));
  new_p->len = p->len+1;

  new_p->items = (struct path_item*)calloc(1,sizeof(struct path_item)*new_p->len);

  if (p->items != NULL)
    {
      
      memcpy(new_p->items, p->items, sizeof(struct path_item)*p->len);
    }
  else
    if (new_p->len != 1 || p->len != 0)
      {
	printf("ALAMOLO %i %i\n", new_p->len, p->len);
      }
  //new_p->situs[new_p->len-1] = s;
  new_p->items[new_p->len-1].s = s;
  new_p->items[new_p->len-1].path_idx = -1;
  new_p->items[new_p->len-1].act = -1;
  new_p->items[new_p->len-1].pos = 0;
  if (new_p->len > 1 && act != -1)
    {
      new_p->items[new_p->len-2].path_idx = path_idx;
      new_p->items[new_p->len-2].act = act;
    }
  set_path_positions(new_p);
  //  new_p->pos = get_path_positions(new_p);
  return new_p;
}


/* void path_insert_old(struct path *p, struct situ *s, int index) */
/* { */
/*   p->len++; */
/*   p->situs = (struct situ**)realloc(p->situs, p->len*sizeof(struct situ*)); */
/*   if (index == -1) */
/*     { */
/*       p->situs[p->len-1] = s; */
/*     } */
/*   else */
/*     { */
/*       memmove(&(p->situs[index+1]), &(p->situs[index]), sizeof(struct situ*)*(p->len-index-1)); */
/*       p->situs[index] = s; */
/*     } */
/*     if (p->pos != NULL) */
/*     free(p->pos); */
/*   p->pos = get_path_positions(p); */
/*   //set_path_positions(p); */
/* } */

void path_insert(struct path *p, struct situ *s, char act, short path_idx, int index)
{
  p->len++;
  p->items = (struct path_item*)realloc(p->items, p->len*sizeof(struct path_item));
  /*if (index == -1)
    index = p->len-1;
  else
  memmove(&(p->items[index+1]), &(p->items[index]), sizeof(struct path_item)*(p->len-index-1));	*/
  //printf("joopajoo %i %i %i %i %i\n", sizeof(struct path_item), sizeof(struct situ*), sizeof(short), sizeof(char), p->len*sizeof(struct path_item));
  index = p->len-1;
  p->items[index].s = s;
  p->items[index].path_idx = -1;
  p->items[index].act = -1;
  p->items[index].pos = 0;

  if (index > 0 && act != -1)
    {
      p->items[index-1].path_idx = path_idx;
      p->items[index-1].act = act;
    }
  set_path_positions(p);
}


void path_pop(struct path *p, int index)
{
  if (index == -1)
      index = p->len-1;
  if (index < p->len-1)
    memmove(&(p->items[index]), &(p->items[index+1]), sizeof(struct path_item)*(p->len-index-1));
  p->len--;
  set_path_positions(p);
}
  

void *add_path_to_active_tree(void **tree, struct path *p)
{
  return tsearch((void*)p, tree, path_cmp);
}

void delete_path(struct path *p)
{
  if (p != NULL)
    {
      if (p->items != NULL)
	free(p->items);
      free(p);
    }
}

/* int *get_path_positions(struct path *p) */
/* { */
/*   //struct plrmodel_node *match_situ = sl->s; */
/*   //  struct situ_list *tmp_sl; */
/*   struct unique_root *cur_un, *prev_un; */
/*   int i,l; */
/*   int *my_pos; */
/*   if (p->len == 0) */
/*     return NULL; */
  

/*   if (p->situs[p->len-1] == NULL) */
/*     l = p->len-1; */
/*   else */
/*     l = p->len; */

/*   my_pos = (int*)calloc(1,sizeof(int)*l); */
/*   my_pos[l-1] = 0; */
/*   for (i = l-2; i >= 0; i--) */
/*     { */
/*       cur_un = p->situs[i]->parent->root; */
/*       prev_un = p->situs[i+1]->parent->root; */
      
/*       if (cur_un->gamestate != prev_un->gamestate) */
/* 	{ */
/* 	  if (cur_un->n_plr != prev_un->n_plr) */
/* 	    { */
/* 	      if (cur_un->utg_pos == 0) */
/* 		{ */
/* 		  my_pos[i] = (my_pos[i+1] + (1 - prev_un->utg_pos))%prev_un->n_plr; */
/* 		} */
/* 	      else */
/* 		{ */
/* 		  my_pos[i] = (my_pos[i+1] + ((cur_un->utg_pos -1) - prev_un->utg_pos))%prev_un->n_plr; */
/* 		} */
/* 	      my_pos[i] = (my_pos[i+1]+1)%cur_un->n_plr; */
/* 	    } */
/* 	  else */
/* 	    { */
/* 	      my_pos[i] = (my_pos[i+1] + (cur_un->utg_pos - prev_un->utg_pos))%prev_un->n_plr; */
/* 	    } */
/* 	} */
/*       else */
/* 	{ */
/* 	  my_pos[i] = (my_pos[i+1]+1)%cur_un->n_plr;   */
/* 	} */
/*     } */
/*   return my_pos; */
/* } */

void set_path_positions(struct path *p)
{
  //struct plrmodel_node *match_situ = sl->s;
  //  struct situ_list *tmp_sl;
  struct unique_root *cur_un, *prev_un;
  int i,l;
  //int *my_pos;
  if (p->len == 0)
    return;
  

  if (p->items[p->len-1].s == NULL)
    l = p->len-1;
  else
    l = p->len;

  //  my_pos = (int*)calloc(1,sizeof(int)*l);
  //my_pos[l-1] = 0;
  p->items[l-1].pos = 0;
  for (i = l-2; i >= 0; i--)
    {
      cur_un = p->items[i].s->parent->root;
      prev_un = p->items[i+1].s->parent->root;
      
      if (cur_un->gamestate != prev_un->gamestate)
	{
	  if (cur_un->n_plr != prev_un->n_plr)
	    {
	      if (cur_un->utg_pos == 0)
		{
		  p->items[i].pos = (p->items[i+1].pos + (1 - prev_un->utg_pos))%prev_un->n_plr;
		  //my_pos[i] = (my_pos[i+1] + (1 - prev_un->utg_pos))%prev_un->n_plr;
		}
	      else
		{
		  p->items[i].pos = (p->items[i+1].pos + ((cur_un->utg_pos -1) - prev_un->utg_pos))%prev_un->n_plr;
		  //my_pos[i] = (my_pos[i+1] + ((cur_un->utg_pos -1) - prev_un->utg_pos))%prev_un->n_plr;
		}
	      p->items[i].pos = (p->items[i+1].pos+1)%cur_un->n_plr;
	      //my_pos[i] = (my_pos[i+1]+1)%cur_un->n_plr;
	    }
	  else
	    {
	      
	      p->items[i].pos = (p->items[i+1].pos + (cur_un->utg_pos - prev_un->utg_pos))%prev_un->n_plr;
	    }
	}
      else
	{
	  p->items[i].pos = (p->items[i+1].pos+1)%cur_un->n_plr;  
	}
    }
}

/* int find_prev_situ_from_path(struct path *p) */
/* { */
/*   //struct plrmodel_node *match_situ = sl->s; */
/*   //struct situ_list *tmp_sl; */
/*   struct unique_root *cur_un, *prev_un; */
/*   int my_pos, utg_pos, i; */
  
/*   my_pos = 0; */

/*   for (i = p->len-2; i >= 0; i--) */
/*     { */
/*       cur_un = p->situs[i]->parent->root; */
/*       prev_un = p->situs[i+1]->parent->root; */
      
/*       if (cur_un->gamestate != prev_un->gamestate) */
/* 	{ */
/* 	  if (cur_un->n_plr != prev_un->n_plr) */
/* 	    { */
/* 	      if (cur_un->utg_pos == 0) */
/* 		{ */
/* 		  my_pos = (my_pos + (1 - utg_pos))%prev_un->n_plr; */
/* 		} */
/* 	      else */
/* 		{ */
/* 		  my_pos = (my_pos + ((cur_un->utg_pos -1) - utg_pos))%prev_un->n_plr; */
/* 		} */
/* 	      my_pos = (my_pos+1)%cur_un->n_plr; */
/* 	    } */
/* 	  else */
/* 	    { */
/* 	      my_pos = (my_pos + (cur_un->utg_pos - utg_pos))%prev_un->n_plr; */
/* 	    } */
/* 	} */
/*       else */
/* 	{ */
/* 	  my_pos = (my_pos+1)%cur_un->n_plr;   */
/* 	} */

/*       if (my_pos == 0) */
/* 	{ */
/* 	  return i; */
/* 	} */
/*     } */
/*   return -1; */
/* } */

int situ_fits_to_path(struct path *p, struct situ *s)
{
  struct situ_type *new_st, *tmp_st;
  int i;

  new_st = get_situ_type_from_situ(s->parent);
  
  for (i = 0; i < p->len; i++)
    {
      if (p->items[i].s != NULL)
	{
	  tmp_st = get_situ_type_from_situ(p->items[i].s->parent);
	  if (is_matching_situs(tmp_st, new_st) != 0)
	    {
	      free(new_st);
	      free(tmp_st);
	      return 0;
	    }
	  free(tmp_st);
	}
    }
  free(new_st);
  return 1;
}

/* int add_situ_to_path(struct path *p, struct situ *s) */
/* { */
/*   if (!situ_fits_to_path(p, s)) */
/*     return 0; */

/*   p->len++; */
/*   p->situs = (struct situ**)realloc(p->situs, p->len*sizeof(struct situ*)); */
/*   p->situs[p->len-1] = s; */
/*   return 1; */
/* } */
  




    
