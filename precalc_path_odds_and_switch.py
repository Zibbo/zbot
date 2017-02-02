import libzpoker
import sys
import random
import shelve
import numpy

gamedir = sys.argv[1]
zp = libzpoker.libzpoker(gamedir)

zp.reset_path_stats()

hands = shelve.open("../huphands2")
i = 0
try:
    for key in hands:
        h = hands[key]
        if h == {}:
            continue
        acts = ""
        for act in h["pf_acts"]:
            acts += act[1]
        for act in h["f_acts"]:
            acts += act[1]
        for act in h["t_acts"]:
            acts += act[1]
        for act in h["r_acts"]:
            acts += act[1]
   
        zp.add_path_to_stats(acts)
        if not i%1000:
            print i
        i+=1
except KeyboardInterrupt:
    pass

new_path_odds = None
for i in range(4):
    new_table = zp.expand_odds_table_and_normalize(zp.path_odds[i])
    if new_path_odds == None:
        new_path_odds = numpy.zeros((4,)+new_table.shape, dtype=numpy.float64)
    new_path_odds[i] = new_table    
    print "expanded", i 
    print new_path_odds[i]
    print
zp.path_odds = new_path_odds

new_gs_switch_path = None
#for i, table in enumerate(zp.gs_switch_path):
for i in xrange(zp.n_us):
    new_table = zp.expand_odds_table_and_normalize(zp.gs_switch_path[i])
    if new_gs_switch_path == None:
        new_gs_switch_path = numpy.zeros((zp.n_us,)+new_table.shape, dtype=numpy.float64)
    new_gs_switch_path[i] = new_table
#    zp.gs_switch_path[i] = zp.expand_odds_table_and_normalize(table)
    print "expanded", i 
    print zp.int_to_us[i] 
    print new_gs_switch_path[i]
    print

zp.gs_switch_path = new_gs_switch_path

#for i in range(4):
zp.save_path_odds()
zp.save_gs_switch_path()


#    if x != 3:
#        zp.load_hand_slots(x)
#    zp.load_hand_types(x)
#    zp.load_hand_diffs(x)
#    if x != 0:
#        zp.load_board_types(x)




#for x in range(4):
#    if x != 0:
#        zp.save_gs_hand_switch_tables(x)
#        zp.save_gs_board_switch_tables(x)
#        zp.save_gs_board_odds_tables(x)
#    zp.save_gs_hand_odds_tables(x)
