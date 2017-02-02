import random
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


class types_data_generator:
    def __init__(self, zp, queue_size = 126, update_items = 64):
        self.zp = zp
        self.queue_size = queue_size
        self.update_items = update_items
        self.wait = True
        self.td_queue = Queue.Queue(queue_size)
        self.td_gen_thread = threading.Thread(target=self.thread_loop)
        self.td_gen_thread.daemon = True
        self.td_gen_thread.start()
        
        
    def thread_loop(self):
        while True:
            while self.wait:
                time.sleep(1)
            if self.td_queue.qsize() < self.queue_size - self.update_items:
                print "TD GEN cannot keep up", self.td_queue.qsize(),self.queue_size, self.update_items
            hand_data_list = self.zp.get_random_traverse_jbs(n_items = self.update_items)
            for item in hand_data_list:
                td = np.zeros((1,), dtype = dt.types_data_np)
                td[0]["public_types"][7:10] = item[0][1:4][::-1] #reversed, cause old data format fuck
                td[0]["private_types"][:] = item[1][:]
                td[0]["vals"][:] = item[2][:]

                #td = dt.types_data_np
                #td["public_types"][7:10] = item[0][1:4][::-1] #reversed, cause old data format fuck
                #td["private_types"][:] = item[1][:]
                #td["vals"][:] = item[2][:]
                #print "queue put", self.td_queue.qsize()
                self.td_queue.put(td, block=True)

    def get_new_td(self):
        return self.td_queue.get(block=True)

    def wait_until_empty(self):
        self.wait = True
        while self.td_queue.qsize() > 0:
            print "queue size", self.td_queue.qsize()
            time.sleep(0.5)
        time.sleep(1)


