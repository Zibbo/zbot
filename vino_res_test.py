import cPickle
import sys

f = file(sys.argv[1], "rb")
d = cPickle.load(f)
f.close()

res_dic = {}


for key,value in d.items():
    if key[0] not in res_dic:
        res_dic[key[0]] = [0.0, 0.0]

    if key[1] not in res_dic:
        res_dic[key[1]] = [0.0, 0.0]
    res_dic[key[0]][0] += value[0]
    res_dic[key[1]][1] += value[1]
 
for x in res_dic.items():
    print x
