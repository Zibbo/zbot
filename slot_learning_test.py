import numpy

items = 512
odds = np.random.random(items)

summa = np.zeros(512, dtype=np.float64)

while 1:
    spread = odds*np.random.random(items)
    off = summa == 0
    turn_on = (odds/100.0)-np.random.random(items) > 0
    turn_on *= off
    summa += 1*turn_on
