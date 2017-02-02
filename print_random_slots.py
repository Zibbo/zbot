import libzpoker
import sys

gamedir = sys.argv[1]
gs = int(sys.argv[2])
#n_types = int(sys.argv[3])
#type_i = int(sys.argv[3])



zp = libzpoker.libzpoker(gamedir)

#zp.i.n_types[gs] = n_types

zp.load_hand_types(gs)
zp.load_hand_diffs(gs)
zp.load_hand_slots(gs)

#print zp.types[1][4]
#print zp.types[1][5]
#print zp.diffs[1][4:8]
for x in range(zp.i.n_types[gs]):
    print x,
    zp.print_hands_from_type(gs, x)
