from ctypes import *
import numpy as np
import datatypes as dt
import sys
from numpy.ctypeslib import ndpointer

if "linux" in sys.platform:
    lib = cdll.LoadLibrary("./libzpoker.so")
else:
    lib = cdll.LoadLibrary("libzpoker.dylib")
lib.precalc_conversions()
lib.zrandom_seed()

HANDS = 1326
FLOPS = 22100
TURNS = 270725
RIVERS = 2598960

ctoi2 = (c_int*52*52).in_dll(lib, "cards_to_int_2")
ctoi3 = (c_int*52*52*52).in_dll(lib, "cards_to_int_3")
ctoi4 = (c_int*52*52*52*52).in_dll(lib, "cards_to_int_4")

itoc2 = (dt.cards_2*HANDS).in_dll(lib, "int_to_cards_2")
itoc3 = (dt.cards_3*FLOPS).in_dll(lib, "int_to_cards_3")
itoc4 = (dt.cards_4*TURNS).in_dll(lib, "int_to_cards_4")


lib.gen_all_preflop_types.argtypes = [c_void_p,c_void_p]
lib.gen_types_for_flop.argtypes = [c_void_p,c_void_p, c_int]
lib.gen_types_for_turn.argtypes = [c_void_p,c_void_p, c_int, c_int]
lib.gen_random_turn_type.argtypes = [c_void_p,c_void_p]
lib.gen_random_river_type.argtypes = [c_void_p,c_void_p]
lib.gen_single_river_type.argtypes = [c_void_p,c_void_p,c_int, c_int, c_int, c_int]

lib.get_difference.argtypes = [c_void_p, c_void_p, c_int]
lib.get_difference.restype = c_float

lib.get_difference_pow2.argtypes = [c_void_p, c_void_p, c_int]
lib.get_difference_pow2.restype = c_float

#lib.get_diff_cuda.argtypes = [c_void_p, c_void_p, c_int]
#lib.get_diff_cuda.restype = c_float

lib.calc_diffs_for_one_type.argtypes = [c_void_p, c_void_p, c_void_p, c_int, c_int]


lib.calc_diffs.argtypes = [c_void_p, c_void_p, c_int, c_int, c_int]
lib.calc_diffs.restype = c_float

lib.get_closest_match.argtypes = [c_void_p, c_int, POINTER(c_int), POINTER(c_int)]
lib.get_closest_match.restype = c_float

lib.try_new_type.argtypes = [c_void_p, c_void_p, c_void_p, c_int, c_int, c_float, c_int]
lib.try_new_type.restype = c_float

lib.get_slot.argtypes = [c_void_p, c_void_p, c_int, c_int]
lib.get_slot.restype = c_short

lib.get_slots_for_all.argtypes = [c_void_p, c_void_p, c_void_p, c_int, c_int, c_int]
lib.get_slots_for_river.argtypes = [c_void_p, c_void_p, c_int, c_int, c_int]

lib.get_slots_wtl_stats_for_river.argtypes = [c_void_p, c_void_p, c_int, c_int, c_int]
lib.get_slots_wtl_stats_for_river.restype = c_short


lib.generate_mapping_from_diffs.argtypes = [c_void_p, c_void_p, c_int, c_int]

lib.gen_board_type.argtypes = [c_void_p, c_void_p, c_int, c_int, c_int, c_int]

lib.find_two_smallest.argtypes = [c_void_p, c_void_p, c_int, c_int]

lib.add_values_to_gs_switch_and_odds.argtypes = [c_void_p, c_void_p, c_void_p, c_void_p, c_int]
lib.get_river_handhv_all.argtypes = [c_int, c_int, c_int, POINTER(c_uint32)]
lib.get_river_hand_hv_all.argtypes = [c_int, c_int, c_int, POINTER(dt.hand_hv)]

queue_init = lib.queue_init
queue_init.argtypes = []
queue_init.restype = c_void_p                    


gen_minimal_plrmodel_tree = lib.gen_minimal_plrmodel_tree
gen_minimal_plrmodel_tree.argtypes = [POINTER(dt.gameinfo), POINTER(dt.unique_root), c_uint64]
gen_minimal_plrmodel_tree.restype = POINTER(dt.plrmodel_node)


queue_get_item_count = lib.queue_get_item_count
queue_get_item_count.argtypes = [c_void_p]
queue_get_item_count.restype = c_int

zp_queue_get_item_count = lib.zp_queue_get_item_count
zp_queue_get_item_count.argtypes = [c_void_p]
zp_queue_get_item_count.restype = c_int



queue_lock = lib.queue_lock
queue_lock.argtypes = [c_void_p]
queue_lock.restype = c_int

queue_unlock = lib.queue_unlock
queue_unlock.argtypes = [c_void_p]

