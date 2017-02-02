import random
import numpy as np
import sys
import math

def gauss(center, width, x):
    return math.exp( -((x-center)**2.0) / (2.0*(width**2.0) ) )


def normal_x(P, q, u):
    x = math.sqrt(-math.log(P)*2*q)*random.randrange(-1,2,2) + u
    return x

res = [0]*10000
res2 = [0]*10000

prob = float(sys.argv[1])
iters = int(sys.argv[2])

u = prob*10000
q = u*(1-prob)


for i in xrange(iters):
    rnd = np.random.random(10000)
    idx = len(np.where(rnd > prob)[0])
    res[idx]+=1
    res2[int(normal_x(random.random(), q,u))] += 1

avg = 0.0
tot = 0.0
for i,x in enumerate(res):
    avg += i*x
    tot += x

avg2 = 0.0
tot2 = 0.0
for i,x in enumerate(res2):
    avg2 += i*x
    tot2 += x

#for i, x in enumerate(res):
#    print i,x
print avg/tot
print avg2/tot2

a = avg/tot
a2 = avg2/tot2
avg = 0.0

for i,x in enumerate(res):
    avg += (abs(a-i)**2)*x
    if x != 0:
        print i, x/float(iters)

for i,x in enumerate(res2):
    avg2 += (abs(a2-i)**2)*x
    if x != 0:
        print i, x/float(iters)



variance = avg/tot
variance2 = avg2/tot2
print a, variance
print a2, variance2
