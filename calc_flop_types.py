import sys
import random
import copy
import numpy

import card_table as ct
import libzpoker
import type

def get_random_flop(flops, wl_slots,tie_slots):
    retval = flops[random.randint(0,22099)][random.randint(0,1325)]
    while retval.sum() == 0:
        retval = flops[random.randint(0,22099)][random.randint(0,1325)]
    return retval

random.seed()

HANDS = 1326

n_types = int(sys.argv[1])
wl_slots = int(sys.argv[2])
tie_slots = int(sys.argv[3])


new_types = numpy.zeros((n_types,tie_slots,wl_slots), dtype = numpy.float64)
#types = [None]*zp.info.n_preflop_types
diffs = numpy.zeros((n_types, n_types), dtype = numpy.float64)
#diffs = [[0.0,]*zp.info.n_preflop_types]*zp.info.n_preflop_types
tmptype = None


for i,x in enumerate(new_types):
    print i
    new_types[i] = type.gen_random_flop_type(wl_slots, tie_slots)

print "calculating diffs"

for i,x in enumerate(new_types):
    type.calc_diffs(new_types, diffs, i)

print "done"
while 1:
    smallest, typelist = type.get_closest_match(diffs);
    i = 0
    j = 0
    while i != n_types:
        tc = random.choice(random.choice(typelist))
        tmptype = type.gen_random_flop_type(wl_slots, tie_slots)
        i = 0
        
        while i < n_types:
            if i != tc:
                diff = type.get_difference(tmptype, new_types[i])
                if diff <= smallest:
                    break
            i+= 1
        j+=1

    new_types[tc] = tmptype.copy()
    type.save_flop_types(new_types, n_types, wl_slots, tie_slots)
    
    diff = type.calc_diffs(new_types,diffs, tc)
    print "small", diff, smallest, "ohi:", j

