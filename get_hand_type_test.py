import libzpoker
import sys


zp = libzpoker.libzpoker()

#t = zp.gen_types_for_flop(0, 40, 5, 2.0, 1.0)
gs = int(sys.argv[1])
zp.load_diffs(gs)
zp.pair_and_minimize_diffs(zp.diffs[gs], zp.diffs_order[gs])

#zp.i.n_types[gs] =  int(sys.argv[2])
#n_types_start = int(sys.argv[2])
#n_types_stop = int(sys.argv[3])

#zp.get_hand_types(gs)