class adjuster:
    def __init__(self, p, workers, target_hand_count, target_priv_node_count, pub_delete_odds, hand_delete_odds, hand_expand_odds, pub_expand_odds, adjust_every ):
        self.workers = workers
        self.p = p
        self.target_hand_count = target_hand_count
        self.target_priv_node_count = target_priv_node_count
        
        self.next_adjust = adjust_every
        self.adjust_every = adjust_every
        self.pub_delete_odds = pub_delete_odds
        self.hand_delete_odds = hand_delete_odds
        self.hand_expand_odds = hand_expand_odds
        self.pub_expand_odds = pub_expand_odds
        self.rng = []
        for i in xrange(64):
            self.rng.append(lzp.get_rng())
        
        self.run = True
        if self.target_hand_count > 0:
            self.hc_pid = pid.PID(1.0, 0.0, 30.0)
            self.hc_pid.setPoint(self.target_hand_count)
        else:
            self.hc_pid = None
        if self.target_priv_node_count > 0:
            self.nc_pid = pid.PID(1.0, 0.0, 10.0)
            self.nc_pid.setPoint(self.target_priv_node_count)
        else:
            self.nc_pid = None
        self.work_thread = threading.Thread(target=self.work_loop)
        self.work_thread.daemon = True
        self.work_thread.start()

        self.adjust_retval = np.zeros(3, dtype=np.uint64)

    def set_target(self, target):
        self.target_hand_count = target
        self.pid.setPoint(self.target_hand_count)

    def adjust_func(self, us, ret_queue):
        adjust_retval = np.zeros(3, dtype=np.uint64)
        lzp.adjust_plrmodel_tree(us.unique_root_node.model_tree, self.pub_delete_odds, self.hand_delete_odds, self.rng[us.id], adjust_retval.ctypes.data_as(C.POINTER(C.c_uint64)))
        ret_queue.put(adjust_retval)

    def work_loop(self):
        while self.run:
            iters = 0
            for w in self.workers:
                iters += w.n_iter
            if iters > self.next_adjust and self.adjust_every > 0:
                hands = 0
                priv_nodes = 0
                pub_nodes = 0
                adjust_start = time.time()
                hc_adjust = 0
                nc_adjust = 0
                print "start adjust", iters
                ret_queue = Queue.Queue()
                threads = []
                self.adjust_retval.fill(0)
                for us in self.p.iter_all_unique_states():
                    if us.gamestate > 0 and us.gamestate < 4:
                        threads.append(threading.Thread(target=self.adjust_func, args=(us, ret_queue)))
                        threads[-1].start()
                        #lzp.adjust_plrmodel_tree(us.unique_root_node.model_tree, self.delete_odds, self.rng, self.adjust_retval.ctypes.data_as(C.POINTER(C.c_uint64)))
                        #hands += self.adjust_retval[0]
                        #priv_nodes += self.adjust_retval[1]
                        #pub_nodes += self.adjust_retval[2]
                for t in threads:
                    t.join()
                while True:
                    try:
                        ar = ret_queue.get(False)
                        #print "adding", ar
                        self.adjust_retval += ar
                        
                    except Queue.Empty:
                        break
                hands = self.adjust_retval[0]
                priv_nodes = self.adjust_retval[1]
                pub_nodes = self.adjust_retval[2]
              
                print "ADJUST", hands, priv_nodes, float(hands)/priv_nodes, self.target_hand_count, self.target_priv_node_count, self.pub_delete_odds, self.hand_delete_odds, time.time()-adjust_start
                if self.hc_pid:
                    if self.target_priv_node_count > 0:
                        self.hc_pid.set_point = (self.target_hand_count/self.target_priv_node_count)*priv_nodes
                        if self.hc_pid.set_point > self.target_hand_count:
                            self.hc_pid.set_point = self.target_hand_count
                        print "set point", (self.target_hand_count/self.target_priv_node_count)*priv_nodes
                    else:
                        self.hc_pid.set_point = self.target_hand_count
                    hc_adjust = self.hc_pid.update(hands)
                    if self.target_hand_count+hc_adjust > 0:
                        self.hand_expand_odds *= float(self.target_hand_count+hc_adjust)/float(self.target_hand_count)
                    else:
                        self.hand_expand_odds *= 1.0/self.target_hand_count
                    if self.hand_expand_odds < 0.0001:
                        self.hand_expand_odds = 0.0001
                    if self.hand_expand_odds > 100000:
                        self.hand_expand_odds = 100000
                    
                if self.nc_pid:
                    nc_adjust = self.nc_pid.update(priv_nodes)
                    if self.target_priv_node_count+nc_adjust > 0:
                        self.pub_expand_odds*= float(self.target_priv_node_count+nc_adjust)/float(self.target_priv_node_count)
                    else:
                        self.pub_expand_odds = 0.000001
                    if self.pub_expand_odds > 1000:
                        self.pub_expand_odds = 1000
                    if self.pub_expand_odds < 0.000001:
                        self.pub_expand_odds = 0.000001
                

                print "PID", hc_adjust, nc_adjust

                # self.hand_expand_odds += dp_adjust
                # if self.hand_expand_odds < 0:
                #     self.hand_expand_odds = 0
                # if self.hand_expand_odds > 100:
                #     self.hand_expand_odds = 100

                # self.pub_expand_odds -= pp_adjust
                # if self.pub_expand_odds < 0:
                #     self.pub_expand_odds = 0
                # if self.pub_expand_odds > 1000:
                #     self.pub_expand_odds = 1000
                #self.delete_odds *= float(hands)/float(self.target_hand_count)
                #self.hand_expand_odds /= float(hands)/float(self.target_hand_count)
                    
                print "NEW ODDS", self.pub_delete_odds, self.hand_delete_odds, self.hand_expand_odds
                
                
                print "NEW ODDS", self.pub_expand_odds
                for w in self.workers:
                    w.hand_expand_odds = self.hand_expand_odds
                    w.pub_expand_odds = self.pub_expand_odds
                    #w.pub_expand_odds = 0
                self.next_adjust += self.adjust_every
            time.sleep(1.0)
            
    
