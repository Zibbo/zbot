from ctypes import *
import numpy as np
import defs
from collections import namedtuple
MAX_PLR = 2
HANDS = 1326


path_nfo = namedtuple("path_nfo", ["seat", "gamestate", "bets", "potsize", "stake", "toraise", "tocall", "actions"])

class mystruct(Structure):
    def print_fields(self):
        for field in self._fields_:
            exec("print '"+field[0] + "', self." + field[0])



class wtl(Structure):
    _fields_ = [("w", c_short),
                ("t", c_short),
                ("l", c_short)]

class rcf(Structure):
    _fields_ = [("r", c_double),
                ("c", c_double),
                ("f", c_double)]


class cards_1(Structure):
    _fields_ = [("c1", c_short)]

class cards_2(Structure):
    _fields_ = [("c1", c_short),
                ("c2", c_short)]

class cards_3(Structure):
    _fields_ = [("c1", c_short),
                ("c2", c_short),
                ("c3", c_short)]

class cards_4(Structure):
    _fields_ = [("c1", c_short),
                ("c2", c_short),
                ("c3", c_short),
                ("c4", c_short)]

class rivertype(Structure):
    _fields_ = [("wl_slots", c_int),
                ("tie_slots", c_int),
                ("wl_scale", c_double),
                ("tie_scale", c_double),
                ("wl_width", c_double),
                ("tie_width", c_double)]


class hand_hv(Structure):
    _fields_ = [("hv", c_uint),
                ("hand", c_short),
                ("c1", c_int8),
                ("c2", c_int8)]

class hand_hv2(Structure):
    _fields_ = [("hv", c_uint16),
                ("hand", c_int16)]

class gameinfo(Structure):
    pass

class type_type(mystruct):
    pass
type_type._fields_ = [("id", c_int32),
                      ("style", c_int32),
                      ("public", c_int32),
                      ("local", c_int32),
                      ("n_types", c_int32),
                      ("n_items_per_type", c_int32),
                      ("gamestate", c_int32),
                      ("info", POINTER(gameinfo)),
                      ("diffs_order", POINTER(c_int16)),
                      ("diffs", POINTER(c_float)),
                      ("diffs_lookup", POINTER(c_uint64)),
                      ("types", POINTER(c_float)),
                      ("slots", POINTER(c_int16)),
                      ("diffs_order_filename", c_char_p),
                      ("diffs_filename", c_char_p),
                      ("diffs_lookup_filename", c_char_p),
                      ("types_filename", c_char_p),
                      ("slots_filename", c_char_p),
                      ("get_slot", c_void_p),
                      ("gen_type", c_void_p)]



gameinfo._fields_ = [("types", (POINTER(c_float)*4)),
                     ("diffs", (POINTER(c_float)*4)),
                     ("diffs_order", (POINTER(c_short)*4)),    
                     ("slots", (POINTER(c_short)*4)),
                     ("b_types", (POINTER(c_float)*4)),
                     ("b_diffs", (POINTER(c_float)*4)),
                     ("b_diffs_order", (POINTER(c_short)*4)),
                     ("b_slots", (POINTER(c_short)*4)),
                     ("n_types", (c_int*4)),
                     ("n_btypes", (c_int*4)),
                     ("n_rtypes", (c_int*4)),
                     ("n_htfb", (c_int*4)),
                     ("gauss_width", (c_float*4)),
                     ("n_type_types", c_int*2),
                     ("type_types", POINTER(type_type)*2),
                     ("sb", c_float),
                     ("bb", c_float),
                     ("maxbets", c_int),
                     ("n_plr", c_int)]


class dataclass:
    pass



class slot(mystruct):
    _fields_ = [("start", c_uint16),
                ("stop", c_uint16)]

class sd_plr(mystruct):
    _fields_ = [("regs_decay", c_double),
                ("ev_decay", c_double),
                ("odds_change", c_double),
                ("positive_regs", c_double),
                ("hands_updated", c_int),
                ("reset_ev_every", c_int),
                ("update_hand_odds_every", c_int),
                ("hand_odds_from", c_int)]

class simu_data(mystruct):
    pass
class unique_root(mystruct):
    pass

class plrmodel_node(mystruct):
    pass

