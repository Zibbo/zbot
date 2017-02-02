import datatypes as dt
import lzp
import ctypes as C
import plrmodel
import sys
import time
import pdb
import socket
import select
import struct
import defs
import os
import Queue
import threading
import numpy as np
import card_table as ct
import pid

import worker2


gamedir = sys.argv[1]
model_name = sys.argv[2]
n_workers = int(sys.argv[3])

target_hand_count = int(sys.argv[4])
target_hands_per_priv = int(sys.argv[5])

if len(sys.argv) > 6:
    adjust = [float(a) for a in sys.argv[6:]]
else:
    adjust = None
p = plrmodel.plrmodel(gamedir)
p.reset_model_name(model_name)

#p.model_name = model_name
p.zp.load_slots_for_all_types()
p.zp.load_diffs_for_all_types()
#p.expand_models_test()
#p.expand_models_until_n_hands_random(10000, 0)
#    pdb.set_trace()

p.gen_plrmodel_tree_for_all_us()
for us in p.iter_all_unique_states():
    if us.gamestate < 4 and us.id >= 2:
        print "expanding", us.state
        if us.gamestate == 0:
            us.expand_type_max(0,us.gamestate)
        us.expand_type_max(1,6)
        us.expand_type_max(1,5)        
        us.expand_type_max(1,4)
        us.expand_type_max(1,3)
        us.expand_type_max(1,2)
        us.expand_type_max(1,1)

p.reset_regs()
if target_hands_per_priv > 0:
    wc = worker2.worker_control(p, target_hand_count, target_hand_count/target_hands_per_priv, 0.01, 0.01, 0.01, 0.01)
else:
    wc = worker2.worker_control(p, target_hand_count, 0, 0, 0.01, 0, 0.001)
if adjust != None:
    wc.stake_adjust[0,0] = adjust[0]
    wc.stake_adjust[0,1] = adjust[1]
    wc.stake_adjust[1,0] = adjust[2]
    wc.stake_adjust[1,1] = adjust[3]
#wc = worker2.worker_control(p, target_hand_count,0, 1.0, 0, 0.001)
wc.set_worker_count(n_workers)
wc.start_workers()

prev_iter_count = 0
prev_iter_time = time.time()
while True:
    sel_ret = select.select([sys.stdin], [],[],5.0)

    if sys.stdin in sel_ret[0]:
        inp = raw_input()
        if inp == "debug":
            pdb.set_trace()
        elif inp == "quit":
            break
    iters = wc.get_iter_count()
    print "iters per second", (iters-prev_iter_count)/(time.time()-prev_iter_time)

    prev_iter_count = iters
    prev_iter_time = time.time()