zp_queue_lock = lib.zp_queue_lock
zp_queue_lock.argtypes = [c_void_p]
zp_queue_lock.restype = c_int

zp_queue_unlock = lib.zp_queue_unlock
zp_queue_unlock.argtypes = [c_void_p]

print_plrmodel_tree = lib.print_plrmodel_tree
print_plrmodel_tree.argtypes = [POINTER(dt.plrmodel_node), c_uint32]

recursive_reset_plrmodel_data = lib.recursive_reset_plrmodel_data
recursive_reset_plrmodel_data.argtypes = [POINTER(dt.plrmodel_node), c_uint64]

count_pmn = lib.count_pmn
count_pmn.argtypes =  [POINTER(dt.plrmodel_node)]
count_pmn.restype = c_int

recount_pub_nodes = lib.recount_pub_nodes
recount_pub_nodes.argtypes = [POINTER(dt.plrmodel_node)]

set_st_from_hand_info = lib.set_st_from_hand_info
set_st_from_hand_info.argtypes = [POINTER(dt.gameinfo), POINTER(dt.hand_info), POINTER(c_int16), POINTER(c_int16)]

set_mutable_types_from_hand_info = lib.set_mutable_types_from_hand_info
set_mutable_types_from_hand_info.argtypes = [POINTER(dt.gameinfo), POINTER(dt.hand_info), POINTER(c_int16), POINTER(c_int16)]

get_action_odds_from_file = lib.get_action_odds_from_file
get_action_odds_from_file.argtypes = [c_int, POINTER(dt.gameinfo), POINTER(c_int16), POINTER(c_int16), POINTER(c_float), c_int]


add_action_to_hi = lib.add_action_to_hi
add_action_to_hi.argtypes = [POINTER(dt.hand_info), c_int]

get_blank_hi = lib.get_blank_hi
get_blank_hi.argtypes = [POINTER(dt.unique_root)]
get_blank_hi.restype = POINTER(dt.hand_info)

get_potsize_slot_from_hi  = lib.get_potsize_slot_from_hi
get_potsize_slot_from_hi.argtypes = [POINTER(dt.type_type), POINTER(dt.hand_info)]
get_potsize_slot_from_hi.restype = c_int16

get_last_act_slot_from_hi  = lib.get_last_act_slot_from_hi
get_last_act_slot_from_hi.argtypes = [POINTER(dt.type_type), POINTER(dt.hand_info)]
get_last_act_slot_from_hi.restype = c_int16

get_bets_slot_from_hi  = lib.get_bets_slot_from_hi
get_bets_slot_from_hi.argtypes = [POINTER(dt.type_type), POINTER(dt.hand_info)]
get_bets_slot_from_hi.restype = c_int16

get_board_slot_from_hi  = lib.get_board_slot_from_hi
get_board_slot_from_hi.argtypes = [POINTER(dt.type_type), POINTER(dt.hand_info)]
get_board_slot_from_hi.restype = c_int16

get_board_combo_slot_from_hi  = lib.get_board_combo_slot_from_hi
get_board_combo_slot_from_hi.argtypes = [POINTER(dt.type_type), POINTER(dt.hand_info)]
get_board_combo_slot_from_hi.restype = c_int16

gen_board_type_from_hi= lib.gen_board_type_from_hi
gen_board_type_from_hi.argtypes = [POINTER(dt.type_type), POINTER(dt.hand_info), POINTER(c_float)]
#gen_board_type_from_hi.restype = 


get_hand_slot_from_hi  = lib.get_hand_slot_from_hi
get_hand_slot_from_hi.argtypes = [POINTER(dt.type_type), POINTER(dt.hand_info)]
get_hand_slot_from_hi.restype = c_int16

#get_action_and_update_spread_from_file = lib.get_action_and_update_spread_from_file
#get_action_and_update_spread_from_file.argtypes = [c_int, POINTER(dt.gameinfo), c_void_p, c_void_p]
#get_action_and_update_spread_from_file.restype = c_int

set_mutable_types_from_path = lib.set_mutable_types_from_path
set_mutable_types_from_path.argtypes = [POINTER(c_int16), c_char_p, c_int, POINTER(dt.unique_root)]
set_mutable_types_from_path.restype = POINTER(dt.unique_root)

get_victory_odds_against_spread_float = lib.get_victory_odds_against_spread_float
get_victory_odds_against_spread_float.argtypes = [POINTER(c_void_p*2), c_void_p, c_int]

get_victory_odds_against_spread_float_hard_way = lib.get_victory_odds_against_spread_float_hard_way
get_victory_odds_against_spread_float_hard_way.argtypes = [POINTER(c_void_p*2), c_void_p, c_int]

