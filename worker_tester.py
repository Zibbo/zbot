import plrmodel
import datatypes as dt
import numpy as np
from defs import *
import socket
import time
import random
import sys

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(("", 0))
p = plrmodel.plrmodel("testipeli")

#hand_data = p.zp.get_random_traverse_jbs()
#for x in hand_data:
#    print x

#us = p.root_state.unique_root_node.next[0].contents.next[0].contents
us = p.root_state.next_states[0].next_states[0]

print us.state
print us.unique_root_node.bets[0]
print us.unique_root_node.bets[1]


wm = np.zeros((1,), dtype = dt.worker_message_np)[0]
td = np.zeros((1,), dtype = dt.types_data_np)[0]
hw = np.empty((SAMPLES,), dtype=np.float32)
hw.fill(1.0/SAMPLES)
v = np.zeros((1,), dtype = dt.variation_np)[0]
pw = np.zeros((1,), dtype = np.float64)


time.sleep(2)
start_time = time.time()
tot_len = 0
for x in xrange(int(sys.argv[1]), int(sys.argv[1]) + int(sys.argv[2])):
    hand_data = p.zp.get_random_traverse_jbs()
    #print v
    v["us_id"] = us.id
    v["potsize"] = 1.5
    v["path_i"] = 2

#print td
#for x in xrange(4):
    
    wm["direction"] = 0
    wm["pov_seat"] = 0
    wm["n_hwev"] = 1
    wm["n_variations"] = 1
    wm["td_p"] = 1
    wm["hwev_p"][1] = 1
    #print wm
    #print hw
    td["id"] = x #random.randint(0,1000000)
    td["private_types"][:] = hand_data[1][:]
    td["vals"][:] = hand_data[2][:]
    td["public_types"][7:10] = hand_data[0][1:4]
    #print td["public_types"]
    
    



    #time.sleep(0.00001)
    len = s.sendto(wm.tostring() + td.tostring() + pw.tostring() + hw.tostring() + v.tostring(), ("", 11113))
    tot_len += len
    while tot_len/(time.time()-start_time) > 150000000:
        time.sleep(0.000001)
    if not x%1000:
#        time.sleep(0.001)
        print x, len

print tot_len/(time.time()-start_time)
