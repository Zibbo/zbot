import sys
import numpy as np
import pdb

import plrmodel
import card_table as ct
import lzp
import ctypes as C
import defs
import datatypes as dt
# def np_reduce(a, mapping):
#     s = np.argsort(mapping)
#     start = 0
#     cur = 0
#     while cur < len(s):

#         if cur+1 >= len(s) or mapping[s[cur]] != mapping[s[cur+1]]:
#             a[s[start:cur+1]] = np.average(a[s[start:cur+1]])
#             start = cur+1
#         cur += 1


# def calc_ev(us, path, solution_path):
#     full_hw = np.zeros((2,defs.HANDS), dtype=np.float64)
#     full_ev = np.zeros((3,2,defs.HANDS), dtype=np.float64)
#     hw = np.zeros((2,169), dtype=np.float64)
#     ev = np.zeros((3,2,169), dtype=np.float64)
#     data_sum = np.zeros((8,defs.HANDS), dtype=np.float64)
#     data_reduce = np.zeros((8,169), dtype=np.float64)
#     to_down_ev = np.zeros((2,169), dtype=np.float64)
    
#     if us.next[0] != None:
#         ev[0] = calc_ev(us.next[0], path+"0", solution_path)
#     if us.next[1].gamestate == 0:
#         ev[1] = calc_ev(us.next[1], path+"1", solution_path)
#     else:
#         assert(us.next[1].gamestate == 1)
#         pov_seat = us.next[1].cur_seat;
#         nonpov_seat = (us.next[1].cur_seat+1)%2;
            
#         for flop in ct.morph_flops.keys():
#             filename = solution_path+flop+"/_"+path+"1"
#             data = np.fromfile(filename, dtype = np.float64).reshape(-1,defs.HANDS)
#             new_hw = data[:2]
#             new_ev = data[2:].reshape(3,2,defs.HANDS)
#             pov_ev = new_ev[pov_seat]
#             nonpov_ev = new_ev[nonpov_seat]
#             for act_i in xrange(3):
#                 ev[1][nonpov_seat] += new_ev[act_i][nonpov_seat]
#             ev[1][pov_seat] = new_ev[0][pov_seat]*(new_ev[0][pov_seat]>new_ev[1][pov_seat]) + new_ev[1][pov_seat]*(new_ev[1][pov_seat]>=new_ev[0][pov_seat])
#             ev[1][pov_seat] = ev[1][pov_seat]*(ev[1][pov_seat]>new_ev[2][pov_seat]) + new_ev[2][pov_seat]*(new_ev[2][pov_seat]>=ev[1][pov_seat])
            
#             #full_hw += new_hw
#             #full_ev += new_ev
#         np_reduce(ev[1][pov_seat], ct.preflop_morph_mapping)
#         np_reduce(ev[1][nonpov_seat], ct.preflop_morph_mapping)


def flop_solver():
    rng = lzp.get_rng()
    start_us = C.pointer(p.root_state.next_states[0].next_states[0].unique_root_node)
    start_hw = np.ones((2,defs.HANDS), dtype=np.float64)/1225.0
    start_hw_p = start_hw.ctypes.data_as(C.POINTER(C.c_double))

    start_stake = np.array((1.0, 0.5), dtype=np.float64)
    start_stake_p = start_stake.ctypes.data_as(C.POINTER(C.c_double))
    flags = np.zeros((2), dtype=np.uint64)
    flags |= defs.BR_LOAD_FLOP_DATA
    #flags |= defs.UPDATE_REGS
    #flags |= defs.UPDATE_AVG_ODDS
    path = np.zeros(256, dtype = np.int8)

    preflop_types = np.zeros(1, dtype = dt.street_types_np)
    preflop_types["hand_types"] = p.zp.slots[0]
    preflop_types_p = preflop_types.ctypes.data_as(C.POINTER(dt.street_types))
        
    pub_types = np.zeros(p.zp.i.n_type_types[1], dtype=np.int16)
    pub_types.fill(-1)
    pub_types[0] = 0
    pub_types_p = pub_types.ctypes.data_as(C.POINTER(C.c_int16))
    priv_types_p = preflop_types[0]["hand_types"].ctypes.data_as(C.POINTER(C.c_int16))
    ret_ev = np.zeros((2,defs.HANDS), dtype=np.float64)
    ret_ev_p = ret_ev.ctypes.data_as(C.POINTER(C.c_double))
    lzp.br_solve(start_us, path.ctypes.data_as(C.c_char_p), 0, 1, -1, -1, -1, preflop_types_p, None, None, None, None, pub_types_p, priv_types_p, ret_ev_p, start_hw_p, start_stake_p, flags.ctypes.data_as(C.POINTER(C.c_uint64)), rng, save_path)
    print ret_ev[0]
    print ret_ev[1]
    print (ret_ev[0]).sum()
    print (ret_ev[1]).sum()
    print np.average(ret_ev[0])
    print np.average(ret_ev[1])
    print (np.average(ret_ev[0]) + np.average(ret_ev[1]))/2.0
    pdb.set_trace()
        
# if __name__ == "__main__":
#     a = np.arange(100)
#     b = np.random.randint(10, size=100)
#     print a
#     print b
#     np_reduce(a,b,None)
#     print a
        
gamedir = sys.argv[1]
plrmodel_path = sys.argv[2]

solution_path = plrmodel_path+"solution/"

p = plrmodel.plrmodel(gamedir)
p.zp.load_hand_slots(0)
p.zp.load_diffs_for_all_types()

p.load_model(plrmodel_path, gamestates = (0,))

save_path = plrmodel_path + "solution/"

flop_solver()
