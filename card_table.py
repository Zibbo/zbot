import numpy as np

stoi = {}
itos = []
#ctoi3 = range(52*52*52)
#ctoi2 = range(52*52)
#itoc2 = range(1326)
#itoc3 = range(22100)
itoc2 = np.zeros((1326,2), dtype=np.int32)
itoc3 = np.zeros((22100,3), dtype=np.int32)
itoc2_no_suit = np.zeros((169,2), dtype=np.int32)
ctoi2 = np.zeros((52,52), dtype=np.int32)
ctoi3 = np.zeros((52,52,52), dtype=np.int32)
ctoi2_no_suit = np.zeros((13,13), dtype=np.int)

ctoi2_3s = np.zeros((39,39), dtype=np.int32)
ctoi2_2s = np.zeros((26,26), dtype=np.int32)

itoc2_3s = np.zeros((741,2), dtype=np.int32)
itoc2_3s = np.zeros((325,2), dtype=np.int32)

suit = ['s','c','d','h']
number = ['A', 'K', 'Q', 'J', 'T', '9', '8', '7', '6', '5', '4', '3', '2']

i = 51

for s in suit:
    for n in number:
        itos.append(n+s)
        stoi[n.lower()+s] = i
        stoi[n.upper()+s] = i
        stoi[n.upper()+s.upper()] = i
        stoi[n.lower()+s.upper()] = i
        if n == "T":
            stoi["10"+s] = i
            stoi["10"+s.upper()] = i
                    
        i -= 1

itos.reverse()


i1 = 51
i = 0
while i1 >= 0:
    i2 = i1-1
    while i2 >= 0:
        ctoi2[i1,i2] = i
        ctoi2[i2,i1] = i
#         ctoi2[i1*52 + i2] = i
#         ctoi2[i2*52 + i1] = i
        itoc2[i] = [i1, i2]
        i+=1
        i2-=1
    i1-=1


i1 = 51
i = 0
while i1 >= 0:
    i2 = i1-1
    
    while i2 >= 0:
        i3 = i2-1
        while i3 >= 0:
            ctoi3[i1,i2,i3] = i
            ctoi3[i1,i3,i2] = i
            ctoi3[i2,i1,i3] = i
            ctoi3[i2,i3,i1] = i
            ctoi3[i3,i1,i2] = i
            ctoi3[i3,i2,i1] = i
            itoc3[i] = [i1, i2, i3]
#             ctoi3[i1*52*52 + i2*52 + i3] = i
#             ctoi3[i1*52*52 + i3*52 + i2] = i
#             ctoi3[i2*52*52 + i1*52 + i3] = i
#             ctoi3[i3*52*52 + i1*52 + i2] = i
#             ctoi3[i2*52*52 + i3*52 + i1] = i
#             ctoi3[i3*52*52 + i2*52 + i1] = i
            
            i+=1
            i3 -= 1
        i2-=1
    i1-=1




i1 = 12
i = 0
while i1 >= 0:
    i2 = i1-1
    while i2 >= 0:
        ctoi2_no_suit[i1,i2] = i
        ctoi2_no_suit[i2,i1] = i
#         ctoi2[i1*52 + i2] = i
#         ctoi2[i2*52 + i1] = i
        itoc2_no_suit[i] = [i1, i2]
        i+=1
        i2-=1
    i1-=1


preflop_morph_mapping = np.zeros(1326, dtype=np.int)

i1 = 51
i = 0
while i1 >= 0:
    i2 = i1-1
    while i2 >= 0:
        if i1%13 == i2%13:
            preflop_morph_mapping[i] = i1%13
            #pair
            pass
        elif i1/13 == 12/13:
            preflop_morph_mapping[i] = 13+ctoi2_no_suit[i1%13,i2%13]
            #suited
            pass
        else:
            preflop_morph_mapping[i] = 13+78+ctoi2_no_suit[i1%13,i2%13]
            pass
    
        i2-=1
    i1-=1





def cards_to_int_3(c):
    return ctoi3[c[0],c[1],c[2]]

def cards_to_int_2(c):
    return ctoi2[c[0],c[1]]

def cards_to_str(cards):
    retval = []
    for x in cards:
        retval.append(itos[x])
    return retval

def get_morph_flop(c):
    if c[0]/13 == c[1]/13 == c[2]/13:
        flop = ctoi3[c[0]%13,c[1]%13,c[2]%13]
    elif c[0]/13 != c[1]/13 and c[0]/13 != c[2]/13 and c[1]/13 != c[2]/13:
        #print c
        tmpc = np.sort(c%13)
        #print tmpc
        #tmpc = tmpc%13
        #print tmpc
        tmpc[1]+=13
        tmpc[2]+=26
        #print tmpc
        #print
        flop = ctoi3[tmpc[0], tmpc[1], tmpc[2]]
    else:
        if (c[0]/13 == c[1]/13):
            flop = ctoi3[c[0]%13, c[1]%13, c[2]%13+13]
        elif c[0]/13 == c[2]/13:
            flop = ctoi3[c[0]%13, c[2]%13, c[1]%13+13]
        else:
            flop = ctoi3[c[1]%13, c[2]%13, c[0]%13+13]


    return flop

morph_flops = {}
flop_morph_mapping = np.zeros(22100, dtype=np.int)
i1 = 51
while i1 >= 0:
    i2 = i1-1
    while i2 >= 0:
        i3 = i2-1
        while i3 >= 0:
            flop = get_morph_flop(np.array((i1,i2,i3)))
            try:
                morph_flops[flop] += 1
            except KeyError:
                morph_flops[flop] = 1
            flop_morph_mapping[ctoi3[i1,i2,i3]] = flop
            i3-=1
        i2-=1
    i1-=1