class worker:
    def __init__(self, p, flags, stake_adjust = None, start_running = True, hand_expand_odds = 0.0, pub_expand_odds = 0.0):
        #self.td_gen = td_gen
        #self.td_gen = types_data_generator(p.zp)
        self.p = p
        self.hand_expand_odds = hand_expand_odds 
        self.pub_expand_odds = pub_expand_odds
        self.rng = lzp.get_rng()
        
        self.start_us = C.pointer(p.root_state.next_states[0].next_states[0].unique_root_node)
        self.start_hw = np.ones((2,defs.SAMPLES), dtype=np.float64)
        self.start_hw_ctypes = (C.POINTER(C.c_double)*2)()
        self.start_hw_ctypes_p = C.pointer(self.start_hw_ctypes)
        self.start_hw_ctypes[0] = self.start_hw[0].ctypes.data_as(C.POINTER(C.c_double))
        self.start_hw_ctypes[1] = self.start_hw[1].ctypes.data_as(C.POINTER(C.c_double))
        self.start_pw = self.start_hw.sum(axis=1)
        self.start_pw_ctypes_p = self.start_pw.ctypes.data_as(C.POINTER(C.c_double))
        self.start_stake = np.array((1.0, 0.5), dtype=np.float64)
        #self.start_stake = np.array((0.0, 0.0), dtype=np.float64)
        self.start_stake_ctypes_p = self.start_stake.ctypes.data_as(C.POINTER(C.c_double))
        if stake_adjust != None:
            self.stake_adjust = stake_adjust
        else:
            self.stake_adjust = np.ones((2,3), dtype=np.float64)
        #self.flags = np.zeros((2), dtype=np.uint64)
        self.flags = flags
        self.mem_list = C.pointer(C.POINTER(dt.mem_list)())
        #self.flags_ctypes_p = self.flags.ctypes.data_as(C.POINTER(C.c_uint64))
        #self.flags |= defs.POV_ODDS_FROM_REGS
        #self.flags |= defs.UPDATE_REGS
        #self.flags |= defs.UPDATE_AVG_ODDS
        self.n_iter = 0
        self.run = start_running
        self.kill = False
        self.work_thread = threading.Thread(target=self.work_loop)
        self.work_thread.daemon = True
        self.work_thread.start()
        pass

    def calc_one_iteration(self, td):
        us = p.root_state.next_states[0].next_states[0].unique_root_node
        hw = (C.POINTER(C.c_double)*2)()
        np_hw = np.ones((2,defs.SAMPLES), dtype=np.float64)
        hw[0] = np_hw[0].ctypes.data_as(C.POINTER(C.c_float))
        hw[1] = np_hw[1].ctypes.data_as(C.POINTER(C.c_float))
        pw = np_hw.sum(axis=1)
        


    def work_loop(self):
        self.ev = np.zeros(0, dtype=np.float64)
        avg_ev = np.zeros((2,1326), dtype=np.float64)
        avg_ev_count = np.zeros((2,1326), dtype=np.uint64)
        ret_ev = np.zeros((2,defs.SAMPLES), dtype=np.float64)
        ret_ev_ctypes = (C.POINTER(C.c_double)*2)()
        ret_ev_ctypes_p = C.pointer(ret_ev_ctypes)
        ret_ev_ctypes[0] = ret_ev[0].ctypes.data_as(C.POINTER(C.c_double))
        ret_ev_ctypes[1] = ret_ev[1].ctypes.data_as(C.POINTER(C.c_double))
        td = np.zeros(1, dtype=dt.types_data_np)
        deck = np.arange(52, dtype=np.int8)
        #td = td_gen.get_new_td()
        self.time_start = time.time()
        self.time_getting_td = 0.0
        self.time_in_walk_tree = 0.0
        while not self.kill:
            while not self.run:
                time.sleep(1)
                if self.kill:
                    break
            #np.random.shuffle(deck)
            tmpd = range(52)
            deck[0] = tmpd.pop(random.randint(0,len(tmpd)-1))
            deck[1] = tmpd.pop(random.randint(0,len(tmpd)-1))
            deck[2] = tmpd.pop(random.randint(0,len(tmpd)-1))
            deck[3] = tmpd.pop(random.randint(0,len(tmpd)-1))
            deck[4] = tmpd.pop(random.randint(0,len(tmpd)-1))
            lzp.gen_types_data(C.pointer(self.p.zp.i), deck[:5].ctypes.data_as(C.POINTER(C.c_int8)), td.ctypes.data)

            ret_ev.fill(0)
            tmp_time = time.time()
            #self.pub_expand_odds = 0
            ret = lzp.walk_tree(self.start_us, td, ret_ev_ctypes_p, self.start_hw_ctypes_p, self.start_pw_ctypes_p, 1, self.start_stake_ctypes_p, self.stake_adjust.ctypes.data_as(C.POINTER(C.c_double)), 1.5, self.flags.ctypes.data_as(C.POINTER(C.c_uint64)), self.rng, self.hand_expand_odds, self.pub_expand_odds, self.mem_list)
            self.time_in_walk_tree += time.time()-tmp_time
            self.ev = np.append(self.ev, np.average((ret_ev/990.0), axis=1))
            self.n_iter += 1
            lzp.free_type_count_from_types_data(td)
            #print np.average(ev.reshape(-1,2), axis=0)
            #pdb.set_trace()
            # si = td["vals"]["sample_i"][0]
            # cards = td["vals"]["c"][0]
            # hands = ct.ctoi2[cards[:,0], cards[:,1]]
            # ret_np = [None]*2
            # ret_np[0] = np.ctypeslib.as_array(ret[0][0], shape=(1081,))
            # ret_np[1] = np.ctypeslib.as_array(ret[0][1], shape=(1081,))
            # #pdb.set_trace()
            # avg_ev[0,hands] += ret_np[0][si]/990.0
            # avg_ev_count[0,hands] += 1
            # avg_ev[1,hands] += ret_np[1][si]/990.0
            # avg_ev_count[1,hands] += 1
        
            # print np.average(avg_ev[0]/avg_ev_count[0])
            # print np.average(avg_ev[1]/avg_ev_count[1])

            # ev[0].append(np.average(ret_np[0]/990.0))
            # ev[1].append(np.average(ret_np[1]/990.0))
            # print sum(ev[0])/len(ev[0])
            # print sum(ev[1])/len(ev[1])
            #pdb.set_trace()
            #print ret

        
    def lock_work_queue(self):
        self.queues_locked = True
        return lzp.zp_queue_lock(self.control_args.receiver_to_worker_queue)
    
    def unlock_work_queue(self):
        if self.queues_locked == True:
            lzp.zp_queue_unlock(self.control_args.receiver_to_worker_queue)


    def lock_work_queue_write(self):
        #self.queues_locked = True
        lzp.queue_lock_write(self.control_args.receiver_to_worker_queue)
    
    def unlock_work_queue_write(self):
        lzp.queue_unlock_write(self.control_args.receiver_to_worker_queue)

        

