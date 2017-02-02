import sys
import numpy as np


res = None

joined_filename = sys.argv[1]

for filename in sys.argv[2:]:
    if res == None:
        res = np.fromfile(filename, np.uint64).reshape(-1,2)
    else:
        new_file = np.fromfile(filename, np.uint64).reshape(-1,2)
        new_file[:,0] += offset
        res = np.concatenate((res, new_file))
    print "last element", res[-1]
    offset = res[-1].sum()
    print "offset", offset
    print res

res.tofile(joined_filename)
