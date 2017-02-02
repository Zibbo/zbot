import sys
import random
import copy
import numpy
import os

import card_table as ct
import libzpoker


gamedir = sys.argv[1]        
zp = libzpoker.libzpoker(gamedir)








#zp.i.wl_slots = int(sys.argv[1])
#zp.i.tie_slots = int(sys.argv[2])
zp.i.n_types[3] = zp.i.n_rtypes[1]
zp.load_hand_types(3)
zp.load_hand_diffs(3)

#path = "flop_types_raw_wl" + str(wl_slots) + "_tie"+ str(tie_slots) + "_wlscale" + str(type.wl_scale) + "_tiescale" + str(type.tie_scale) + "/"
path = gamedir + "flop_types_raw/"

if not os.path.isdir(path):
    os.makedirs(path)

#f = file(filename, "wb")

for x in range(22100):
    if not os.path.isfile(path+str(x)+".ftype"):
        t = zp.gen_types_for_flop(x, False)
        t.tofile(path+str(x)+".ftype")
    print x
#f.close()

