import libzpoker
import numpy

zp = libzpoker.libzpoker("testipeli")


zp.load_hand_wtl_odds()
nh = 32
for x in xrange(nh):
    print x, zp.hand_wtl_odds[1,nh+x,nh:nh*2]
