from ctypes import *

#DATATYPES
class wtl(Structure):
    _fields_ = [("w", c_short),
                ("t", c_short),
                ("l", c_short)]

class rcf(Structure):
    _fields_ = [("r", c_double),
                ("c", c_double),
                ("f", c_double)]
                



class gameinfo(Structure):
    _fields_ = [("types", (POINTER(c_double)*4)),
                ("all_flop_types_raw_fd", c_int),
                ("slots", (POINTER(c_short)*4)),
                ("gs_trans", (POINTER(c_double)*4)),
                ("n_types", (c_int*4)),
                ("scale", c_double),
                
                ("small_blind", c_double),
                ("big_blind", c_double),
                ("maxbets", c_double)]

class plrmodel_node(Structure):
    pass
plrmodel_node._fields_ = [("r",POINTER(plrmodel_node)), 
                          ("c",POINTER(plrmodel_node)),
                          ("p",POINTER(plrmodel_node)),
                          ("r_odds", c_int32),
                          ("c_odds", c_int32),
                          ("f_odds", c_int32),
                          ("r_ev", c_int32),
                          ("c_ev", c_int32),
                          ("potsize", c_double),
                          ("bets",c_double),
                          ("toc", c_double),
                          ("tor", c_double),
                          ("pos",c_int32),
                          ("gamestate", c_int32),
                          ("id", c_int32)
                          ]


class c_wt_change(Structure):
    _fields_ = [("t", POINTER(c_double)),
                ("s", POINTER(c_double)),
                ("o", POINTER(c_double)),
                ("n_items", c_int)]

class c_calc_ev(Structure):
    _fields_ = [("t", POINTER(c_double)),
                ("r_ev", POINTER(c_double)),
                ("c_ev", POINTER(c_double)),
                ("r_odds", POINTER(c_double)),
                ("c_odds", POINTER(c_double)),
                ("f_odds", POINTER(c_double)),
                ("cost", c_double),
                ("potsize", c_double),
                ("tlen", c_int)]

class c_calc_ev_over_gamestate_switch(Structure):
    _fields_ = [("t", POINTER(c_double)),
                ("r_ev", POINTER(c_double)),
                ("c_ev", POINTER(c_double)),
                ("r_odds", POINTER(c_double)),
                ("c_odds", POINTER(c_double)),
                ("f_odds", POINTER(c_double)),
                ("cost", c_double),
                ("potsize", c_double),
                ("tlen", c_int),
                ("slen", c_int),
                ("gs_trans_table", POINTER(c_double))]

class c_calc_showdown_ev(Structure):
    _fields_ = [("t", POINTER(c_double)),
                ("wt", POINTER(c_double)),
                ("potsize", c_double),
                ("sd_wtl_table", POINTER(c_double)),
                ("tlen", c_int)]

class c_gamestate_switch(Structure):
    _fields_ = [("t", POINTER(c_double)),
                ("s", POINTER(c_double)),
                ("t_len", c_int),
                ("s_len", c_int),
                ("gs_trans_table", POINTER(c_double))]

class c_gamestate_switch_and_mult(Structure):
    _fields_ = [("t", POINTER(c_double)),
                ("s", POINTER(c_double)),
                ("m", POINTER(c_double)),
                ("t_len", c_int),
                ("s_len", c_int),
                ("gs_trans_table", POINTER(c_double))]


class c_update_rcf(Structure):
    _fields_ = [("r_odds", POINTER(c_double)),
                ("c_odds", POINTER(c_double)),
                ("f_odds", POINTER(c_double)),
                ("r_ev", POINTER(c_double)),
                ("c_ev", POINTER(c_double)),
                ("n_types", c_int)]

class c_update_rcf_estimate(Structure):
    _fields_ = [("r_odds", POINTER(c_double)),
                ("c_odds", POINTER(c_double)),
                ("f_odds", POINTER(c_double)),
                ("r_est", POINTER(c_double)),
                ("c_est", POINTER(c_double)),
                ("f_est", POINTER(c_double)),
                ("wt", POINTER(c_double)),
                ("n_types", c_int)]
