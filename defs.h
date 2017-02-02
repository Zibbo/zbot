#ifndef DEFS_H
#define DEFS_H

#include <limits.h>
#include <stdint.h>

#include <poker_defs.h>
//#include "uthash.h"

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

#define prefetch(x) __builtin_prefetch(x)
#define prefetchw(x) __builtin_prefetch(x,1)

#define PREFETCH_STRIDE 64

static inline void prefetch_range(void *addr, size_t len)
{
  char *cp;
  char *end = addr + len;
  
  for (cp = addr; cp < end; cp += PREFETCH_STRIDE)
    prefetch(cp);
}

#define MAX_READERS 1024

#define HANDS 1326
#define FLOPS 22100
#define SAMPLES 1081
#define DATA_SAMPLES 1081
#define PATH_LEVEL_INC 10

#define GS_PF 0
#define GS_F 1
#define GS_T 2
#define GS_R 3
#define GS_S 4

#define RAISE 0
#define CALL 1
#define FOLD 2

#define AVG_N 100
#define EPS (1.0/16777216.0)
#define ACTS 3
#define MAX_PLR 2


#define N_PUB_TYPES 10
#define N_PRIV_TYPES 4
#define MAX_PATH_LEN 32
#define MAX_VARIATION 4
#define MAX_HWEV 2

#define NODE_MIN_LIFE 1000

//FLAGS

#define POV_ODDS_FROM_REGS 0x1
#define POV_ODDS_FROM_AVG 0x2
#define POV_ODDS_FROM_BYTE_ODDS 0x4
#define UPDATE_AVG_ODDS 0x8
#define UPDATE_REGS 0x10


#define PM_RESET_VISITS 0x1
#define PM_RESET_EV_DIFF 0x2
#define PM_RESET_HAND_COUNT 0x4
#define PM_RESET_VALID_BMAP 0x8
#define SITU_RESET_HAND_ODDS 0x10
#define SITU_RESET_REGS 0x20
#define SITU_RESET_D_REGS 0x40
#define SITU_RESET_AVG_ODDS 0x80
#define SITU_RESET_EV 0x100
#define SITU_RESET_EV_DIFF 0x200
#define SITU_RESET_VISITS 0x400
#define PM_RESET_TYPE_MAPPING 0x800
#define PM_RESET_TYPE_MAPPING_ALT 0x1000

#define BR_LOAD_FLOP_DATA 0x1
#define BR_SAVE_FLOP_DATA 0x2

struct hand_info
{
  struct unique_root *root_us;
  struct unique_root *cur_us;
  uint8_t acts_i[4];
  uint8_t acts[4][32];
  int8_t hole_cards[4];
  int8_t board[5];
  int8_t padding[3];
};

struct type_type
{
  int id;
  int style; //0: linear int, 1:linear float, 2:diffs
  int public;
  int local;
  int n_types;
  int n_items_per_type;
  // int n_type_space;
  int gamestate;
  struct gameinfo *info;
  int16_t *diffs_order;
  float *diffs;
  uint64_t *diffs_lookup;
  float *types;
  int16_t *slots;
  char *diffs_order_filename;
  char *diffs_filename;
  char *diffs_lookup_filename;
  char *types_filename;
  char *slots_filename;
  int16_t (*get_slot) (const struct type_type *, const struct hand_info *);
  void (*gen_type) (const struct type_type *, const struct hand_info *, const float *);
};

struct sd_plr
{
  double regs_decay;
  double ev_decay;
  double odds_change;
  double positive_regs;
  int hands_updated;
  int reset_ev_every;
  int update_hand_odds_every;
  int hand_odds_from;
  
};

struct simu_data
{
  struct gameinfo *info;
  int16_t *board_slots; 
  int16_t **hand_slots; 
  unsigned int *handvals;
  
  struct sd_plr plr[10];
 //int update_mode[10];
  //int update_every[10];
};

/* struct traverse_data */
/* { */
/*   uint64_t seq; */
/*   int pt_sample_size;   */
/*   int16_t *public_types; */
/*   int16_t **private_types; */
/*   int self_pos; */
/*   int cur_pos; */
/*   struct hand_hv2 *handval; */
/*   struct traverse_plr_data *pd; */
/*   struct sd_plr *plr; */
  
/*   /\*  struct gameinfo *info; */
/*   short int *board; */
/*   short int *hand; */
/*   struct hand_hv *handval; */
/*   struct sd_plr plr[MAX_PLR]; */
/*   unsigned int self_pos; */
/*   unsigned int n_plr;*\/ */
/* }; */
struct traverse_plr_data
{
  double *hw;
  double *ev;
  int *path_level;
  double pw;
};