class worker_control:
    def __init__(self, p, target_hand_count = 0, target_priv_node_count = 0, pub_expand_odds = 0.0, hand_expand_odds  = 0.0, pub_delete_odds = 0.0, hand_delete_odds = 0.0):
        self.p = p
        self.target_hand_count = target_hand_count
        self.target_priv_node_count = target_priv_node_count
        self.pub_expand_odds = pub_expand_odds
        self.hand_expand_odds = hand_expand_odds 
        self.hand_delete_odds = hand_delete_odds
        self.pub_delete_odds = pub_delete_odds
        self.workers = []
        self.flags = np.zeros((2), dtype=np.uint64)
        self.flags |= defs.POV_ODDS_FROM_REGS
        self.flags |= defs.UPDATE_REGS
        self.stake_adjust = np.ones((2,3), dtype=np.float64)
        
        if self.target_hand_count > 0:
            self.adj = adjuster(p, self.workers, self.target_hand_count, self.target_priv_node_count, self.pub_delete_odds, self.hand_delete_odds, self.hand_expand_odds, self.pub_expand_odds, 200)
        else:
            self.adj = None
            self.flags |= defs.UPDATE_AVG_ODDS
        
        self.workers_running = False
        
    def get_iter_count(self):
        iters=0
        for w in self.workers:
            iters += w.n_iter
        return iters
        
    def start_workers(self):
        self.workers_running = True
        for w in self.workers:
            w.run = True

    def stop_workers(self):
        self.workers_running = False
        for w in self.workers:
            w.run = False

    def kill_workers(self):
        while len(self.workers) > 0:
            self.delete_worker()
    
    def add_worker(self):
        self.workers.append(worker(self.p, self.flags, self.stake_adjust, self.workers_running, self.hand_expand_odds, self.pub_expand_odds))

    def delete_worker(self, index=0):
        w = self.workers.pop(0)
        w.kill = True
        

    def set_worker_count(self, count):
        while count > len(self.workers):
            self.add_worker()
        while count < len(self.workers):
            self.delete_worker()

    def get_ev(self):
        ev = [0.0,0.0]
        ev_c = [0,0]
        for w in self.workers:
            ev[0] += w.ev[0::2].sum()
            ev_c[0] += w.ev.size/2
            ev[1] += w.ev[1::2].sum()
            ev_c[1] += w.ev.size/2
        return ev[0]/ev_c[0], ev[1]/ev_c[1]
    
