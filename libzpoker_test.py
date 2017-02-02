import zpoker
import sys
import random
import card_table as ct
import time
import lzp




gamedir = sys.argv[1]
zp = zpoker.zpoker(gamedir)

print  zp.itoc3[5863].c1
print  zp.itoc3[5863].c2
print  zp.itoc3[5863].c3
print  lzp.itoc3[5863].c1
print  lzp.itoc3[5863].c2
print  lzp.itoc3[5863].c3

print lzp.ctoi3[0][7][47]
