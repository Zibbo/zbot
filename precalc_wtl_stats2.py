import sys
import numpy
import random
import card_table as ct
import libzpoker
import scipy.weave as weave


gamedir = sys.argv[1]
zp = libzpoker.libzpoker(gamedir)
print "1"
zp.load_hand_types(3)
print "2"
#print zp.types[3]
#slots = zp.get_slots_for_river(ct.cards_to_int_3((ct.stoi["As"], ct.stoi["Ks"], ct.stoi["Qs"])), ct.stoi["Js"], ct.stoi["2h"])
#ct.cards_to_int_3((ct.stoi["Ad"], ct.stoi["9s"],
#sys.exit()

wtl_stats = numpy.zeros((zp.i.n_types[3], zp.i.n_types[3], 3), dtype = numpy.float64)

for i1 in xrange(zp.i.n_types[3]):
    type1 = zp.types[3][i1]
    type1_wo = type1[0] + type1[1]
    for i2 in xrange(i1,zp.i.n_types[3]):
        type2 = zp.types[3][i2]
        type2_wo = type2[0] + type2[1]
        diff = abs(type1_wo -type2_wo)
        tot_tie = type1[1]+type2[1]-diff
        if tot_tie < 0:
            tot_tie = 0.0
        if type1_wo > type2_wo:
            wtl_stats[i1,i2] = (1.0-tot_tie, tot_tie, 0.0)
            wtl_stats[i2,i1] = (0.0, tot_tie, 1.0-tot_tie)
        elif type2_wo > type1_wo:
            wtl_stats[i2,i1] = (1.0-tot_tie, tot_tie, 0.0)
            wtl_stats[i1,i2] = (0.0, tot_tie, 1.0-tot_tie)
        else:
            tot_tie = 1.0
            wtl_stats[i2,i1] = (0.5-tot_tie/2.0, tot_tie, 0.5-tot_tie/2.0)
            wtl_stats[i1,i2] = (0.5-tot_tie/2.0, tot_tie, 0.5-tot_tie/2.0)
i = 0
for x in wtl_stats:
    print i, x
    i+=1


wtl_stats = zp.expand_odds_table_and_normalize(wtl_stats, False, 1)
zp.hand_wtl_odds = wtl_stats
zp.save_hand_wtl_odds()
sys.exit()
        
zp.load_board_types(2)
print "3"
zp.load_hand_slots(2)
print "4"
zp.load_hand_diffs(2)
print "5"
zp.load_hand_diffs(3)
print "6"
zp.load_board_types(3)
print "7"
wtl_stats = numpy.zeros((zp.i.n_btypes[3], zp.i.n_types[3], zp.i.n_types[3], 3), dtype = numpy.float64)
#wtl_stats_final = numpy.zeros((zp.i.n_btypes[3], zp.i.n_types[3], zp.i.n_types[3]), dtype = numpy.float64)
print "8"
tmp_wtl_stats = numpy.zeros((zp.i.n_types[3], zp.i.n_types[3], 3), dtype = numpy.float64)
#tmp_wtl_stats_final = numpy.zeros((zp.i.n_types[3], zp.i.n_types[3]), dtype = numpy.float64)

print "9"

bslots_count = [0]*128
b_slot = 0
x = 0
try:
    while 1:
        x+=1
        b = random.sample(range(52), 5)
        flop_i = zp.ctoi3[b[0]][b[1]][b[2]]
        turn_i = b[3]
        river_i = b[4]
        tmp_wtl_stats.fill(0.0)
        b_slot = zp.get_slots_wtl_stats_for_river(tmp_wtl_stats, flop_i, turn_i, river_i)
        wtl_stats[b_slot] += tmp_wtl_stats
    
        if not x%100:
            print x, b_slot, ct.cards_to_str(b)
            if not x%1000:
                print wtl_stats[b_slot]
        bslots_count[b_slot] += 1
except KeyboardInterrupt:
    pass

tmp_wtl_stats.fill(0.0)

wtl_stats = zp.expand_odds_table_and_normalize(wtl_stats, False, 1)

# i = 1
# while i < wtl_stats.shape[-2]:
#     print wtl_stats[1, 30][i:i*2]
#     i *= 2

# zp.hand_wtl_odds = numpy.zeros(wtl_stats.shape[:-1], dtype = numpy.float64)

# for b in xrange(wtl_stats.shape[0]):
#     print "doing btype", b
#     for i1 in xrange(wtl_stats.shape[1]):
#         for i2 in xrange(wtl_stats.shape[2]):
#             assert wtl_stats[b,i1,i2].shape[0] == 3 
#             if wtl_stats[b,i1,i2].sum() != 0:
#                 zp.hand_wtl_odds[b,i1,i2] = (wtl_stats[b,i1,i2][0] + wtl_stats[b,i1,i2][1]/2)/wtl_stats[b,i1,i2].sum() 
#             else:
#                 zp.hand_wtl_odds[b,i1,i2] = -1.0

print "normalizing"

i = 0
elements = wtl_stats.shape[0] * wtl_stats.shape[1] * wtl_stats.shape[2]
iter_array = wtl_stats.ravel()
code = """
int i = 0;
float tot;
while (i < elements)
{
  tot = iter_array[i*3] + iter_array[i*3+1] + iter_array[i*3+2];
  if (tot > 0)
    {
      iter_array[i*3] /= tot;
      iter_array[i*3+1] /= tot;
      iter_array[i*3+2] /= tot;
    }
/*  if (!(i%10000))
    printf("%i %i\\n", i, elements);*/
  i++;
}
printf("%i %i\\n", i, elements);
"""
print code
print wtl_stats
weave.inline(code, ['elements', 'iter_array'])
print wtl_stats
print "inline done"
# while i < elements:
#     tot = iter_array[i].sum()
#     if tot > 0:
#         iter_array[i] /= tot
#     i += 1
#     if not i%10000:
#         print i
        
zp.hand_wtl_odds = wtl_stats
zp.save_hand_wtl_odds()

i = 1
while i < zp.hand_wtl_odds.shape[-1]:
    print zp.hand_wtl_odds[1, 30][i:i*2]
    i *= 2

# for i1 in xrange(zp.i.n_types[3]):
#     for i2 in xrange(zp.i.n_types[3]):
#         if tmp_wtl_stats[i1,i2].sum() != 0:
#             tmp_wtl_stats_final[i1,i2] = (tmp_wtl_stats[i1,i2][0] + tmp_wtl_stats[i1,i2][1]/2)/tmp_wtl_stats[i1,i2].sum() 
#         else:
#             tmp_wtl_stats_final[i1,i2] = -1.0


# for x in wtl_stats_final:
#     print numpy.abs(x-tmp_wtl_stats_final).sum()

# for i,x in enumerate(bslots_count):
#     print i,x,":",
# print ""

# print wtl_stats.sum()
# print wtl_stats.sum()/0.001
# print 990*990

# for x in tmp_wtl_stats_final:
#     print x
# print ""

# for x in wtl_stats_final[64]:
#     print x
# print ""

# for x in tmp_wtl_stats_final - wtl_stats_final[64]:
#     print x
# print ""

# for x in xrange(128):
#     print tmp_wtl_stats_final[x,127-x]
#     print tmp_wtl_stats_final[127-x,x]
