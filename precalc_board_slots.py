import zpoker
import sys

gamedir = sys.argv[1]
zp = zpoker.zpoker(gamedir)

#t = zp.gen_types_for_flop(0, 40, 5, 2.0, 1.0)
gs = int(sys.argv[2])

n_types_start = int(sys.argv[3])
n_types_stop = int(sys.argv[4])

b_types = int(sys.argv[5])

zp.i.n_types[gs-1] = n_types_start
zp.i.n_types[gs] = n_types_stop

zp.i.n_btypes[gs] = b_types

if gs == 3:
    zp.load_hand_types(gs)

if not zp.load_hand_diffs(gs):
    sys.exit(0)
if not zp.load_hand_diffs(gs-1):
    sys.exit(0)
if not zp.load_hand_slots(gs):
    sys.exit(0)
if not zp.load_hand_slots(gs-1):
    sys.exit(0)

zp.load_board_types(gs)

zp.precalc_board_slots(gs)
