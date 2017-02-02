import random
import numpy as np
import sys
import math

def gauss(height, center, width, x):
    return math.exp( -((x-center)**2.0) / (2.0*(width**2.0) ) )

def gauss2(height, center, width, x):
    return (1.0/(math.sqrt(2*math.pi*width))) * math.exp( -((x-center)**2.0) / (2.0*(width**2.0) ) )
    
def std_dist(x, u, q):
    a = 1.0/math.sqrt(2*math.pi*q)
    b = math.exp(-(((x-u)**2)/(2*q)))
    print x,a,b
    return b

def normal_x(P, q, u):
    x = math.sqrt(-math.log(P)*2*q) + u
    return x


res1 = []
res2 = []
res3 = []
for x in np.arange(4000, 6000, 10):
    #res1.append(gauss(1, 0, 3, x))
    #res2.append(gauss2(1, 0, 3, x))
    res3.append(std_dist(x, 5000.0, 2500.0))




print res1
print res2
print res3


for x in np.arange(0.001, 1.0, 0.001):
    print normal_x(x, 2500.0, 5000.0)
