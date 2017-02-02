import lzp
import zpoker
import sys
import numpy as np
import random
import zlib
import select
from ctypes import *
import datatypes as dt
import Queue
import defs
import card_table as ct
import threading

def random_board():           
    return random.sample(range(52), 5)

def get_morph_flop(c):
    if c[0]/13 == c[1]/13 == c[2]/13:
        flop = ct.ctoi3[c[0]%13,c[1]%13,c[2]%13]
    elif c[0]/13 != c[1]/13 and c[0]/13 != c[2]/13 and c[1]/13 != c[2]/13:
        #print c
        tmpc = np.sort(c%13)
        #print tmpc
        #tmpc = tmpc%13
        #print tmpc
        tmpc[1]+=13
        tmpc[2]+=26
        #print tmpc
        #print
        flop = ct.ctoi3[tmpc[0], tmpc[1], tmpc[2]]
    else:
        if (c[0]/13 == c[1]/13):
            flop = ct.ctoi3[c[0]%13, c[1]%13, c[2]%13+13]
        elif c[0]/13 == c[2]/13:
            flop = ct.ctoi3[c[0]%13, c[2]%13, c[1]%13+13]
        else:
            flop = ct.ctoi3[c[1]%13, c[2]%13, c[0]%13+13]


    return flop

def calc_flop(work_queue, save_path):
    flop_types = np.zeros(1, dtype = dt.street_types_np)
    turn_types = np.zeros(52, dtype = dt.street_types_np)
    river_types = np.zeros((52,52), dtype = dt.street_types_np)
    river_handvals = np.zeros((52,52,defs.HANDS), dtype = dt.hand_hv2_np)
    
    while True:
        flop_types["board_type"].fill(-1)
        flop_types["hand_types"].fill(-1)
        #print flop_types
        turn_types["board_type"].fill(-1)
        turn_types["hand_types"].fill(-1)
        #print turn_types
        river_types["board_type"].fill(-1)
        river_types["hand_types"].fill(-1)

        river_handvals["hv"].fill(-1)
        river_handvals["sample_i"].fill(-1)
        river_handvals["c"].fill(-1)
        try:
            flop = work_queue.get(block=False)
        except:
            print "queue emprty"
            break
        flop_types[0]["board_type"] = zp.get_board_slot_flop(flop)
        flop_types[0]["hand_types"][:] = zp.get_all_hand_slots_flop(flop)

        flop_i = lzp.ctoi3[flop[0]][flop[1]][flop[2]]

        print "got flop"
        turn = 0
        while turn < 52:
            turn_types[turn]["board_type"] = zp.get_board_slot_turn(list(flop)+[turn])
            turn_types[turn]["hand_types"][:] = zp.get_all_hand_slots_turn(list(flop)+[turn])
            turn += 1
        print "got turns"

        for turn in xrange(52):
            if turn in flop:
                continue
            for river in xrange(52):
                if river in flop or turn == river:
                    continue
                river_types[turn,river]["board_type"] = zp.get_board_slot_river(list(flop)+[turn,river])
                river_types[turn,river]["hand_types"][:] = zp.get_all_hand_slots_river(list(flop)+[turn,river])
                lzp.lib.get_river_hand_hv2_all_hands(flop_i, turn, river, river_handvals[turn,river].ctypes.data_as(POINTER(dt.hand_hv2)))
                #print river_handvals[turn,river]
            print "got one turn rivers"
        flop_types.tofile(save_path + str(flop_i)+".flop_types")
        turn_types.tofile(save_path + str(flop_i)+".turn_types")
        river_types.tofile(save_path + str(flop_i)+".river_types")
        river_handvals.tofile(save_path + str(flop_i)+".river_handvals")
        
gamedir = sys.argv[1]
zp = zpoker.zpoker(gamedir)

for x in range(4):
    if x != 3:
        zp.load_hand_slots(x)
    zp.load_hand_types(x)
    zp.load_hand_diffs(x)
        
    if x != 0:
        #zp.load_board_types(x)
        zp.load_board_slots(x)


# flops = {}
# i1 = 51
# while i1 >= 0:
#     i2 = i1-1
#     while i2 >= 0:
#         i3 = i2-1
#         while i3 >= 0:
#             flop = get_morph_flop(np.array((i1,i2,i3)))
#             try:
#                 flops[flop] += 1
#             except KeyError:
#                 flops[flop] = 1
#             i3-=1
#         i2-=1
#     i1-=1


flops_list = ct.morph_flops.keys()
flops_list.sort()

random.seed(1337)
random.shuffle(flops_list)


flop_queue = Queue.Queue()

workers = []

save_path = sys.argv[2]
n_workers = int(sys.argv[5])

start_flop = int(sys.argv[3])
stop_flop = int(sys.argv[4])

for flop in flops_list[start_flop:stop_flop]:
    flop_queue.put(ct.itoc3[flop])


for i in xrange(n_workers):
    new_worker = threading.Thread(target=calc_flop, args=(flop_queue, save_path))
    new_worker.daemon = True
    new_worker.start()
    workers.append(new_worker)
    
for w in workers:
    w.join()

sys.exit()

flop_queue.put([1,42,22])
calc_flop(flop_queue)

sys.exit()

        
gamedir = sys.argv[1]
jbs_data_filename = sys.argv[2]+".data"
jbs_offset_filename = sys.argv[2]+".offset"

f = file(jbs_data_filename, "ab")
off_f = file(jbs_offset_filename, "ab")





header = numpy.zeros((2), dtype=numpy.int64)
hand_values = numpy.zeros((zp.SAMPLES), dtype=dt.hand_hv2_np)
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
    
    board_jbs = numpy.array(zp.get_board_slots_all_gs(board), dtype=numpy.int16)
    #print board_jbs
    if -1 in board_jbs:
        print board, board_jbs
        sys.exit()
    hand_slots = zp.get_all_hand_slots_all_gs(board)
    valid_slots= numpy.where(hand_slots[3] != -1)[0]
    
#    for i, x in enumerate(hand_slots):
#        hand_slots[i] = x[valid_slots[0]]

    lzp.lib.get_river_hand_hv2_all(lzp.ctoi3[board[0]][board[1]][board[2]], board[3], board[4], hand_values.ctypes.data_as(POINTER(dt.hand_hv2)))
    #lzp.lib.get_river_hand_hv_all(zp.ctoi3[board[0]][board[1]][board[2]], board[3], board[4], hand_values.ctypes.data_as(POINTER(c_uint32)))
    hand_jbs = numpy.vstack((hand_slots[0][valid_slots], hand_slots[1][valid_slots], hand_slots[2][valid_slots], hand_slots[3][valid_slots]))#.transpose()
    board_jbs_str = board_jbs.tostring()
#    for x in xrange(zp.HANDS):
#        hand_jbs_str += hand_jbs[x].tostring()
#        hand_jbs_str += hand_values[x].tostring()
    #print len(board_jbs_str), len(hand_jbs.tostring()), len(hand_values.tostring())
    #print hand_values
    combined_cmp = zlib.compress(board_jbs_str+hand_jbs.tostring()+hand_values.tostring())
    
    header[0] = f.tell()
    header[1] = len(combined_cmp)
   
    #print len(combined_cmp)
    f.write(combined_cmp)
    header.tofile(off_f)
    if not i%1000:
        print i, board, header

f.close()
off_f.close()