class situ_hand_data(mystruct):
    _fields_ = [("visits", c_uint),
                ("visits_tot", c_uint),
                ("path_level", c_uint),
                ("padding", c_uint),
                ("regs", c_double*3),
                ("avg_odds", c_double*3),
                ("ev", c_double*2),
                ("wev", c_double*2)]


class traverse_data(mystruct):
    _fields_ = [("info", POINTER(gameinfo)),
                ("board", POINTER(c_short)),
                ("hand", POINTER(c_short)),
                ("handval", POINTER(hand_hv)),
                ("plr", sd_plr*MAX_PLR),
                ("self_pos", c_uint),
                ("n_plr", c_uint)]

class traverse_plr_data(mystruct):
    _fields_ = [("hw", POINTER(c_double)),
                ("ev", POINTER(c_double)),
                ("path_level", POINTER(c_int)),
                ("pw", c_double)]
    
class situ(mystruct):
    pass

class situ_type(mystruct):
    pass

class situ_list(mystruct):
    pass



simu_data._fields_ = [("info", POINTER(gameinfo)),
                      ("board_slots", POINTER(slot)),
                      ("hand_slots", POINTER(POINTER(slot))),
                      ("handvals", POINTER(c_uint)),
                     
                      ("plr", sd_plr*10)]


                      #("update_mode", c_int*10),
                      #("update_every", c_int*10)]



plrmodel_node._fields_ = [("use_lock", c_uint32),
                          ("struct_lock", c_uint32),
                          ("t", POINTER(type_type)),
                          ("root", POINTER(unique_root)),
                          ("prev", POINTER(plrmodel_node)),
                          ("types_bmap", POINTER(c_uint64)),
                          ("type_mapping", POINTER(c_int16)),
                          ("next_list", POINTER(c_void_p)),
                          ("data_index", c_uint64),
                          ("data_format", c_uint32),
                          
                          ("pub_node_count", c_uint32),
                          ("id", c_uint32),
                          ("len", c_int16),
                          ("slot_in_prev", c_int16)]

unique_root._fields_ = [("root_idx", c_int32),
                        ("n_plr", c_int16),
                        ("to_act", c_int16),
                        ("gamestate", c_int32),
                        ("cur_seat", c_int32),
                        ("bets", POINTER(c_double)),
                        ("action_cost", c_double*3),
                        ("n_type_types", c_int32*2),
                        ("type_types_order", POINTER(c_int32)*2),
                        ("next", POINTER(unique_root)*3),
                        ("model_tree", POINTER(plrmodel_node)),
                        ("mutex_pool", c_void_p)]

situ._fields_ = [("max_levels", c_short),
                 ("n_hands", c_uint32),
                 ("hand_slots", POINTER(slot)),
                 ("hand_odds", POINTER(c_double)),
                 ("hd", POINTER(situ_hand_data)),
#                 ("hand_odds_avg", POINTER(c_double)),
#                 ("hand_regs", POINTER(c_double)),
#                 ("hand_ev", POINTER(c_double)),
#                 ("hand_path_level", POINTER(c_uint)),
#                 ("hand_visits", POINTER(c_uint)),
                 ("tmp_data", c_void_p),
                 ("parent", POINTER(plrmodel_node))]

situ_type._fields_ = [("average_potsize", slot),
                      ("potsize", slot),
                      ("path_action", slot*4),
                      ("path_bets", slot*4),
                      ("path_last_aggr", slot*4),
                      ("board", slot*4),
                      ("root_idx", c_int)]
                 

situ_list._fields_ = [("next", POINTER(situ_list)),
                      ("s", POINTER(plrmodel_node))]

class hand_info(mystruct):
    _fields_ = [("root_us", POINTER(unique_root)),
                ("cur_us", POINTER(unique_root)),
                ("acts_i", c_uint8*4),
                ("acts", c_uint8*4*32),
                ("hole_cards", c_int8*4),
                ("board", c_int8*5),
                ("padding", c_int8*3)]
                                                

class worker_thread_args(mystruct):
    _fields_ = [("in_queue", c_void_p),
                ("out_queue", c_void_p),
                ("us", POINTER(unique_root)),
                ("us_tree", c_void_p),
                ("wm_tree", c_void_p),
                ("td_tree", c_void_p),
                ("cont", c_int32)]

