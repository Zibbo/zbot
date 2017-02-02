import zpoker
import sys

gamedir = sys.argv[1]
zp = zpoker.zpoker(gamedir)

#t = zp.gen_types_for_flop(0, 40, 5, 2.0, 1.0)
gs = int(sys.argv[2])
slots = int(sys.argv[3])

if gs < 3:
    zp.i.n_types[3] = zp.i.n_rtypes[gs]
    zp.load_hand_types(3)
    zp.load_hand_diffs(3)

zp.i.n_types[gs] = slots

zp.get_hand_types(gs)

zp.save_hand_types(gs)
zp.save_hand_diffs(gs)
