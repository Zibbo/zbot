import libzpoker
import sys


zp = libzpoker.libzpoker()

#t = zp.gen_types_for_flop(0, 40, 5, 2.0, 1.0)
gs = int(sys.argv[1])
slots = int(sys.argv[2])


zp.i.wl_slots = 40
zp.i.tie_slots = 5
zp.i.wl_scale = 2.0
zp.i.tie_scale = 1.0
zp.i.wl_width = 15.0
zp.i.tie_width = 5.0
zp.i.n_types[gs] = slots
#zp.i.n_types[1] = slots
#zp.i.n_types[2] = slots
#zp.i.n_types[3] = 128
i = 0
save = False
print zp.load_types(gs)
print zp.load_diffs(gs)
print zp.types[gs]
print zp.diffs[gs]
while zp.try_to_add_new_random_type_to_types(gs, 10000, save):
    print "try", i
    i+=1
    if not i%1000 or gs == 0: 
        save = True
    else:
        save = False
        
#for x in range(int(sys.argv[2])):
#    zp.calc_diffs(0, x)
#print zp.get_closest_match(0)

#for x in t:
#    print x
   
