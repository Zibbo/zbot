import sys
import random
import copy
import numpy

import card_table as ct
import type

HANDS = 1326

n_types = int(sys.argv[1])
wl_slots = int(sys.argv[2])
tie_slots = int(sys.argv[3])


alltypes = type.load_preflop_types_raw(wl_slots, tie_slots)
print alltypes.shape

new_types = numpy.zeros((n_types,tie_slots,wl_slots), dtype = numpy.float64)
#types = [None]*zp.info.n_preflop_types
diffs = numpy.zeros((n_types, n_types), dtype = numpy.float64)
#diffs = [[0.0,]*zp.info.n_preflop_types]*zp.info.n_preflop_types
tmptype = None

for i,x in enumerate(new_types):
    new_types[i] = alltypes[random.randint(0,HANDS-1)].copy()


for i,x in enumerate(new_types):
    print i
    type.calc_diffs(new_types, diffs, i)

#print types
#print diffs

while 1:
    smallest, typelist = type.get_closest_match(diffs);
    #print smallest, typelist
#    print types[typepair[0]]
#    print types[typepair[1]]
    #if typef.get_avg_diff(new_types[typepair[0]]) > get_avg_diff(new_types[typepair[1]]):
    #    tc = typepair[1]
    #else:
    #    tc = typepair[0]

    i = 0
    j = 0
    while i != n_types:
        tc = random.choice(random.choice(typelist))
        tmptype = alltypes[random.randint(0,HANDS-1)].copy()
        i = 0
        #print tmptype
        #print zp.info.n_preflop_types
        while i < n_types:
            if i != tc:
                #print "tmptype", tmptype
                #print "new_type", new_types[i]
                diff = type.get_difference(tmptype, new_types[i])
            #print diff, smallest, tmptype, i
                if diff <= smallest:
                    break
            i+= 1
        j+=1

    new_types[tc] = tmptype.copy()
    type.save_preflop_types(new_types, n_types, wl_slots, tie_slots)
    
    diff = type.calc_diffs(new_types,diffs, tc)
    print "small", diff, smallest, "ohi:", j
