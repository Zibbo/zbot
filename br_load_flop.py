import numpy as np

import sys
import pdb
import card_table as ct

data = np.fromfile(sys.argv[1], dtype = np.float64).reshape(-1,1326)

hw = data[:2]

ev = data[2:].reshape(3,2,1326)

print hw
print ev

pdb.set_trace()
