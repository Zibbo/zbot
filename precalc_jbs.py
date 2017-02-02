import lzp
import zpoker
import sys
import numpy
import random
import zlib
import select
from ctypes import *

def random_board():           
    return random.sample(range(52), 5)


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
hand_values = numpy.zeros((zp.HANDS), dtype=numpy.uint32)
cont = True
i = 0
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
    board = random_board()
    if not i%1000:
        print i, board
    board_jbs = numpy.array(zp.get_board_slots_all_gs(board), dtype=numpy.int16)
    hand_slots = zp.get_all_hand_slots_all_gs(board)
    lzp.lib.get_river_handhv_all(zp.ctoi3[board[0]][board[1]][board[2]], board[3], board[4], hand_values.ctypes.data_as(POINTER(c_uint32)))
    hand_jbs = numpy.vstack((hand_slots[0], hand_slots[1], hand_slots[2], hand_slots[3])).transpose()
    board_jbs_str = board_jbs.tostring()
#    for x in xrange(zp.HANDS):
#        hand_jbs_str += hand_jbs[x].tostring()
#        hand_jbs_str += hand_values[x].tostring()
    combined_cmp = zlib.compress(board_jbs_str+hand_jbs.tostring()+hand_values.tostring())
    
    header[0] = f.tell()
    header[1] = len(combined_cmp)
    
    #print len(combined_cmp)
    f.write(combined_cmp)
    header.tofile(off_f)


f.close()
off_f.close()