class receiver_thread_args(mystruct):
    _fields_ = [("out_queue", c_void_p),
                ("td_tree", c_void_p),
                ("listen_address", c_char_p),
                ("listen_port", c_int32),
                ("cont", c_int32)]


class sender_thread_args(mystruct):
    _fields_ = [("us_tree", c_void_p),
                ("in_queue", c_void_p),
                ("loopback_queue", c_void_p),
                ("cont", c_int32)]

class thread_control_args(mystruct):
    _fields_ = [("w_args", POINTER(worker_thread_args)),
                ("r_args", POINTER(receiver_thread_args)),
                ("s_args", POINTER(sender_thread_args)),
                ("receiver_to_worker_queue", c_void_p),
                ("worker_to_sender_queue", c_void_p),
                ("us_tree", c_void_p),
                ("td_tree", c_void_p),
                ("wm_tree", c_void_p),
                ("root_us", POINTER(unique_root)),
                ("worker_thread_id", POINTER(c_uint64)),
                ("sender_thread_id", c_uint64),
                ("receiver_thread_id", c_uint64),
                ("n_worker_thread", c_int32)]
    
class f3(mystruct):
    _fields_ = [("f", c_float*3)]


hand_hv_np = np.dtype([("hv", np.uint32),
                       ("hand", np.int16),
                       ("c1", np.uint8),
                       ("c2", np.uint8)])

hand_hv2_np = np.dtype([("hv", np.uint16),
                         ("sample_i", np.int16),
                         ("c", np.uint8, 4)])


hand_hv_river = np.dtype([("board_cards", np.int16),
                             ("board_slot", np.int16),
                             ("hand_slots", np.int16, defs.HANDS),
                             ("hand_hvs", hand_hv_np, defs.HANDS)])

hand_hv_turn = np.dtype([("board_cards", np.int16),
                         ("board_slot", np.int16),
                         ("hand_slots", np.int16, defs.HANDS),
                         ("river", hand_hv_river, 52)])

hand_hv_flop = np.dtype([("board_cards", np.int16),
                          ("board_slot", np.int16),
                          ("hand_slots", np.int16, defs.HANDS),
                          ("turn", hand_hv_turn, 52)])


#situ_hand_data_np = np.dtype( [("visits", np.uint32),
#                               ("path_level", np.uint32),
#                               ("regs", np.float64,3),
#                               ("avg_odds", np.float64,3),
#                               ("ev", np.float64,2)])
                              
variation_np = np.dtype([("path_weight", np.float64),
                          ("potsize", np.float64),
                          ("us_id", np.int32),
                          ("path_i", np.int32),
                          ("path", np.int8, defs.MAX_PATH_LEN)], align=True)

worker_message_np = np.dtype([("direction", np.int8),
                               ("action", np.int8),
                               ("pov_seat", np.int8),
                               ("n_variations", np.int8),
                               ("types_data_id", np.int32),
                               ("positive_regs", np.float64),
                               ("flags", np.uint64),
                               ("td_p", np.int64),
                               ("hwev_p", np.int64, defs.MAX_HWEV),
                               ("v_p", np.int64, defs.MAX_VARIATION)], align=True)

types_data_np = np.dtype([("timestamp", np.float64),
                           ("id", np.int32),
                           ("public_types", np.int16, defs.N_PUB_TYPES),
                           ("private_types", np.int16, (defs.N_PRIV_TYPES, defs.SAMPLES)),
                           ("vals", hand_hv2_np, defs.SAMPLES),
                           ("tc", np.uint64, defs.N_PRIV_TYPES),
                           ("n_types", np.int32, defs.N_PRIV_TYPES)], align=True)



hwev_data_np = np.dtype([("ref_count", np.int32),
                         ("padding", np.int32),
                         ("d", np.float64, defs.SAMPLES),
                         ("pw", np.float64)])


street_types_np = np.dtype([("board_type", np.int16),
                            ("hand_types", np.int16,defs.HANDS)], align=True)

class street_types(mystruct):
    _fields_ = [("board_type", c_int16),
                ("hand_types", c_int16*HANDS)]


class mem_list(mystruct):
    pass

mem_list._fields_ = [("next", POINTER(mem_list)),
                     ("mem", c_void_p)]
