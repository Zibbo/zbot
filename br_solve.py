import sys
import numpy as np
import ctypes as C
import random
import Queue
import threading
import os
import pdb

import lzp
import defs
import plrmodel
import datatypes as dt
import card_table as ct

def load_hand_data_for_flop(flop_i):
    prec_dir = "/Volumes/store/types_prec/"
    #prec_dir = "biggame/types_prec/"
    flop_types = np.fromfile(prec_dir+str(flop_i)+".flop_types", dtype = dt.street_types_np)
    turn_types = np.fromfile(prec_dir+str(flop_i)+".turn_types", dtype = dt.street_types_np)
    river_types = np.fromfile(prec_dir+str(flop_i)+".river_types", dtype = dt.street_types_np).reshape((52,52))
    river_handvals = np.fromfile(prec_dir+str(flop_i)+".river_handvals", dtype = dt.hand_hv2_np).reshape((52,52,defs.HANDS))

    return (flop_types, turn_types, river_types, river_handvals)

def get_morph_flop(c):
    if c[0]/13 == c[1]/13 == c[2]/13:
        flop = ct.ctoi3[c[0]%13,c[1]%13,c[2]%13]
    elif c[0]/13 != c[1]/13 and c[0]/13 != c[2]/13 and c[1]/13 != c[2]/13:
        #print c
        tmpc = np.sort(c%13)
        #print tmpc
        #tmpc = tmpc%13
        #print tmpc
        tmpc[1]+=13
        tmpc[2]+=26
        #print tmpc
        #print
        flop = ct.ctoi3[tmpc[0], tmpc[1], tmpc[2]]
    else:
        if (c[0]/13 == c[1]/13):
            flop = ct.ctoi3[c[0]%13, c[1]%13, c[2]%13+13]
        elif c[0]/13 == c[2]/13:
            flop = ct.ctoi3[c[0]%13, c[2]%13, c[1]%13+13]
        else:
            flop = ct.ctoi3[c[1]%13, c[2]%13, c[0]%13+13]


    return flop


def flop_solver(flop_queue):
    rng = lzp.get_rng()
    start_us = C.pointer(p.root_state.next_states[0].next_states[0].unique_root_node)
    start_hw = np.ones((2,defs.HANDS), dtype=np.float64)/1225.0
    #start_hw = np.ones((2,defs.HANDS), dtype=np.float64)/1326.0
    start_hw_p = start_hw.ctypes.data_as(C.POINTER(C.c_double))

    start_stake = np.array((1.0, 0.5), dtype=np.float64)
    start_stake_p = start_stake.ctypes.data_as(C.POINTER(C.c_double))
    flags = np.zeros((2), dtype=np.uint64)
    flags |= defs.BR_SAVE_FLOP_DATA
    #flags |= defs.UPDATE_REGS
    #flags |= defs.UPDATE_AVG_ODDS
    path = np.zeros(256, dtype = np.int8)

    while True:
        try:
            flop = flop_queue.get(block=False)
        except:
            print "no flops, breaking"
            break
        flop_i = lzp.ctoi3[flop[0]][flop[1]][flop[2]]

        try:
            os.mkdir(save_path+str(flop_i))
        except OSError:
            if sys.exc_info()[1].strerror == "File exists":
                if len(os.listdir(save_path+str(flop_i))) >= 70:
                    print flop_i, "already done"
                    continue
            pass
        
        
        flop_types, turn_types, river_types, river_handvals = load_hand_data_for_flop(flop_i)
        preflop_types = np.zeros(1, dtype = dt.street_types_np)
        preflop_types["hand_types"] = p.zp.slots[0]

        preflop_types_p = preflop_types.ctypes.data_as(C.POINTER(dt.street_types))
        flop_types_p = flop_types.ctypes.data_as(C.POINTER(dt.street_types))
        turn_types_p = turn_types.ctypes.data_as(C.POINTER(dt.street_types))
        river_types_p = river_types.ctypes.data_as(C.POINTER(dt.street_types))
        river_handvals_p = river_handvals.ctypes.data_as(C.POINTER(dt.hand_hv2))
        
        #flop_types_p = None
        #turn_types_p = None
        #river_types_p = None
        #river_handvals_p = None

        pub_types = np.zeros(p.zp.i.n_type_types[1], dtype=np.int16)
        pub_types.fill(-1)
        pub_types[0] = 0
        pub_types_p = pub_types.ctypes.data_as(C.POINTER(C.c_int16))
        priv_types_p = preflop_types[0]["hand_types"].ctypes.data_as(C.POINTER(C.c_int16))
        ret_ev = np.zeros((2,defs.HANDS), dtype=np.float64)
        ret_ev_p = ret_ev.ctypes.data_as(C.POINTER(C.c_double))
        retval = lzp.br_solve(start_us, path.ctypes.data_as(C.c_char_p), 0, 1, flop_i, -1, -1, preflop_types_p, flop_types_p, turn_types_p, river_types_p, river_handvals_p, pub_types_p, priv_types_p, ret_ev_p, start_hw_p, start_stake_p, flags.ctypes.data_as(C.POINTER(C.c_uint64)), rng, save_path)
        print "retval", retval
        #pdb.set_trace()
gamedir = sys.argv[1]
plrmodel_path = sys.argv[2]

if plrmodel_path[-1] != "/":
    plrmodel_path += "/"

p = plrmodel.plrmodel(gamedir)
p.zp.load_hand_slots(0)
p.zp.load_diffs_for_all_types()

p.load_model(plrmodel_path)

save_path = plrmodel_path + "solution/"
try:
    os.mkdir(save_path)
except OSError:
    pass

# morph_flops = {}
# i1 = 51
# while i1 >= 0:
#     i2 = i1-1
#     while i2 >= 0:
#         i3 = i2-1
#         while i3 >= 0:
#             flop = get_morph_flop(np.array((i1,i2,i3)))
#             try:
#                 morph_flops[flop] += 1
#             except KeyError:
#                 morph_flops[flop] = 1
#             i3-=1
#         i2-=1
#     i1-=1

flops_list = ct.morph_flops.keys()
flops_list.sort()

random.seed(1337)
random.shuffle(flops_list)


flop_queue = Queue.Queue()

workers = []

#save_path = sys.argv[2]
n_workers = int(sys.argv[5])

start_flop = int(sys.argv[3])
stop_flop = int(sys.argv[4])

for flop in flops_list[start_flop:stop_flop]:
    flop_queue.put(ct.itoc3[flop])


for i in xrange(n_workers):
    new_worker = threading.Thread(target=flop_solver, args=(flop_queue,))
    new_worker.daemon = True
    new_worker.start()
    workers.append(new_worker)
    
for w in workers:
    w.join()

sys.exit()
