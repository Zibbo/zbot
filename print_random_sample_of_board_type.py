import libzpoker
import sys
import matplotlib.pyplot as plt
import random
import card_table as ct
import time
import numpy

gamedir = sys.argv[1]
zp = libzpoker.libzpoker(gamedir)

gs = int(sys.argv[2])



b_types = int(sys.argv[3])

type_n = int(sys.argv[4])
type_n_stop = int(sys.argv[5])
n_types_start = int(sys.argv[6])
n_types_stop = int(sys.argv[7])

zp.i.n_types[gs-1] = n_types_start
zp.i.n_types[gs] = n_types_stop

zp.i.n_btypes[gs] = b_types
zp.load_board_types(gs)
zp.load_board_diffs(gs)

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

t1 = numpy.zeros((zp.i.n_htfb[gs-1], zp.i.n_htfb[gs]), dtype = numpy.float64)
i = 0
while True:
    b = random.sample(range(52), 5)
    flop_i = zp.ctoi3[b[0]][b[1]][b[2]]
    turn_i = b[3]
    river_i = b[4]

    t1 = zp.gen_board_type(gs, flop_i, turn_i, river_i)
    s = zp.get_slot_new(zp.i.b_types[gs], t1.ctypes.data, len(zp.b_types[gs]), t1.size)
    if s >= type_n and s <= type_n_stop:
        for x in b[:gs+2]:
            print ct.itos[x],
        print s,i
    i+=1
