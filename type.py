import libzpoker
import numpy
import random
import card_table

zp = libzpoker.libzpoker()
wl_scale = 2.0
tie_scale = 1.0
wl_width = 1.0/15.0
tie_width = 1.0/5.0

use_C = True

def save_preflop_types_raw(t, wl_slots, tie_slots):
    filename = "types_preflop_pf" + str(zp.HANDS)  + "_wl" + str(wl_slots) + "_tie" + str(tie_slots) + ".ftype"
    t.tofile(filename)

def load_preflop_types_raw(wl_slots, tie_slots):
    filename = "types_preflop_pf" + str(zp.HANDS)  + "_wl" + str(wl_slots) + "_tie" + str(tie_slots) + ".ftype"
    try:
        t = numpy.fromfile(filename, numpy.float64)
    except:
        t = zp.gen_all_preflop_types(wl_slots, tie_slots, wl_scale, tie_scale, wl_width, tie_width)
        save_preflop_types_raw(t, wl_slots, tie_slots)
    t = t.reshape((zp.HANDS, tie_slots, wl_slots))
    return t

def save_preflop_types(t, n_types, wl_slots, tie_slots):
    filename = "types_preflop_pf" + str(n_types)  + "_wl" + str(wl_slots) + "_tie" + str(tie_slots) + ".ftype"
    t.tofile(filename)

def load_preflop_types(n_types, wl_slots, tie_slots):
    filename = "types_preflop_pf" + str(n_types)  + "_wl" + str(wl_slots) + "_tie" + str(tie_slots) + ".ftype"
    t = numpy.fromfile(filename, numpy.float64)
    t.reshape(n_types, tie_slots, wl_slots)
    return t

def save_flop_types(t, n_types, wl_slots, tie_slots):
    filename = "types_flop_pf" + str(n_types)  + "_wl" + str(wl_slots) + "_tie" + str(tie_slots) + ".ftype"
    t.tofile(filename)

def load_flop_types(n_types, wl_slots, tie_slots):
    filename = "types_flop_pf" + str(n_types)  + "_wl" + str(wl_slots) + "_tie" + str(tie_slots) + ".ftype"
    t = numpy.fromfile(filename, numpy.float64)
    t.reshape(n_types, tie_slots, wl_slots)
    return t

def save_turn_types(t, n_types, wl_slots, tie_slots):
    filename = "types_turn_t" + str(n_types)  + "_wl" + str(wl_slots) + "_tie" + str(tie_slots) + ".ftype"
    t.tofile(filename)

def load_turn_types(n_types, wl_slots, tie_slots):
    filename = "types_turn_t" + str(n_types)  + "_wl" + str(wl_slots) + "_tie" + str(tie_slots) + ".ftype"
    t = numpy.fromfile(filename, numpy.float64)
    t.reshape(n_types, tie_slots, wl_slots)
    return t

def gen_types_for_flop(flop, wl_slots, tie_slots):
    return zp.gen_types_for_flop(flop, wl_slots, tie_slots, wl_scale, tie_scale, wl_width, tie_width)

def gen_types_for_turn(flop, turn, wl_slots, tie_slots):
    return zp.gen_types_for_turn(flop, turn, wl_slots, tie_slots, wl_scale, tie_scale, wl_width, tie_width)

def gen_random_turn_type(wl_slots, tie_slots):
    return zp.gen_random_turn_type(wl_slots, tie_slots, wl_scale, tie_scale, wl_width, tie_width)

def gen_random_flop_type_X(wl_slots, tie_slots):
    global h_idx
    global f_idx
    global flop_mmap
    h_idx += 1
    if h_idx >= zp.HANDS:
        h_idx = 0
        f_idx += 1
        path = "flop_types_raw_wl40_tie5_wlscale2.0_tiescale1.0/"
        flop_mmap = numpy.memmap(path+str(f_idx)+".ftype", numpy.float64, "r", shape = (zp.HANDS, tie_slots, wl_slots))
    while card_table.itoc2[h_idx][0] in card_table.itoc3[f_idx] or card_table.itoc2[h_idx][1] in card_table.itoc3[f_idx] == 0:
        h_idx += 1
        if h_idx >= zp.HANDS:
            h_idx = 0
            f_idx += 1
            path = "flop_types_raw_wl40_tie5_wlscale2.0_tiescale1.0/"
            flop_mmap = numpy.memmap(path+str(f_idx)+".ftype", numpy.float64, "r", shape = (zp.HANDS, tie_slots, wl_slots))

    retval = flop_mmap[h_idx]
    return retval

def gen_random_flop_type(wl_slots, tie_slots):
    path = "flop_types_raw_wl40_tie5_wlscale2.0_tiescale1.0/"
    c = random.sample(range(52), 5)
    flop_n = zp.ctoi3[c[0]][c[1]][c[2]]
    hand_n = zp.ctoi2[c[3]][c[4]]
    flop = numpy.memmap(path+str(flop_n)+".ftype", numpy.float64, "r", shape = (zp.HANDS, tie_slots, wl_slots))
    #flop = numpy.fromfile(path+str(flop_n)+".ftype", numpy.float64)
    #flop = flop.reshape((zp.HANDS, tie_slots, wl_slots))
    #print len(flop)
#    hand_n = random.randint(0,zp.HANDS-1)

        #print flop[hand_n]
#        hand_n = random.randint(0,zp.HANDS-1)
    #print "sum", flop[hand_n].sum()
    retval = flop[hand_n]
    flop.close()
    return retval

def get_difference(t1, t2):
    if use_C:
        return zp.get_difference(t1,t2)
    return (abs(t1-t2)).sum()
    

def calc_diffs(types, diffs, t):
    if use_C:
        return zp.calc_diffs(types,diffs,t)
        
    small = 1000000000
    for i,x in enumerate(types):
        if i == t:
            diffs[t][t] = 1000000000
            continue
        diffs[t][i] = get_difference(types[t], types[i]);
        diffs[i][t] = diffs[t][i];
        
        if diffs[t][i] < small:
            small = diffs[t][i];
    return small

def get_avg_diff(diff):
    tot = 0.0
    for x in diff:
        if x >= 1000000000:
            continue
        tot += x
    return tot/float(len(diff)-1.0)

def get_closest_match(diffs):
    if use_C:
        return zp.get_closest_match(diffs)
    small = 1000000000
    small_list = []
    n_types = len(diffs)
    #print "n_types", n_types
    i = 0
    while i < n_types:
        j = i+1
        while j < n_types:
            diff = diffs[i][j]
            if diffs[i][j] < small:
                small = diff
                small_list = [(i,j)]
            elif diff == small:
                small_list.append((i,j))
            j += 1
        i += 1
    return small, small_list


def print_type(t):
    largest = 0.0
#    for x in t:
#        if x > largest:
#            largest = x
    for x in t:
        print "#"*int(30*x/t.max()), x
