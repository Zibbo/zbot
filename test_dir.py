import os
import sys

try:
    pass
#os.mkdir("testipeli")
except OSError:
    print dir(sys.exc_info()[1])
    print sys.exc_info()[1].errno
    print sys.exc_info()[1].args
    print sys.exc_info()[1].strerror
    
l =  os.listdir(".")

print l
print len(l)
