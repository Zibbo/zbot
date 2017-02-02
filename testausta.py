import copy
import sys

PLR_COUNT = int(sys.argv[1])

def gen_new(situ):
    if len(situ) != 3 or len(situ[2]) <= 1:
        return []

    count = situ[0]
    pos = situ[1]
    bets = situ[2]

    largest = 0
    for x in bets:
        if x > largest:
            largest = x

    new_pos = (pos+1)%len(bets)

    start = bets[0]
    if count >= PLR_COUNT:
        for x in bets:
            if x != start:
                break
        else:
            return []


    r = copy.deepcopy(situ)
    c = copy.deepcopy(situ)
    f = copy.deepcopy(situ)
    #print c
    c[0] += 1
    c[1] = new_pos
    c[2][pos] = largest

    count = c[0]
    start = c[2][0]
    if count >= PLR_COUNT:
        for x in c[2]:
            if x != start:
                break
        else:
            c = []
    

    f[0] += 1
    if f[1]+1 == len(f[2]):
        f[1] = 0
    f[2].pop(pos)

    if len(f[2]) <= 1:
        f = []

    if largest != 4:
        r[0] += 1
        r[1] = new_pos
        r[2][pos] = largest+1
    else:
        r = []

    
    return [r,c,f]


situ = [[0, 1, [0]*PLR_COUNT]]

cont = True
tot_list = []
state_dic = {}
new_list = situ
i = 0
while cont:
    old_list = new_list
    new_list = []
    #print old_list
    for x in old_list:        
        n = gen_new(x)
        for y in n:
            if y != []:
                key = tuple(y[2])
                try:
                    state_dic[key] += 1
                except:
                    state_dic[key] = 1
                    new_list.append(y)

    if new_list == []:
        break

    print i, len(new_list)
    i += 1

# final_list = []
# for x1 in tot_list:
#     for x2 in x1:
#         if x2 == []:
#             continue
#         start = x2[2][0]
#         for x in x2[2]:
#             if x != start:
#                 break
#         else:
#             continue

#         final_list.append(x2)
        


# for x in final_list:
#     key = tuple(x[2])
#     if key not in state_dic:
#         state_dic[key] = 1
#     else:
#         state_dic[key] += 1

print state_dic
#print len(final_list)
print len(state_dic)
    

