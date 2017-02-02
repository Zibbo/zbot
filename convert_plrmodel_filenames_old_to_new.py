import os
import sys 

plrmodel_path = sys.argv[1]
if plrmodel_path[-1] != "/":
    plrmodel_path+="/"

files = os.listdir(plrmodel_path)

for f in files:
    f_spl = f.split("_",1)
    print f_spl[1]
    os.rename(plrmodel_path+f, plrmodel_path+f_spl[1])
    
