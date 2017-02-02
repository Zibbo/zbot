#ifndef _PLRMODEL_PATHS_H_
#define _PLRMODEL_PATHS_H_


#include "defs.h"
int path_cmp(const void *key1, const void *key2);
int path_cmp_unique_nodes(const void *key1, const void *key2);
int path_cmp_first_and_last(const void *key1, const void *key2);
struct path *path_copy(struct path *p);
//struct path *path_add_to_copy(struct path *p, struct situ *s);
struct path *path_add_to_copy(struct path *p, struct situ *s, char act, short path_idx);
//void path_insert(struct path *p, struct situ *s, int index);
void path_insert(struct path *p, struct situ *s, char act, short path_idx, int index);

void path_pop(struct path *p, int index);
void *add_path_to_active_tree(void **tree, struct path *p);
void delete_path(struct path *p);
int *get_path_positions(struct path *p);
void set_path_positions(struct path *p);
int find_prev_situ_from_path(struct path *p);
int situ_fits_to_path(struct path *p, struct situ *s);

#endif
