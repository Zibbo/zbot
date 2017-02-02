import libzpoker
import numpy

zp = libzpoker.libzpoker("testipeli")


zp.load_hand_wtl_odds()
zp.load_gs_hand_switch_tables(1)
zp.load_gs_hand_switch_tables(2)
zp.load_gs_hand_switch_tables(3)
zp.load_gs_hand_odds_tables(0)
zp.load_gs_hand_odds_tables(1)
zp.load_gs_hand_odds_tables(2)
zp.load_gs_hand_odds_tables(3)

nh = [64,8,8,8]

hw = [numpy.zeros((nh[0]), dtype=numpy.float64),numpy.zeros((nh[0],nh[1]), dtype=numpy.float64),numpy.zeros((nh[0],nh[2]), dtype=numpy.float64),numpy.zeros((nh[0],nh[3]), dtype=numpy.float64)]

print zp.gs_odds_hand[0]

hw[0] = zp.gs_odds_hand[0][1, nh[0]:nh[0]*2]

for i in xrange(nh[0]):
    hw[1][i] = zp.gs_switch_hand[1][1,nh[0]+i,nh[1]:nh[1]*2]

for i in xrange(nh[0]):
    for n in xrange(nh[1]):
        for x in xrange(nh[2]):
            hw[2][i,x] += zp.gs_switch_hand[2][1,nh[1]+n,nh[2]+x]*hw[1][i,n]

for i in xrange(nh[0]):
    for n in xrange(nh[2]):
        for x in xrange(nh[3]):
            hw[3][i,x] += zp.gs_switch_hand[3][1,nh[2]+n,nh[3]+x]*hw[2][i,n]


print hw[1][0]
print hw[2][0]
print hw[3][0]

#print numpy.add.reduce(zp.hand_wtl_odds[1,1,nh[3]:nh[3]*2]*zp.gs_odds_hand[3][1][:,numpy.newaxis][nh[3]:nh[3]*2], axis = 0)
#print zp.gs_odds_hand[3][1][:,numpy.newaxis]
print
avg_hand_odds = zp.gs_odds_hand[3][1][:,numpy.newaxis][nh[3]:nh[3]*2]
victory_odds = numpy.zeros((nh[3],3), dtype=numpy.float64)
for x in xrange(nh[3]):
    #print x, zp.hand_wtl_odds[1,nh[3]+x,nh[3]:nh[3]*2]
    victory_odds[x] += numpy.add.reduce(zp.hand_wtl_odds[nh[3]+x,nh[3]:nh[3]*2]*avg_hand_odds, axis=0)
print victory_odds
for x in xrange(nh[0]):
    print x, numpy.add.reduce(hw[3][x][:,numpy.newaxis] * victory_odds, axis = 0)
