import shutil
import os
import sys
import random


src_dir = sys.argv[1]
dest_dir = sys.argv[2]

n_files_to_copy = int(sys.argv[3])


files = os.listdir(src_dir)
files_to_copy = []

for i in xrange(n_files_to_copy):
    files_to_copy.append(files.pop(random.randint(0,len(files)-1)))

for f in files_to_copy:
    shutil.copytree(src_dir+f, dest_dir+f)

