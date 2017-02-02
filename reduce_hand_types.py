import zpoker
import sys
import numpy
from ctypes import *

gamedir = sys.argv[1]
gs = int(sys.argv[2])
start_types = int(sys.argv[3])
reduce_to = int(sys.argv[4])

zp = zpoker.zpoker(gamedir)

zp.i.n_types[gs] = start_types

zp.load_hand_types(gs)
zp.load_hand_diffs(gs)
zp.load_hand_slots(gs)

types = zp.types[gs]
diffs = zp.diffs[gs]
#slots = zp.slots[gs]

new_diffs, new_diffs_order, new_types = zp.reduce_types(zp.diffs[gs], start_types, reduce_to)

print "Generating mappings... ", 
mapping = numpy.arange(start_types+1, dtype=numpy.int16)
mapping[-1] = -1 #pikku hax, etta -1 typeille tulee mapatessa -1 typeksi
i = 0
while i < start_types:
    smallest = 1000000000
    j = 0
    smallest_j = -1
    while j < reduce_to:
        if i == new_types[j]:
            smallest = 0
            smallest_j = j
            break
        if zp.diffs[gs][i, new_types[j]] < smallest:
            smallest = zp.diffs[gs][i, new_types[j]]
            smallest_j = j
        j+=1
    mapping[i] = smallest_j
    i+=1


print mapping
print start_types, zp.i.n_types[gs]

final_types = numpy.zeros((reduce_to, zp.i.n_rtypes[gs]), dtype=numpy.float32)
i = 0
while i < reduce_to:
    final_types[i] = types[new_types[i]]
    i+=1
    
zp.types[gs] = final_types
zp.i.types[gs] = zp.types[gs].ctypes.data_as(POINTER(c_float))
zp.i.n_types[gs] = reduce_to
zp.diffs[gs] = new_diffs
zp.i.diffs[gs] = zp.diffs[gs].ctypes.data_as(POINTER(c_float))

print "Recalculating slots... ", mapping[-1]
if zp.slots[gs] != None:
    i = 0
    slots = zp.slots[gs]
    n_slots = len(slots)
    percent = n_slots/100
    while i < len(zp.slots[gs]):
        slots[i] = mapping[slots[i]]
                #else:
                #    print slots[i]
        i+=1
        if not i%percent:
            print (float(i)/float(n_slots))*100, "% "
print "Done"
print mapping, len(mapping)
        #new_types.sort()
print new_types



zp.save_hand_slots(gs)
zp.save_hand_diffs(gs)
zp.save_hand_types(gs)