load_plrmodel_tree = lib.load_plrmodel_tree
load_plrmodel_tree.argtypes = [POINTER(dt.gameinfo), c_int,  POINTER(dt.unique_root)]
load_plrmodel_tree.restype = POINTER(dt.plrmodel_node)

zrandom_seed = lib.zrandom_seed

tt_call_gen_type = lib.tt_call_gen_type
tt_call_gen_type.argtypes = [POINTER(dt.type_type), POINTER(dt.hand_info), POINTER(c_float)]

fill_diffs_lookup = lib.fill_diffs_lookup
fill_diffs_lookup.argtypes = [POINTER(dt.type_type), POINTER(c_float)]
fill_diffs_lookup.restype = c_int

get_rng = lib.get_rng
get_rng.restype = c_void_p




walk_tree = lib.walk_tree
walk_tree.argtypes = [POINTER(dt.unique_root), ndpointer(dt.types_data_np), POINTER(POINTER(c_double)*2),POINTER(POINTER(c_double)*2), POINTER(c_double), c_int, POINTER(c_double), POINTER(c_double), c_double, POINTER(c_uint64), c_void_p, c_double, c_double, POINTER(POINTER(dt.mem_list))]
#walk_tree.restype = POINTER(POINTER(c_double)*2)

br_solve = lib.br_solve
br_solve.argtypes = [POINTER(dt.unique_root), c_char_p, c_int, c_int, c_int, c_int, c_int, POINTER(dt.street_types), POINTER(dt.street_types), POINTER(dt.street_types), POINTER(dt.street_types), POINTER(dt.hand_hv2), POINTER(c_int16),POINTER(c_int16), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_uint64), c_void_p, c_char_p] 
br_solve.restype = c_double

save_gameinfo = lib.save_gameinfo
save_gameinfo.argtypes = [POINTER(dt.gameinfo), c_int]

save_unique_roots = lib.save_unique_roots
save_unique_roots.argtypes = [POINTER(dt.unique_root), c_int, c_int]


get_board_combo_slot = lib.get_board_combo_slot
get_board_combo_slot.argtypes = [POINTER(dt.gameinfo), POINTER(c_float), POINTER(c_int16), POINTER(c_int16), c_int, c_int]
get_board_combo_slot.restype = c_int16

get_board_combo_diffs = lib.get_board_combo_diffs
get_board_combo_diffs.argtypes = [POINTER(dt.gameinfo), POINTER(c_float), POINTER(c_int16), POINTER(c_int16), c_int, c_int, POINTER(c_float)]

expand_type_max = lib.expand_type_max
expand_type_max.argtypes = [POINTER(dt.plrmodel_node), c_int, c_int]

gen_types_data = lib.gen_types_data
gen_types_data.argtypes = [POINTER(dt.gameinfo), POINTER(c_int8), c_void_p]

free_type_count_from_types_data = lib.free_type_count_from_types_data
free_type_count_from_types_data.argtypes = [ndpointer(dt.types_data_np)]


adjust_plrmodel_tree = lib.adjust_plrmodel_tree
adjust_plrmodel_tree.argtypes = [POINTER(dt.plrmodel_node), c_double, c_double, c_void_p, POINTER(c_uint64)]
#adjust_plrmodel_tree.restype = c_int64

save_plrmodel_struct = lib.save_plrmodel_struct
save_plrmodel_struct.argtypes = [c_int, POINTER(dt.plrmodel_node)]

save_plrmodel_regs = lib.save_plrmodel_regs
save_plrmodel_regs.argtypes = [c_int, POINTER(dt.plrmodel_node)]

save_plrmodel_avg_odds = lib.save_plrmodel_avg_odds
save_plrmodel_avg_odds.argtypes = [c_int, POINTER(dt.plrmodel_node)]

save_plrmodel_byte_odds_from_avg = lib.save_plrmodel_byte_odds_from_avg
save_plrmodel_byte_odds_from_avg.argtypes = [c_int, POINTER(dt.plrmodel_node)]

save_plrmodel_byte_odds_from_regs = lib.save_plrmodel_byte_odds_from_regs
save_plrmodel_byte_odds_from_regs.argtypes = [c_int, POINTER(dt.plrmodel_node)]

load_plrmodel_struct = lib.load_plrmodel_struct
load_plrmodel_struct.argtypes = [POINTER(dt.gameinfo), c_int, POINTER(dt.unique_root)]
load_plrmodel_struct.restype = POINTER(dt.plrmodel_node)

load_plrmodel_regs = lib.load_plrmodel_regs
load_plrmodel_regs.argtypes = [POINTER(dt.plrmodel_node), c_int]

load_plrmodel_avg_odds = lib.load_plrmodel_avg_odds
load_plrmodel_avg_odds.argtypes = [POINTER(dt.plrmodel_node), c_int]

