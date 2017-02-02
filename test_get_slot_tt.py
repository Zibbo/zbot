import sys
import numpy as np
import ctypes as C

import zpoker
import datatypes as dt
import lzp

gamedir = sys.argv[1]
public = int(sys.argv[2])
tid = int(sys.argv[3])


zp = zpoker.zpoker(gamedir)

zp.load_types_for_all_types()
zp.load_diffs_for_all_types()
zp.load_slots_for_all_types()


zp.set_info_pointers()
hi = dt.hand_info()
tt = zp.i.type_types[public][tid]

zp.load_diffs_lookup_for_type(tt)

diffs_lookup = np.zeros((tt.n_types, 10, (tt.n_types-1)/64+1), dtype=np.uint64)
tt.diffs_lookup = diffs_lookup.ctypes.data_as(C.POINTER(C.c_uint64))

b = np.arange(52, dtype=np.int8)
last_change = 1
try:
    while True:
        np.random.shuffle(b)
        cb = (C.c_int8*5)(*b[:5])

        hi.board = cb

        t = np.zeros(tt.n_items_per_type, dtype=np.float32)
        lzp.tt_call_gen_type(C.pointer(tt), C.pointer(hi), t.ctypes.data_as(C.POINTER(C.c_float)))
        ret = lzp.fill_diffs_lookup(C.pointer(tt), t.ctypes.data_as(C.POINTER(C.c_float)))
        if ret:
            print ret, last_change
            last_change = 0
        last_change += 1
except KeyboardInterrupt:
    pass

diffs_lookup.tofile("testipeli/board_diffs_lookup_gs3_s1024_tb128_te128.ftype")
