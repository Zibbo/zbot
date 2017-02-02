import zpoker
import sys
import numpy
import random
import zlib
import select
from ctypes import *

import datatypes as dt
import lzp
def random_board():           
    return random.sample(range(52), 5)

def random_flop():
    return random.sample(range(52), 3)

gamedir = sys.argv[1]
jbs_data_filename = sys.argv[2]+".data"
jbs_offset_filename = sys.argv[2]+".offset"

f = file(jbs_data_filename, "ab")
off_f = file(jbs_offset_filename, "ab")

zp = zpoker.zpoker(gamedir)

for x in range(4):
    if x != 3:
        zp.load_hand_slots(x)
    zp.load_hand_types(x)
    zp.load_hand_diffs(x)
    if x != 0:
        zp.load_board_types(x)

header = numpy.zeros((2), dtype=numpy.int64)

cont = True
i = 0
# hand_hv_type = numpy.dtype([("hv", numpy.uint32),
#                             ("hand", numpy.int16),
#                             ("c1", numpy.uint8),
#                             ("c2", numpy.uint8)])

# hand_hv_river_type = numpy.dtype([("board_cards", numpy.int16),
#                                   ("board_slot", numpy.int16),
#                                   ("hand_slots", numpy.int16, zp.HANDS),
#                                   ("hand_hvs", hand_hv_type, zp.HANDS)])

# hand_hv_turn_type = numpy.dtype([("board_cards", numpy.int16),
#                                  ("board_slot", numpy.int16),
#                                  ("hand_slots", numpy.int16, zp.HANDS),
#                                  ("hand_hvs", hand_hv_type, zp.HANDS),
#                                  ("river", hand_hv_river_type, 52)])

# hand_hv_flop_type = numpy.dtype([("board_cards", numpy.int16),
#                                  ("board_slot", numpy.int16),
#                                  ("hand_slots", numpy.int16, zp.HANDS),
#                                  ("hand_hvs", hand_hv_type, zp.HANDS),
#                                  ("turn", hand_hv_turn_type, 52)])

flop_data_tmp = numpy.zeros((1), dtype=dt.hand_hv_flop)
flop_data = flop_data_tmp[0]
print flop_data.itemsize


while cont:
    i+=1
    if select.select([sys.stdin,],[],[],0.0)[0]:
        print "getting input"
        inp = raw_input()
        print "input:", inp
        if inp == "break":
            cont = False
        else:
            print "unidentified input", inp
    flop = random_flop()
    flop_data["board_cards"] = zp.ctoi3[flop[0]][flop[1]][flop[2]]
    flop_data["board_slot"] = zp.get_board_slot_flop(flop)
    flop_data["hand_slots"][:] = zp.get_all_hand_slots_flop(flop)
    
    #    board_slots[1] = zp.get_board_slot_flop(flop)
    #    hand_slots[1] = zp.get_all_hand_slots_flop(flop)
    for tc in xrange(52):
        turn_data = flop_data["turn"][tc]
        if tc not in flop:
            turn_data["board_cards"] = tc
            turn_data["board_slot"] = zp.get_board_slot_turn(flop+[tc])
            turn_data["hand_slots"][:] = zp.get_all_hand_slots_turn(flop+[tc])
        else:
            turn_data["board_cards"] = tc
            turn_data["board_slot"] = -1
            turn_data["hand_slots"].fill(-1)
        print flop, tc
        for rc in xrange(52):
            river_data = turn_data["river"][rc]
            board = flop + [tc] + [rc]
            river_data["board_cards"] = rc
            if len(set(board)) == 5:
                river_data["board_slot"] = zp.get_board_slot_river(board)
                zp.get_all_hand_slots_river(board, river_data["hand_slots"])
                #river_data["hand_slots"] = zp.get_all_hand_slots_river(board)
                #print river_data["hand_hvs"]
                lzp.lib.get_river_hand_hv_all(lzp.ctoi3[board[0]][board[1]][board[2]], tc, rc, river_data["hand_hvs"].ctypes.data_as(POINTER(dt.hand_hv)))
                #print river_data["hand_hvs"]
                #sys.exit()
            else:
            
                river_data["board_slot"] = -1
                river_data["hand_slots"].fill(-1)
                river_data["hand_hvs"].fill((0,-1,-1,-1))
                
    print i, board
    new_data = zlib.compress(flop_data.tostring())
    header[0] = f.tell()
    header[1] = len(new_data)

    #print len(combined_cmp)
    f.write(new_data)
    header.tofile(off_f)
    if i >= 500:
        cont = False

f.close()
off_f.close()
