#ifndef _PLRMODEL_CMD_LIST_H_
#define _PLRMODEL_CMD_LIST_H_

#include "defs.h"


void calc_rcf_odds_for_one_path_node(struct cmd_lists *cmd, struct memory_pointers *memp, struct path *p, int index, int len);

void calc_path_odds_from_node_odds(struct cmd_lists *cmd, struct memory_pointers *memp, unsigned int odds_start, int len, unsigned int first_node_act);

void calc_path_weight_and_potsize(struct cmd_lists *cmd, struct path *p, double structure_odds, unsigned int p_gdo, double action_cost);
void calc_hand_hw(struct cmd_lists *cmd, struct situ *src, struct situ *dest, double *hand_odds, unsigned int p_gdo, double structure_odds);
void calc_ev(struct cmd_lists *cmd, struct situ *prev, struct situ *cur, double *hand_odds, unsigned int p_gdo, double structure_odds, double own_action_cost, double other_action_cost);
void calc_ev_fold(struct cmd_lists *cmd, struct situ *prev, struct situ *cur, unsigned int p_gdo, double structure_odds, double own_action_cost, double other_action_cost);
void calc_sd_value(struct gameinfo *info, struct cmd_lists *cmd, struct plrmodel_node *n);
void calc_situ_odds(struct cmd_lists *cmd, struct situ *s);
void exec_path_node_odds(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga);
void exec_path_odds_from_node_odds(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga);
void exec_path_weight_and_potsize(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga);
void exec_correct_zero_potsize(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga);
void exec_normalize_potsize(struct global_arrays *ga, int n_situs);
void exec_hand_hw(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga);
void exec_ev_fold(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga);
void exec_ev(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga);
void exec_ev_sd(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga);
void exec_sd_value(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga);
void exec_regret(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga, unsigned int n_hands, int n_plr);
void exec_situ_odds(struct gameinfo *info, struct cmd_lists *cmd, struct global_arrays *ga);
#endif 
