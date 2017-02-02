import numpy as np
import card_table as ct




flop_dict = {}


def get_morph_count(c):
    if c[0]/13 == c[1]/13 == c[2]/13:
        return 4
    elif c[0]/13 != c[1]/13 and c[0]/13 != c[2]/13 and c[1]/13 != c[2]/13:
        if c[0]%13 == c[1]%13 == c[2]%13:
            return 4
        elif c[0]%13 != c[1]%13 and c[0]%13 != c[2]%13 and c[1]%13 != c[2]%13:
            return 24
        else:
            return 12
    else:
        return 12

def get_morph_flop(c):
    if c[0]/13 == c[1]/13 == c[2]/13:
        flop = ct.ctoi3[c[0]%13,c[1]%13,c[2]%13]
    elif c[0]/13 != c[1]/13 and c[0]/13 != c[2]/13 and c[1]/13 != c[2]/13:
        tmpc = np.sort(c%13)
        tmpc[1]+=13
        tmpc[2]+=26
        flop = ct.ctoi3[tmpc[0], tmpc[1], tmpc[2]]
    else:
        if (c[0]/13 == c[1]/13):
            flop = ct.ctoi3[c[0]%13, c[1]%13, c[2]%13+13]
        elif c[0]/13 == c[2]/13:
            flop = ct.ctoi3[c[0]%13, c[2]%13, c[1]%13+13]
        elif c[1]/13 == c[2]/13:
            flop = ct.ctoi3[c[1]%13, c[2]%13, c[0]%13+13]
        else:
            print "fail"
            sys.exit()

            #return flop
    try:
        flop_dict[flop]+=1
    except KeyError:
        flop_dict[flop]=1

    
mc = 0
mc2 = 0
i1 = 51
while i1 >= 0:
    i2 = i1-1
    while i2 >= 0:
        i3 = i2-1
        while i3 >= 0:
            get_morph_flop(np.array((i1,i2,i3)))
            i3-=1
        i2-=1
    i1-=1

print flop_dict

print len(flop_dict)
for key in flop_dict.keys():
    mc += get_morph_count(ct.itoc3[key])
    

for v in flop_dict.values():
    mc2+=v

for i in flop_dict.items():
    print get_morph_count(ct.itoc3[i[0]]), i[1], ct.cards_to_str(ct.itoc3[i[0]])
print mc
print mc2
