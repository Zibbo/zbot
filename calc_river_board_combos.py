import sys
import numpy as np
import ctypes as C
import pdb
import random


import zpoker
import datatypes as dt
import lzp

zp = zpoker.zpoker("biggame")


zp.load_types_for_all_types()
zp.load_diffs_for_all_types()
zp.load_slots_for_all_types()
zp.set_info_pointers()

b_diffs = [None]*4
max_diff = np.zeros(4, dtype=np.float64)
for i in xrange(1,4):
    b_diffs[i] = np.ctypeslib.as_array(zp.i.b_diffs[i], shape= (1024,1024))
    b_diffs[i][np.where(b_diffs[i] > 100000000)] = 0
    max_diff[i] = b_diffs[i].max()



hi = dt.hand_info()
pub_types = np.zeros(zp.i.n_type_types[1], dtype=np.int16)
pub_types.fill(0)
#test_pub_types = np.zeros(p.zp.i.n_type_types[1], dtype=np.int16)
priv_types = np.zeros(zp.i.n_type_types[0], dtype=np.int16)
priv_types.fill(0)

combo_dict = {}

random.seed(1234)

def rnd_func():
    c = random.sample(range(52), 5)
    hi.board[0] = c[0]
    hi.board[1] = c[1]
    hi.board[2] = c[2]
    hi.board[3] = c[3]
    hi.board[4] = c[4]
    pub_types[7:10] = -1
    lzp.set_st_from_hand_info(C.pointer(zp.i), C.pointer(hi), pub_types.ctypes.data_as(C.POINTER(C.c_int16)),priv_types.ctypes.data_as(C.POINTER(C.c_int16)))
    return np.array((pub_types[9],pub_types[8], pub_types[7]), dtype=np.int16)

def dist_func1(t1,t2):
    gs = len(t1)
    tot_diff = 0.0
    print gs, t1,t2
    for i in xrange(gs):
        tot_diff += b_diffs[i+1][t1[i],t2[i]]/max_diff[i+1]
        
    return tot_diff

def dist_func2(types, new_type, diffs):
    #pdb.set_trace()
    diffs[:] = b_diffs[1][types[:,0],new_type[0]]/max_diff[1]+b_diffs[2][types[:,1],new_type[1]]/max_diff[2] + b_diffs[3][types[:,2],new_type[2]]/max_diff[3]
    
    #zp.get_group_with_biggest_diffs(2**14, (3,), rnd_func, dist_func1, True, diffs_unit=np.float64)
diffs, diffs_order, types = zp.get_group_with_biggest_diffs(2**14, (3,), rnd_func, dist_func2, False)

diffs.tofile("river_board.diffs")
types.tofile("river_board.types")

sys.exit()
hi.board[0] = 51
while hi.board[0] >= 0:
    hi.board[1] = hi.board[0]-1
    while hi.board[1] >= 0:
        hi.board[2] = hi.board[1]-1
        while hi.board[2] >= 0:
            pub_types[9] = -1
            hi.board[3] = hi.board[2]-1
            while hi.board[3] >= 0:
                pub_types[8] = -1
                hi.board[4] = hi.board[3]-1    
                while hi.board[4] >= 0:
                    pub_types[7] = -1
                    lzp.set_st_from_hand_info(C.pointer(zp.i), C.pointer(hi), pub_types.ctypes.data_as(C.POINTER(C.c_int16)),priv_types.ctypes.data_as(C.POINTER(C.c_int16)))
                    #print zp.get_board_slots_all_gs([i1,i2,i3,i4,i5])
                    #print pub_types
                    t = (pub_types[9],pub_types[8], pub_types[7])
                    try:
                        combo_dict[t] += 1
                    except KeyError:
                        combo_dict[t] = 1
                    hi.board[4]-=1
                hi.board[3]-=1
            hi.board[2]-=1
        hi.board[1]-=1
        print sum(combo_dict.values())
        print len(combo_dict)
    hi.board[0]-=1
