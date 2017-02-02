import libzpoker
import sys


zp = libzpoker.libzpoker()

zp.load_diffs(1)

type_list = zp.build_pyramid2(range(zp.i.n_types[1]), zp.diffs[1])
diffs = zp.diffs[1]
final_i1 = type_list[0]
final_i2 = type_list[-1]
for i, x in enumerate(type_list):
    print diffs[x,final_i1], diffs[x, final_i2], diffs[x,final_i1] - diffs[x, final_i2], x, i
