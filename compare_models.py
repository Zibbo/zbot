import datatypes as dt
import lzp
import ctypes as C
import plrmodel
import sys
import time
import pdb
import socket
import select
import struct
import defs
import os
import Queue
import threading
import numpy as np
import card_table as ct
import pid
import cPickle

import worker2



def calc_ev(gamedir, seat0_model_name, seat1_model_name, n_workers, n_iters, source = ["BYTE", "BYTE"], report_every=0):
    p = plrmodel.plrmodel(gamedir)
    p.reset_model_path(seat0_model_name)
    p.load_struct(seat=0)
    p.recount_data_index(seat=0)

    if source[0] == "BYTE_AVG":
        p.mmap_byte_odds(seat=0)
        p.set_byte_odds_from_mmap(seat=0)
    elif source[0] == "BYTE_REGS":
        p.mmap_byte_odds(seat=0,source="REGS")
        p.set_byte_odds_from_mmap(seat=0)
    elif source[0] == "AVG":
        p.load_avg_odds(seat=0)
        #p.mmap_avg_odds(seat=0)
        #p.set_avg_odds_from_mmap(seat=0)
    elif source[0] == "REGS":
        p.load_regs(seat=0)
        #p.mmap_regs(seat=0)
        #p.set_regs_from_mmap(seat=0)
        
    p.reset_model_path(seat1_model_name)
    p.load_struct(seat=1)
    p.recount_data_index(seat=1)
    if source[1] == "BYTE_AVG":
        p.mmap_byte_odds(seat=1)
        p.set_byte_odds_from_mmap(seat=1)
    elif source[1] == "BYTE_REGS":
        p.mmap_byte_odds(seat=1,source="REGS")
        p.set_byte_odds_from_mmap(seat=1)
    elif source[1] == "AVG":
        p.load_avg_odds(seat=1)
        #p.mmap_avg_odds(seat=1)
        #p.set_avg_odds_from_mmap(seat=1)
    elif source[1] == "REGS":
        p.load_regs(seat=1)
        #p.mmap_regs(seat=1)
        #p.set_regs_from_mmap(seat=1)
    
    p.zp.load_slots_for_all_types()
    p.zp.load_diffs_for_all_types()
    
    wc = worker2.worker_control(p)

    if source[0] == "BYTE_AVG" or source[0] == "BYTE_REGS":
        wc.flags[0] = defs.POV_ODDS_FROM_BYTE_ODDS
    elif source[0] == "AVG":
        wc.flags[0] = defs.POV_ODDS_FROM_AVG
    elif source[0] == "REGS":
        wc.flags[0] = defs.POV_ODDS_FROM_REGS
    if source[1] == "BYTE_AVG" or source[1] == "BYTE_REGS":
        wc.flags[1] = defs.POV_ODDS_FROM_BYTE_ODDS
    elif source[1] == "AVG":
        wc.flags[1] = defs.POV_ODDS_FROM_AVG
    elif source[1] == "REGS":
        wc.flags[1] = defs.POV_ODDS_FROM_REGS

    wc.set_worker_count(n_workers)
    wc.start_workers()

    next_report = report_every
    iters = 0
    prev_iter_count = 0
    prev_iter_time = time.time()
    while iters < n_iters:
        iters = wc.get_iter_count()
        if iters >= next_report:
            next_report += report_every
            print wc.get_ev()
        time.sleep(1)
    retval = wc.get_ev()
    wc.kill_workers()
    time.sleep(3)
    return retval



if __name__ == "__main__":
    
    gamedir = sys.argv[1]
    n_workers = int(sys.argv[2])
    n_iters = int(sys.argv[3])
    report_every = int(sys.argv[4])
    res_file = sys.argv[5]
    model_names = sys.argv[6:]
    print model_names
    n_models = len(model_names)
    try:
        f = file(res_file, "rb")
        res_dic = cPickle.load(f)
        f.close()
    except:
        res_dic = {}
        
    results = np.zeros((n_models, n_models), dtype=np.float64)
    for x in xrange(n_models):
        for y in xrange(n_models):
            if (model_names[x], model_names[y]) in res_dic:
                results[x,y] = res_dic[(model_names[x], model_names[y])][1]
                continue
            print "EV FOR",  model_names[x], model_names[y]
            res1 = calc_ev(gamedir, model_names[x], model_names[y], n_workers, n_iters, ["BYTE_AVG", "BYTE_AVG"], report_every)
            #res2 = calc_ev(gamedir, model_names[y], model_names[x], n_workers, n_iters,  ["BYTE", "BYTE"], report_every)
            results[x,y] = res1[1]
            res_dic[(model_names[x], model_names[y])] = res1
            f = file(res_file, "wb")
            cPickle.dump(res_dic, f)
            f.close()
            #results[y,x] = res1[1] + res2[0]
            print results
