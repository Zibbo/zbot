import sys

import worker_control

c = worker_control.ctrl(sys.argv[1])
print "ctrl reated"

while True:
    c.perform_one_loop(0.01)
    