/* struct traverse_data */
/* { */
/*   // struct traverse_hand_data *hd; */
/*   struct traverse_plr_data *pd;   */
/*   unsigned int n_plr; */
/* }; */

struct unique_root
{
  int root_idx;
  int16_t n_plr;
  int16_t to_act;
  int gamestate;
  int cur_seat;
  double *bets;
  double action_cost[3];
  int n_type_types[2];
  int *type_types_order[2];

  struct unique_root *next[ACTS];
  struct plrmodel_node *model_tree;
  pthread_rwlock_t *mutex_pool;
  //struct sockaddr_in *handler_addr;
  //  struct zp_queue *in_queue;
  //pthread_t handler_thread;
};



/* struct plrmodel_node_old */
/* { */
/*   struct type_type *t; */
/*   int16_t len;  */
/*   int16_t slot_in_prev; */
/*   int32_t lock; */
/*   double visits[2]; */
/*   double avg_odds_diff; */
/*   uint32_t avg_odds_diff_count; */
/*   uint32_t hand_count; */
/*   uint64_t timestamp; */
/*   struct unique_root *root; */
/*   struct plrmodel_node *prev;  */
/*   uint64_t *types_bmap; */
/*   uint64_t *valid_bmap; */
/*   int16_t *type_mapping; */
/*   int16_t *type_mapping_alt; */

/*   void **next_list; // viimeisessa osoittaa situ structiin */
/* }; */

struct plrmodel_node
{
  uint32_t use_lock;
  uint32_t struct_lock;
  struct type_type *t;
  struct unique_root *root;
  struct plrmodel_node *prev; 
  uint64_t *types_bmap;
  //uint64_t *expand_bmap;
  int16_t *type_mapping;
  void **next_list; // viimeisessa osoittaa situ structiin
  uint64_t data_index;
  uint32_t data_format;
  
  uint32_t pub_node_count;
  uint32_t id;
  int16_t len; 
  int16_t slot_in_prev;  
};


struct situ_list
{
  struct situ_list *next;
  struct plrmodel_node *s;
};

struct situ_list_new
{
  struct situ_list_new *next;
  struct situ *s;

};



struct situ
{
  double *regs[ACTS];
  double *avg_odds[ACTS];
  uint8_t *byte_odds;
};






struct rivertype
{
  int wl_slots;
  int tie_slots;
  double wl_scale;
  double tie_scale;
  double wl_width;
  double tie_width;
};

struct gameinfo
{
  float *types[4];
  float *diffs[4];
  short int *diffs_order[4];
  short int *slots[4];
  float *b_types[4];
  float *b_diffs[4];
  short int *b_diffs_order[4];
  short int *b_slots[4];
  int n_types[4];
  int n_btypes[4];
  int n_rtypes[4];
  int n_htfb[4];
  float gauss_width[4];
  int n_type_types[2];
  struct type_type *type_types[2];
  float small_blind;
  float big_blind;
  int maxbets;
  int n_plr;
  
};


/* struct plrmodel_node */
/* { */
/*   struct plrmodel_node *raise; */
/*   struct plrmodel_node *call; */
/*   double potsize; */
/*   double bets; */
/*   double tocall; */
/*   double toraise; */
/*   int32_t pos; */
/*   int32_t gamestate; */
/*   int32_t wt_index[2]; */
/* }; */

struct d3
{
  double d[3];
};

struct d2
{
  double d[2];
};

struct f3
{
  float f[3];
};

struct f2
{
  float f[2];
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

struct wtl
{
  short int w;
  short int t;
  short int l;
};

struct wtl_f
{
  double w;
  double t;
  double l;
};

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

struct ev
{
  double raise;
  double call;
  double fold;
};


/*struct simu_hand
{
  CardMask board[4];
  int hv[4];
  int winner;
  };*/


struct simu_tmp_data
{
  double ev[HANDS];
  unsigned int ev_count[HANDS];
};

struct street_types
{
  int16_t board_type;
  int16_t hand_types[HANDS];
};

struct hv_slots
{
  char hv[3];
};

struct hand_hv_river
{
  short int board_cards;
  short int board_slot;
  short int hand_slots[HANDS];
  struct hand_hv hand_hvs[HANDS];
};

struct hand_hv_turn
{
  short int board_cards;
  short int board_slot;
  short int hand_slots[HANDS];
  struct hand_hv_river river[52];
};


struct hand_hv_flop
{
  short int board_cards;
  short int board_slot;
  short int hand_slots[HANDS];
  struct hand_hv_turn turn[52];
};

struct mem_list
{
  struct mem_list *next;
  void *mem;
};



#endif



