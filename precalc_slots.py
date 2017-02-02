import zpoker
import sys

gamedir = sys.argv[1]
gs = int(sys.argv[2])

    
zp = zpoker.zpoker(gamedir)
try:
    zp.i.n_types[gs] = int(sys.argv[3])
except:
    pass
#zp.i.wl_slots = 64
#zp.i.tie_slots = 4
#zp.i.wl_scale = 2.0
#zp.i.tie_scale = 1.0
#zp.i.wl_width = 15.0
#zp.i.tie_width = 5.0

#zp.i.n_types[0] = 169
#zp.i.n_types[1] = 8192
#zp.i.n_types[2] = 8192
#zp.i.n_types[3] = zp.i.n_rtypes[gs]
if gs != 3:
    zp.load_hand_types(3)
    zp.load_hand_diffs(3)
zp.load_hand_types(gs)
zp.load_hand_diffs(gs)
print "types loaded"
zp.precalc_and_save_slots(gs)