load_plrmodel_byte_odds = lib.load_plrmodel_byte_odds
load_plrmodel_byte_odds.argtypes = [POINTER(dt.plrmodel_node), c_int]

set_regs_from_mmap = lib.set_regs_from_mmap
set_regs_from_mmap.argtypes = [POINTER(dt.plrmodel_node), POINTER(c_double)]

set_avg_odds_from_mmap = lib.set_avg_odds_from_mmap
set_avg_odds_from_mmap.argtypes = [POINTER(dt.plrmodel_node), POINTER(c_double)]

set_byte_odds_from_mmap = lib.set_byte_odds_from_mmap
set_byte_odds_from_mmap.argtypes = [POINTER(dt.plrmodel_node), POINTER(c_uint8)]

count_hands = lib.count_hands
count_hands.argtypes = [POINTER(dt.plrmodel_node)]
count_hands.restype = c_uint64

count_zero_regs_hands = lib.count_zero_regs_hands
count_zero_regs_hands.argtypes = [POINTER(dt.plrmodel_node)]
count_zero_regs_hands.restype = c_uint64

prune_zero_regs_hands = lib.prune_zero_regs_hands
prune_zero_regs_hands.argtypes = [POINTER(dt.plrmodel_node)]
prune_zero_regs_hands.restype = c_uint64

count_zero_avg_hands = lib.count_zero_avg_hands
count_zero_avg_hands.argtypes = [POINTER(dt.plrmodel_node)]
count_zero_avg_hands.restype = c_uint64

prune_zero_avg_hands = lib.prune_zero_avg_hands
prune_zero_avg_hands.argtypes = [POINTER(dt.plrmodel_node)]
prune_zero_avg_hands.restype = c_uint64

recount_data_index = lib.recount_data_index
recount_data_index.argtypes = [POINTER(dt.plrmodel_node), c_uint64]
recount_data_index.restype = c_uint64

get_first_matching_situ = lib.get_first_matching_situ
get_first_matching_situ.argtypes = [POINTER(dt.plrmodel_node), POINTER(c_int16), c_double, c_void_p, c_int]
get_first_matching_situ.restype = POINTER(dt.plrmodel_node)

get_first_matching_situ_weighed_random = lib.get_first_matching_situ_weighed_random
get_first_matching_situ_weighed_random.argtypes = [POINTER(dt.plrmodel_node), POINTER(c_int16), c_double, c_void_p, c_int]
get_first_matching_situ_weighed_random.restype = POINTER(dt.plrmodel_node)


get_closest_local_type = lib.get_closest_local_type
get_closest_local_type.argtypes = [POINTER(dt.plrmodel_node), c_int16]
get_closest_local_type.restype = c_int16

find_n_closest_type_diffs_valid_edge = lib.find_n_closest_type_diffs_valid_edge
find_n_closest_type_diffs_valid_edge.argtypes = [POINTER(c_int16), POINTER(c_float), POINTER(c_uint64), c_int, c_int16, POINTER(c_int16), POINTER(c_double), c_int]


get_hand_type = lib.get_hand_type
get_hand_type.argtypes = [POINTER(dt.gameinfo), c_int, POINTER(c_int16), c_int]
get_hand_type.restype = c_int16

get_hand_types = lib.get_hand_types
get_hand_types.argtypes = [POINTER(dt.gameinfo), c_int, POINTER(c_int16)]
get_hand_types.restype = POINTER(c_int16)


get_board_type = lib.get_board_type
get_board_type.argtypes = [POINTER(dt.gameinfo), c_int, POINTER(c_int16)]
get_board_type.restype = c_int16

get_hand_hv2 = lib.get_hand_hv2
get_hand_hv2.argtypes = [POINTER(c_int8), POINTER(dt.hand_hv2)]


get_action_odds = lib.get_action_odds
get_action_odds.argtypes = [POINTER(dt.unique_root), POINTER(c_int16), POINTER(c_int16), POINTER(c_double), c_void_p]


get_showdown_ev_one_hand_gen = lib.get_showdown_ev_one_hand_gen
get_showdown_ev_one_hand_gen.argtypes = [POINTER(c_double), POINTER(dt.hand_hv2), c_double, c_double, c_int, c_int]
get_showdown_ev_one_hand_gen.restype = c_double

if __name__ == "__main__":
    import zpoker
    a = c_void_p()
    print a
    print lib.precalc_conversions
    print "cast", cast(lib.precalc_conversions, c_void_p)
    print "addressod", addressof(lib.precalc_conversions)
    a.value = addressof(lib.precalc_conversions)
    print a
    print pointer(lib.precalc_conversions).contents
    print byref(lib.precalc_conversions)
    print prot
    t = test_struct()
    zp = zpoker.zpoker("testipeli/")
