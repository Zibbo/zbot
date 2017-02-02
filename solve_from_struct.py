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
n_iters = int(sys.argv[4])

adjust = [float(a) for a in sys.argv[5:9]]
    
p = plrmodel.plrmodel(gamedir)
p.reset_model_path(model_name)
p.load_struct()
p.recount_data_index()


#p.mmap_regs()
#p.mmap_avg_odds()
#p.set_regs_from_mmap()
#p.set_avg_odds_from_mmap()
p.zp.load_slots_for_all_types()
p.zp.load_diffs_for_all_types()

if (len(sys.argv) > 9 and sys.argv[9] == "reset"):
    p.reset_regs()
    p.reset_avg_odds()
else:
    p.load_regs()
    p.load_avg_odds()

wc = worker2.worker_control(p, 0,0)
if adjust != None:
    wc.stake_adjust[0,0] = adjust[0]
    wc.stake_adjust[0,1] = adjust[1]
    wc.stake_adjust[1,0] = adjust[2]
    wc.stake_adjust[1,1] = adjust[3]
    
wc.set_worker_count(n_workers)
wc.start_workers()

iters = 0
prev_iter_count = 0
prev_iter_time = time.time()
while iters < n_iters:
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
wc.stop_workers()
time.sleep(5)
print "saving regs"
p.save_regs()
print "saving avg"
p.save_avg_odds()
print "saving byte"
p.save_byte_odds(source="AVG")
