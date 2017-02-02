import math
import pylab
import numpy as np
import sys
import random

x = 0.0
inc = 0.001

ev = np.zeros((2,), dtype=np.float64)
regs = np.zeros((2,), dtype=np.float64)
odds = np.zeros((2,), dtype=np.float64)

s = []
c = []
tot_ev = [0]
tot_odds = []
tot_avg_odds = []
evc = 0
while x < int(sys.argv[1]):
    x+=inc
    evc+=1

    cur_odds = regs.copy()
    res = np.where(cur_odds < 0)
    cur_odds[res] = 0
    if cur_odds.sum() == 0:
        cur_odds.fill(0.5)
    else:
        cur_odds[:] /= cur_odds.sum()

    ev[0] += math.sin(x)*random.uniform(0,5)+random.uniform(-5,5)+(0.5-cur_odds[0])*random.uniform(0,1)
    ev[1] += math.cos(x)*random.uniform(0,5)+random.uniform(-5,5)+(0.5-cur_odds[1])*random.uniform(0,1)
    if evc%1:        
        continue
    avg_ev = (ev*cur_odds).sum()
    regs[:] += ev[:]-avg_ev
    odds += cur_odds

    s.append(ev[0])
    c.append(ev[1])
    tot_odds.append(cur_odds[0])
    tot_avg_odds.append(odds[0]/odds.sum())
    tot_ev.append(tot_ev[-1]+avg_ev)
    ev.fill(0)

    x+=inc
# s = np.arange(0,10,0.001)
# c = np.arange(0,10,0.001)
# t = np.arange(0,10,0.001)
# s[:] = np.sin(s[:])
# c[:] = np.cos(c[:])
# t[:] = np.tan(t[:])
print odds
print regs

#pylab.plot(s)
#pylab.plot(c)
pylab.plot(tot_odds)
pylab.plot(tot_avg_odds)
#pylab.plot(tot_ev)
pylab.show()
