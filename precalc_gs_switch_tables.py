import libzpoker
import sys
import random

gamedir = sys.argv[1]
zp = libzpoker.libzpoker(gamedir)


for x in range(4):
    if x != 3:
        zp.load_hand_slots(x)
    zp.load_hand_types(x)
    zp.load_hand_diffs(x)
    if x != 0:
        zp.load_board_types(x)


zp.precalc_gs_switch_and_odds_tables()

for x in range(4):
    if x != 0:
        zp.save_gs_hand_switch_tables(x)
        zp.save_gs_board_switch_tables(x)
        zp.save_gs_board_odds_tables(x)
    zp.save_gs_hand_odds_tables(x)
