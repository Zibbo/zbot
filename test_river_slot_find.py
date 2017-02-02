from operator import mul
import math


def get_skip(start, cur, items):
    mul1 = reduce(mul, range(start + 1 - items, start + 1))/math.factorial(items)
    mul2 = reduce(mul, range(cur + 1 - items, cur + 1))/math.factorial(items)
    return mul1 - mul2

def f(cards):
    sum=0
    start = 52
    for i,c in enumerate(cards):
        sum+= get_skip(start, c+1, 5-i)
        start=c
    print sum
    

print f([51,50,49,48,47])
print f([4,3,2,1,0])