if __name__ == "__main__":
    gamedir = sys.argv[1]
    model_name = sys.argv[2]
    new_model_name = sys.argv[3]
    n_workers = int(sys.argv[4])
    
    target_hand_count = int(sys.argv[5])
    p = plrmodel.plrmodel(gamedir)
    #p.model_name = model_name
    p.gen_plrmodel_tree_for_all_us()
    p.reset_regs()
    p.zp.load_slots_for_all_types()
    p.zp.load_diffs_for_all_types()
    #p.expand_models_test()
    #p.expand_models_until_n_hands_random(10000, 0)
#    pdb.set_trace()

    if model_name == "new":
        p.gen_plrmodel_tree_for_all_us()
        for us in p.iter_all_unique_states():
            if us.gamestate < 4 and us.id >= 2:
                print "expanding", us.state
                if us.gamestate == 0:
                    us.expand_type_max(0,us.gamestate)
                us.expand_type_max(1,6)
                us.expand_type_max(1,5)        
                us.expand_type_max(1,4)
                us.expand_type_max(1,3)
                us.expand_type_max(1,2)
                us.expand_type_max(1,1)
                
    else:
        p.load_model(model_name)
    
    #td_gen = types_data_generator(p.zp)
    workers = []
    flags = np.zeros((2), dtype=np.uint64)
    flags |= defs.POV_ODDS_FROM_REGS
    flags |= defs.UPDATE_REGS
    flags |= defs.UPDATE_AVG_ODDS
       
    for x in xrange(n_workers):
        workers.append(worker(p, flags))

    adj = adjuster(p, workers, target_hand_count)
    
    save_every = 100000
    next_save = 100000
    prev_iter_count = 0
    prev_iter_time = time.time()
    while True:
        sel_ret = select.select([sys.stdin], [],[],5.0)

        if sys.stdin in sel_ret[0]:
            inp = raw_input()
            if inp == "debug":
                pdb.set_trace()
            elif inp == "quit":
                break
        iters = 0
        for w in workers:
            iters += w.n_iter
        print "iters per second", (iters-prev_iter_count)/(time.time()-prev_iter_time)
        
        prev_iter_count = iters
        prev_iter_time = time.time()
        
        if iters > next_save:
            print "Saving"
            td_gen.wait_until_empty()
            p.reset_model_name(new_model_name+"_"+str(iters))
            p.save_model()
            next_save += save_every
            td_gen.wait = False
            
