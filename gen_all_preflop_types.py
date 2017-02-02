import libzpoker
import sys

gamedir = sys.argv[1]

zp = libzpoker.libzpoker(gamedir)

zp.i.n_types[3] = zp.i.n_rtypes[0]

zp.load_hand_types(3)
zp.load_hand_diffs(3)

zp.gen_all_preflop_types()
