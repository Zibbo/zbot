import copy
import time
import socket
import select
import struct
import sys
import numpy as np
import Queue
import threading


import datatypes as dt
import pdb
import os
import card_table as ct
import plrmodel
import defs

class client:
    def __init__(self, s, ctrl_address, pm):
        self.ctrl_s = s
        self.data_s = None
        self.ctrl_address = ctrl_address
        self.data_address = None
        self.worker_queue_size = 0
        self.pm = pm

        self.data_send_queue = Queue.Queue(100)
        self.ctrl_send_queue = Queue.Queue(100)

        self.sd_thread = threading.Thread(target=self.send_data_thread)
        self.sc_thread = threading.Thread(target=self.send_ctrl_thread)
        self.rc_thread = threading.Thread(target=self.recv_ctrl_thread)

        self.sd_thread.daemon = True
        self.sc_thread.daemon = True
        self.rc_thread.daemon = True

        self.sd_thread.start()
        self.sc_thread.start()
        self.rc_thread.start()
        
        
        
    def send_data_thread(self):
        print "client send data thread started"
        while True:
            msg = self.data_send_queue.get()
            if type(msg) == str:
                self.data_s.send(msg)
            else:
                for part in msg:
                    self.data_s.send(part)

    def send_ctrl_thread(self):
        print "client send ctrl thread started"
        while True:
            msg = self.ctrl_send_queue.get()
            self.ctrl_s.send(msg)

    
    def recv_ctrl_thread(self):
        print "client recv thread started"
        while True:
            data_raw, address = self.ctrl_s.recvfrom(4, socket.MSG_WAITALL)
            if len(data_raw) == 0:
                print "Client closed connection", self.ctrl_address
                return -1
            #print len(data_raw), address
            msg_id = struct.unpack("I", data_raw)[0]
            #print msg_id
            if msg_id == 0xffffffff:
                data_raw, address = self.ctrl_s.recvfrom(2, socket.MSG_WAITALL)
                data_port = struct.unpack("H", data_raw)[0]
                print data_port
                self.data_address = (self.ctrl_address[0], data_port)

                self.data_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.data_s.bind(("", 0))
                print "data address", self.data_address
                self.data_s.connect(self.data_address)

            elif msg_id == 12:
                data_raw = self.ctrl_s.recv(4, socket.MSG_WAITALL)
                self.worker_queue_size = struct.unpack("I", data_raw)[0]
                #print "got worker queue", self.worker_queue_size
            elif msg_id == 13: #avg_fitness
                data_raw = self.ctrl_s.recv(12, socket.MSG_WAITALL)
                print "fitness", struct.unpack("=Id", data_raw)
            elif msg_id == 14: #avg odds change
                data_raw = self.ctrl_s.recv(8, socket.MSG_WAITALL)
                print "avg_odds_change", struct.unpack("d", data_raw)

            elif msg_id == 20: #reduce model retval
                data_raw = self.ctrl_s.recv(12, socket.MSG_WAITALL)
                us_id, start_hc, end_hc = struct.unpack("=III", data_raw)

                cl = self.pm.us_by_id(us_id).hand_counts_reduce = [start_hc, end_hc]

            elif msg_id == 21: #expand model retval
                data_raw = self.ctrl_s.recv(12, socket.MSG_WAITALL)
                us_id, start_hc, end_hc = struct.unpack("=III", data_raw)
                cl = self.pm.us_by_id(us_id).hand_counts_expand = [start_hc, end_hc]

            elif msg_id == 100: #plrmodel save
                data_raw = self.ctrl_s.recv(8, socket.MSG_WAITALL)
                us_id, file_len = struct.unpack("=II", data_raw)
                filename = self.model_path + str(us_id) + "_" + str(self.pm.zp.int_to_us[us_id]) + ".model_tree"
                print "receiving model", filename, file_len
                f = file(filename, "wb")
                while file_len > 0:
                    if file_len > 1024*1024:
                        read_len = 1024*1024
                    else:
                        read_len = file_len
                    data_raw = self.ctrl_s.recv(read_len, socket.MSG_WAITALL)
                    f.write(data_raw)
                    file_len -= len(data_raw)
                    assert len(data_raw) == read_len
                    assert file_len >= 0
                f.close()

                #return msg_id

    def gen_msg_id(self, id):
        return struct.pack("I", id)
        
    def send_ctrl_message(self, id = None, data = ""):
        if id == None:
            msg_id = ""
        else:
            msg_id = struct.pack("I", id)
        #self.ctrl_s.send(msg_id+data)
        self.ctrl_send_queue.put(msg_id+data)
        
    def send_data_message(self, id = None, data  = ""):
        if id == None:
            msg_id = ""
        else:
            msg_id = struct.pack("I", id)
        #self.data_s.send(msg_id + data)
        self.data_send_queue.put(msg_id+ data)



    def preload_types_data(self, td):
        self.send_data_message(1,td.tostring())

    def clean_types_data(self, td_id):
        self.send_data_message(3, struct.pack("i", td_id))

    def send_worker_message(self, wm, hwev, v):
        #self.send_data_message(id=2, data = (wm,hwev,v))
        self.data_send_queue.put((self.gen_msg_id(2),wm,hwev,v))
        #self.data_send_queue.put(hwev)
        #self.data_send_queue.put(v)
        #self.send_data_message(data=wm)
        #self.send_data_message(data=hwev)
        #self.send_data_message(data=v)
        
        #self.data_s.send(wm)
        #self.data_s.send(hwev)
        #self.data_s.send(v)
    
    def get_worker_queue_status(self):
        self.send_ctrl_message(12, "")

    def get_avg_fitness(self, timestamp):
        self.send_ctrl_message(13, struct.pack("Q", timestamp))

    def adjust_model(self, us_id, lo_limit, hi_limit, aod_limit,timestamp):
        self.send_ctrl_message(20, struct.pack("=IdddQ", us_id, lo_limit, hi_limit, aod_limit, timestamp))

    def expand_model(self, us_id, multiplier):
        self.send_ctrl_message(21, struct.pack("=Id", us_id, multiplier))

    def set_handler(self, us_id, handler_addr):
        msg = struct.pack("i16sH", us_id, handler_addr[0], handler_addr[1])
        self.send_ctrl_message(2, msg)

    def save_model(self, us_id):
        self.send_ctrl_message(100, struct.pack("=I", us_id))

    def load_model(self, us_id, model_path):
        filename = model_path + str(us_id) + "_" + str(self.pm.zp.int_to_us[us_id]) + ".model_tree"
        filesize = os.path.getsize(filename)
        msg = struct.pack("=II", 101, us_id)
        self.ctrl_s.send(msg)
        f = file(filename, "rb")
        msg = f.read(1024)
        tot_len = 0
        tot_sent = 0
        while len(msg) > 0:
            tot_len += len(msg)
            tot_sent += self.ctrl_s.send(msg)
            msg = f.read(1024)
        assert(tot_len == tot_sent and tot_len == filesize)
        f.close()
    def reset(self, us_id, reset_flags):
        msg = struct.pack("=IQ", us_id, reset_flags)
        self.send_ctrl_message(3,msg)

    def update_avg_odds(self):
        print "send update avg odds"
        self.send_ctrl_message(14)

class types_data_generator:
    def __init__(self, zp, queue_size = 1000, update_items = 500):
        self.zp = zp
        self.queue_size = queue_size
        self.update_items = update_items
        
        self.td_queue = Queue.Queue(queue_size)
        self.td_gen_thread = threading.Thread(target=self.thread_loop)
        self.td_gen_thread.daemon = True
        self.td_gen_thread.start()
        
        
    def thread_loop(self):
        while True:
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
                self.td_queue.put(td[0], block=True)

    def get_new_td(self):
        return self.td_queue.get(block=True)

        
class ctrl:
    def __init__(self, gamedir):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.s.bind(("", 11111))
        self.s.listen(32)

        self.data_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.data_s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.data_s.bind(("", 11112))
        self.data_s.listen(32)
        self.clients_by_socket = {}
        self.data_con = {}
        self.read_wait_list = [self.s, self.data_s, sys.stdin]

        self.p = plrmodel.plrmodel(gamedir)

        self.hand_start_wm = np.zeros((1,), dtype = dt.worker_message_np)[0]

        self.hand_start_wm["n_variations"] = 1

        self.queue_limit = 0
        self.old_queue_limit = 0
        self.iter_per_send = 100
        self.next_send = time.time()
        self.timeout = 0.1
        self.avg_time_sending = 0.01
        self.avg_time_per_send = 0
        self.total_iterations = 0
        self.wm_flags = 0
        self.lo_limit = 0.001
        self.hi_limit = 0.001
        self.aod_limit = 0.2
        self.td_id_count = 0
        self.timestamp = 0

        self.items_processing = {}
        self.tot_odds_count = np.zeros(2, dtype=np.float64)
        self.tot_odds_change = np.zeros(2, dtype=np.float64)

        self.adjust_state = 0
        self.adjust_limit = 0.0
        self.target_hand_count = 20000
        self.target_reduce_ratio = 0.80

        self.model_name = ""
        self.flags = [0]*self.p.n_plr

        self.set_flag(defs.POV_ODDS_FROM_REGS)
        self.set_flag(defs.NONPOV_ODDS_FROM_REGS) 
        #set_flag(defs.UPDATE_EV, flags)
        #set_flag(defs.UPDATE_AVG_ODDS, flags)
        #set_flag(defs.UPDATE_REGS, flags)
        self.set_flag(defs.POV_UPDATE_VISITS)
        self.set_flag(defs.NONPOV_UPDATE_VISITS)

        self.avg_ev = np.zeros((2,1326), dtype=np.float64)
        self.avg_ev_count = np.zeros((2,1326), dtype=np.float64)

        self.ev_tot = [0.0,0.0]
        self.ev_tot_count = [0.0,0.0]
        self.ev_tot2 = [0.0,0.0]
        self.ev_tot_count2 = [0.0,0.0]

        self.active_seats = range(self.p.n_plr)
        self.td_dic = {}
        self.uao = 0
        self.uao_every = 10000 
        self.uao_last = 0
        self.no_expand = False
        #self.#adjust_every = 3*target_hand_count
        self.adjust_every = 0
        self.count_ev = False
        self.last_td_time = time.time()

        self.td_gen = types_data_generator(self.p.zp, 2000, 200)
        
        self.dic_lock = threading.Lock()


    def setup_us_handling_clients(self):
        client_list = self.clients_by_socket.values()
        n_clients = len(client_list)

        if n_clients == 1:
            for us in self.p.iter_all_unique_states():
            
                if us.id < 2:
                    us.handling_client = None
                    client_list[0].set_handler(us.id, ("127.0.0.1", 11112))
                else:
                    us.handling_client = client_list[0]
                
        elif n_clients == 2:
            for us in self.p.iter_all_unique_states():
                if us.id < 2:
                    handler_addr = ("127.0.0.1", 11112)
                elif us.gamestate == 4:
                    continue
                elif us.gamestate == 3:
                    us.handling_client = client_list[0]
                    handler_addr = client_list[0].data_address
                else:
                    us.handling_client = client_list[1]
                    handler_addr = client_list[1].data_address
                for client in client_list:
                    if client.data_address != handler_addr:
                        client.set_handler(us.id, handler_addr)

        elif 0 and n_clients == 2:
            for us in self.p.iter_all_unique_states():
                if us.id < 2:
                    handler_addr = ("127.0.0.1", 11112)
                elif us.gamestate == 4:
                    continue
                else:
                    us.handling_client = client_list[us.cur_seat]
                    handler_addr = client_list[us.cur_seat].data_address
                
                for client in client_list:
                    if client.data_address != handler_addr:
                        client.set_handler(us.id, handler_addr)


        else:
            print "wrong n_clients", n_clients
            sys.exit(0)
    
    def get_first_decision_variations(self, pov_seat):
        us = self.p.root_state.next_states[0].next_states[0]

        if us.cur_seat == pov_seat:
            variations = np.zeros((2,), dtype = dt.variation_np)
            variations[0]["path_weight"] = 0.5
            variations[0]["us_id"] = us.next_states[0].id
            variations[0]["path_i"] = 3
            variations[0]["potsize"] = 3.0
            variations[0]["path"][2] = 0
            variations[1]["path_weight"] = 0.5
            variations[1]["us_id"] = us.next_states[1].id
            variations[1]["path_i"] = 3
            variations[1]["potsize"] = 2.0
            variations[1]["path"][2] = 1
        else:
            variations = np.zeros((1,), dtype = dt.variation_np)
            variations[0]["path_weight"] = 1.0
            variations[0]["us_id"] = us.id
            variations[0]["path_i"] = 2
            variations[0]["potsize"] = 1.5

        return variations
    
    def calc_one_iteration(self):
        client_list = self.clients_by_socket.values()
        time_sending = 0.0
        td = self.td_gen.get_new_td()
        #        td_a = gen_random_types_data(self.p.zp)
        #td = td_a[0]
        td["id"] = self.td_id_count
        with self.dic_lock:
            self.td_dic[self.td_id_count] = td
            self.items_processing[self.td_id_count] = 0
            self.td_id_count+=1
        for cl in client_list:
            time_start = time.time()
            cl.preload_types_data(td)
            time_sending += time.time() - time_start
        #time.sleep(1)
        wm = np.zeros((1,), dtype = dt.worker_message_np)[0]
        #wm["pov_seat"] = pov_seat
        #wm["n_variations"] = 1
        #wm["types_data_id"] = td["id"]
        #wm["flags"] = self.flags
        #wm["positive_regs"] = 0.0
        hwev = np.zeros((1,), dtype = dt.hwev_data_np)[0]
        hwev["pw"] = 1.0
        hwev["d"].fill(1.0/defs.SAMPLES)

        for pov_seat in self.active_seats:
            wm = np.zeros((1,), dtype = dt.worker_message_np)[0]
            wm["n_variations"] = 1
            wm["types_data_id"] = td["id"]
            wm["positive_regs"] = 0.0
            n_hwev = 0
            #tmp_wm = copy.copy(wm)
            for x in xrange(defs.MAX_PLR):
                if x != pov_seat:
                    n_hwev += 1
                    wm["hwev_p"][x] = 1
                else:
                    wm["hwev_p"][x] = 0
            wm["pov_seat"] = pov_seat
            wm["flags"] = self.flags[pov_seat]
            variations = self.get_first_decision_variations(pov_seat)
            for v in variations:
                us_tuple = self.p.zp.int_to_us[v["us_id"]]
                cl = self.p.states_dict[us_tuple].handling_client
                time_start = time.time()
                cl.send_worker_message(wm, hwev, v)
                #cl.send_data_message(2, wm.tostring() + n_hwev*hwev.tostring() + v.tostring())
                time_sending += time.time() - time_start
                #return time_sending
        return td["id"]


    def set_flags(self, flag_dic, flags):
        def keyfunc(item):
            return item[1]

        items = flag_dic.items()
        items.sort(key=keyfunc)
        while True:
            for i,flag in enumerate(items):
                print i, flag, flags&flag[1]
            try:
                flag = raw_input("swap flag?")
                flag = int(flag)
            except ValueError:
                break
            if flag >= 0 and flag < len(flag_dic):
                mask = items[flag][1]
                if flags&mask:
                    flags &= ~mask
                else:
                    flags |= mask
            elif flag == -1:
                if flags:
                    flags = 0
                else:
                    flags = 0xfffffffffffffff

        return flags

    def set_flag(self, flag, pos=-1):
        if pos == -1:
            iter_list = range(len(self.flags))
        else:
            iter_list = range(pos, pos+1)
        for i in iter_list:
            self.flags[i] |= flag
    
    def clear_flag(self, flag, pos=-1):
        if pos == -1:
            iter_list = range(len(self.flags))
        else:
            iter_list = range(pos, pos+1)
        for i in iter_list:
            self.flags[i] &= ~flag


    # def set_pov_odds_from(self, src, flags, pos = -1):
    #     self.clear_flag(defs.POV_ODDS_FROM_REGS, flags, pos)
    #     self.clear_flag(defs.POV_ODDS_FROM_AVG, flags, pos)
    #     self.clear_flag(defs.POV_ODDS_FROM_EV, flags, pos)

    #     if src == "REGS":
    #         self.set_flag(defs.POV_ODDS_FROM_REGS, flags, regs)
    #     elif src == "AVG":
    #         self.set_flag(defs.POV_ODDS_FROM_AVG, flags, regs)
    #     elif src == "EV":
    #         self.set_flag(defs.POV_ODDS_FROM_EV, flags, regs)

    # def set_nonpov_odds_from(self, src, flags, pos = -1):
    #     clear_flag(defs.NONPOV_ODDS_FROM_REGS, flags, pos)
    #     clear_flag(defs.NONPOV_ODDS_FROM_AVG, flags, pos)
    #     clear_flag(defs.NONPOV_ODDS_FROM_EV, flags, pos)

    #     if src == "REGS":
    #         set_flag(defs.NONPOV_ODDS_FROM_REGS, flags, regs)
    #     elif src == "AVG":
    #         set_flag(defs.NONPOV_ODDS_FROM_AVG, flags, regs)
    #     elif src == "EV":
    #         set_flag(defs.NONPOV_ODDS_FROM_EV, flags, regs)



    # def set_update_ev(flags, pos = -1):
    #     set_flag(defs.UPDATE_EV, flags, pos)
        
    # def set_update_ev(flags, pos = -1):
    #     if pos == -1:
    #         iter_list = range(len(flags))
    #     else:
    #         iter_list = range(pos, pos+1)
    #     for i in iter_list:
    #         flags[i] &= defs.UPDATE_EV

    def data_socket_receiver_thread(self, s):
        print "receive thread start"
        while True:
            #print "wait message"
            msg = s.recv(dt.worker_message_np.itemsize + 4, socket.MSG_WAITALL)
            #print "got message"
            wm = np.fromstring(msg[4:], dtype=dt.worker_message_np)
            #print wm
            td_id = wm["types_data_id"][0]
            with self.dic_lock:
                td = self.td_dic[td_id]
            for hwev in wm["hwev_p"][0]:
                #print hwev
                if hwev != 0:
                    msg = s.recv(dt.hwev_data_np.itemsize, socket.MSG_WAITALL)
                    if self.count_ev:
                        ev = np.fromstring(msg, dtype=dt.hwev_data_np)
                        seat = wm["pov_seat"][0]
                        ev_d = ev["d"][0]
                        #idxlist = np.where(ev_d > -1000.0)

                        cards = td["vals"]["c"]
                        si = td["vals"]["sample_i"]
                        si_valid = np.where(ev_d[si] > -1000.0)
                        #if (len(si_valid[0]) != defs.SAMPLES):
                        #    print len(si_valid[0])
                            #pdb.set_trace()
                        #print len(si_valid[0])
                        hands = ct.ctoi2[cards[:,0], cards[:,1]]
                        self.avg_ev[seat,hands[si_valid]] += ev_d[si[si_valid]]
                        self.avg_ev_count[seat, hands[si_valid]] += 1

                        self.ev_tot[seat] += np.average(ev_d[si[si_valid]])
                        self.ev_tot_count[seat] += float(len(si_valid[0]))/defs.SAMPLES
                        #pdb.set_trace()
                        self.ev_tot2[seat] += ev["pw"][0]
                        self.ev_tot_count2[seat] += 1
                        if (abs(np.average(ev_d) - ev["pw"][0]) > 0.00001):
                            pdb.set_trace()
                            #print len(idxlist[0]),len(idxlist[1])
                        #avg_ev_count[seat] += 1
                            #avg_ev_count[seat] += len(idxlist[0])/1081.0
                        #print wm["pov_seat"], np.average(ev["d"])
            assert(wm["n_variations"] == 1)
            msg = s.recv(dt.variation_np.itemsize, socket.MSG_WAITALL)

            seat = wm["pov_seat"][0]
            #pdb.set_trace()
            self.tot_odds_count[seat] += 1
            if self.tot_odds_count[seat] > 10000:
                self.tot_odds_count[seat] = 10000
                self.tot_odds_change[seat] *= (1.0-1.0/10000.0)
            self.tot_odds_change[seat] +=  wm["positive_regs"][0]
            #print seat, wm["positive_regs"][0]
            self.dic_lock.acquire()
            self.items_processing[td_id]+=1
            if self.items_processing[td_id] == len(self.active_seats):
                del self.items_processing[td_id]
                del self.td_dic[td_id]
                if not td_id%1000:

                    print "deleted", td_id, self.tot_odds_change, self.tot_odds_count, self.tot_odds_change/self.tot_odds_count, "td per second:", 1000.0/(time.time() - self.last_td_time)
                    self.last_td_time = time.time()
                    if self.count_ev:
                        for i in xrange(2):
                            print self.avg_ev[i]/self.avg_ev_count[i] - (1.0 - i*0.5)
                            #if avg_ev_count[i] != 0:
                            #    print i, avg_ev[i]/avg_ev_count[i]
                        for i in xrange(2):
                            print np.average(self.avg_ev[i]/self.avg_ev_count[i]) - (1.0 - i*0.5)
                            if self.ev_tot_count[i] != 0:
                                print self.ev_tot[i]/self.ev_tot_count[i] - (1.0 - i*0.5)
                            if self.ev_tot_count2[i] != 0:
                                print self.ev_tot2[i]/self.ev_tot_count2[i] - (1.0 - i*0.5)

                for cl in self.clients_by_socket.values():
                    cl.clean_types_data(td_id)
            self.dic_lock.release()
            if self.adjust_state == 0 and self.tot_odds_count.sum() > 100 and self.tot_odds_change.sum()/self.tot_odds_count.sum() < self.adjust_limit:
                print "starting adjust", self.tot_odds_change,self.tot_odds_count, self.tot_odds_change/self.tot_odds_count, self.adjust_limit 
                self.adjust_state = 1

                # if uao and not td_id_count%uao_every and td_id_count != uao_last:
                #     uao_last = td_id_count
                #     for cl in clients_by_socket.values():
                #         cl.update_avg_odds()

    
    def perform_one_loop(self, timeout):
        
        sel_ret = select.select(self.read_wait_list, [], [], timeout)
        for x in sel_ret[0]:
            if x == sys.stdin:
                inp = raw_input()
                print "input:", inp

                if inp == "cont_iter":
                    self.queue_limit = int(raw_input("queue limit?"))

                elif inp == "queue count":
                    for cl in self.clients_by_socket.values():
                        print cl.worker_queue_size

                elif inp == "adjust":
                    print "lo_limit", self.lo_limit
                    print "hi_limit", self.hi_limit
                    print "aod_limit", self.aod_limit
                    print "timestamp", self.timestamp
                    yn = raw_input("Ok?")
                    if yn == "y":
                        # for us in p.iter_all_unique_states():
                        #     if us.handling_client != None and us.gamestate != 4:
                        #         us.handling_client.adjust_model(us.id, lo_limit, hi_limit, aod_limit, timestamp)
                        #         us.hand_counts_reduce = None
                        #     #for cl in clients_by_socket.values():
                        #     #cl.reduce_model(lo_limit, hi_limit, aod_limit, timestamp)
                        # timestamp = 0
                        # queue_limit = 0
                        # tot_odds_count = np.zeros(2, dtype=np.float64)
                        # tot_odds_change = np.zeros(2, dtype=np.float64)
                        self.adjust_state = 1

                elif inp == "expand":
                    multiplier = float(raw_input("Expand multiplier?"))

                    for cl in self.clients_by_socket.values():
                        cl.expand(multiplier)
                    timestamp = 0
                    queue_limit = 0
                elif inp == "set handlers":
                    self.setup_us_handling_clients()
                    #elif inp == "flags":
                    #wm_flags = int(raw_input("flags?"))
                    
                elif inp == "debug":
                    pdb.set_trace()


                elif inp == "flags":
                    while True:
                        try:
                            pos = int(raw_input("pos? "))
                            self.flags[pos] = self.set_flags(defs.wm_flags, self.flags[pos])
                            if pos == -1:
                                self.flags[0] = self.flags[-1]

                        except ValueError:
                            break
                elif inp == "reset":
                    rf = [0,0]
                    while True:
                        try:
                            pos = int(raw_input("pos? "))
                            self.rf[pos] = self.set_flags(defs.reset_flags, self.rf[pos])
                            if pos == -1:
                                self.rf[0] = self.rf[-1]
                        except ValueError:
                            break
                    for us in self.p.iter_all_unique_states():
                        if us.handling_client != None and us.gamestate != 4:
                            us.handling_client.reset(us.id, self.rf[us.cur_seat])

                elif inp == "save":
                    model_path = self.p.zp.gamedir + "plrmodels/plrmodel_" + self.model_name+ time.strftime("_%Y_%m_%d_%H_%M_%S") + "/"
                    #model_path = raw_input("path")
                    try:
                        os.mkdir(model_path)
                    except OSError:
                        pass
                    for cl in self.clients_by_socket.values():
                        cl.model_path = model_path
                    for us in self.p.iter_all_unique_states():
                        if us.handling_client != None and us.gamestate != 4:
                            us.handling_client.save_model(us.id)
                    queue_limit = 0


                elif inp == "load":
                    def cmp_key(filename):
                        return os.stat(filename).st_mtime
                    models = []
                    for i, f in enumerate(os.listdir(self.p.zp.gamedir+"plrmodels/")):
                        if "plrmodel_" in f:
                            models.append(self.p.zp.gamedir+"plrmodels/" + f + "/")
                    models.sort(key=cmp_key)
                    for i, f in enumerate(models):
                        print str(i)+":", f
                    model = int(raw_input("Choose model: "))
                    seat = int(raw_input("Choose seat: "))


                    #model_path = p.zp.gamedir + "plrmodel_" + model_name+ time.strftime("_%Y_%m_%d_%H_%M_%S") + "/"
                    #model_path = raw_input("path")
                    #try:
                    #    os.mkdir(model_path)
                    #except OSError:
                    #    pass
                    #for cl in clients_by_socket.values():
                    #    cl.model_path = 
                    for us in self.p.iter_all_unique_states():
                        if us.handling_client != None and us.gamestate != 4:
                            if us.cur_seat == seat or seat == -1:
                                us.handling_client.load_model(us.id, models[model])
                    queue_limit = 0


                elif inp == "quit":
                    for cl in self.clients_by_socket.values():
                        if cl.ctrl_s != None:
                            cl.ctrl_s.close()
                        if cl.data_s != None:
                            cl.data_s.close()
                    self.s.close()
                    self.data_s.close()
                    sys.exit(0)

            elif x == self.s:
                new_connection = self.s.accept()
                print "new client", new_connection[1]
                new_client = client(new_connection[0], new_connection[1], self.p)

                self.clients_by_socket[new_connection[0]] = new_client
                #self.read_wait_list.append(new_connection[0])

            elif x == self.data_s:
                new_connection = self.data_s.accept()
                print "new data client", new_connection[1]
                self.data_con[new_connection[0]] = new_connection[1]
                #self.read_wait_list.append(new_connection[0])
                new_data_thread = threading.Thread(target=self.data_socket_receiver_thread, args=(new_connection[0],))
                new_data_thread.daemon = True
                new_data_thread.start()

            # elif x in self.clients_by_socket:
            #     cl = self.clients_by_socket[x]
            #     if (cl.recv_data() == -1):
            #         self.read_wait_list.pop(self.read_wait_list.index(cl.ctrl_s))
            #         del self.clients_by_socket[x]
            #         cl.ctrl_s.close()
            #         cl.data_s.close()
            #         del cl


            # elif x in self.data_con:
            #     msg = x.recv(dt.worker_message_np.itemsize + 4, socket.MSG_WAITALL)
            #     wm = np.fromstring(msg[4:], dtype=dt.worker_message_np)
            #     td_id = wm["types_data_id"][0]
            #     td = self.td_dic[td_id]
            #     for hwev in wm["hwev_p"][0]:
            #         #print hwev
            #         if hwev != 0:
            #             msg = x.recv(dt.hwev_data_np.itemsize, socket.MSG_WAITALL)
            #             if self.count_ev:
            #                 ev = np.fromstring(msg, dtype=dt.hwev_data_np)
            #                 seat = wm["pov_seat"][0]
            #                 ev_d = ev["d"][0]
            #                 #idxlist = np.where(ev_d > -1000.0)

            #                 cards = td["vals"]["c"]
            #                 si = td["vals"]["sample_i"]
            #                 si_valid = np.where(ev_d[si] > -1000.0)
            #                 #if (len(si_valid[0]) != defs.SAMPLES):
            #                 #    print len(si_valid[0])
            #                     #pdb.set_trace()
            #                 #print len(si_valid[0])
            #                 hands = ct.ctoi2[cards[:,0], cards[:,1]]
            #                 self.avg_ev[seat,hands[si_valid]] += ev_d[si[si_valid]]
            #                 self.avg_ev_count[seat, hands[si_valid]] += 1

            #                 self.ev_tot[seat] += np.average(ev_d[si[si_valid]])
            #                 self.ev_tot_count[seat] += float(len(si_valid[0]))/defs.SAMPLES
            #                 #pdb.set_trace()
            #                 self.ev_tot2[seat] += ev["pw"][0]
            #                 self.ev_tot_count2[seat] += 1
            #                 if (abs(np.average(ev_d) - ev["pw"][0]) > 0.00001):
            #                     pdb.set_trace()
            #                     #print len(idxlist[0]),len(idxlist[1])
            #                 #avg_ev_count[seat] += 1
            #                     #avg_ev_count[seat] += len(idxlist[0])/1081.0
            #                 #print wm["pov_seat"], np.average(ev["d"])
            #     assert(wm["n_variations"] == 1)
            #     msg = x.recv(dt.variation_np.itemsize, socket.MSG_WAITALL)

            #     seat = wm["pov_seat"][0]
            #     #pdb.set_trace()
            #     self.tot_odds_count[seat] += 1
            #     if self.tot_odds_count[seat] > 10000:
            #         self.tot_odds_count[seat] = 10000
            #         self.tot_odds_change[seat] *= (1.0-1.0/10000.0)
            #     self.tot_odds_change[seat] +=  wm["positive_regs"][0]
            #     #print seat, wm["positive_regs"][0]
            #     self.items_processing[td_id]+=1
            #     if self.items_processing[td_id] == len(self.active_seats):
            #         del self.items_processing[td_id]
            #         del self.td_dic[td_id]
            #         if not td_id%1000:

            #             print "deleted", td_id, self.tot_odds_change, self.tot_odds_count, self.tot_odds_change/self.tot_odds_count, "td per second:", 1000.0/(time.time() - self.last_td_time)
            #             self.last_td_time = time.time()
            #             if self.count_ev:
            #                 for i in xrange(2):
            #                     print self.avg_ev[i]/self.avg_ev_count[i] - (1.0 - i*0.5)
            #                     #if avg_ev_count[i] != 0:
            #                     #    print i, avg_ev[i]/avg_ev_count[i]
            #                 for i in xrange(2):
            #                     print np.average(self.avg_ev[i]/self.avg_ev_count[i]) - (1.0 - i*0.5)
            #                     if self.ev_tot_count[i] != 0:
            #                         print self.ev_tot[i]/self.ev_tot_count[i] - (1.0 - i*0.5)
            #                     if self.ev_tot_count2[i] != 0:
            #                         print self.ev_tot2[i]/self.ev_tot_count2[i] - (1.0 - i*0.5)

            #         for cl in self.clients_by_socket.values():
            #             cl.clean_types_data(td_id)
            #     if self.adjust_state == 0 and self.tot_odds_count.sum() > 100 and self.tot_odds_change.sum()/self.tot_odds_count.sum() < self.adjust_limit:
            #         print "starting adjust", self.tot_odds_change,self.tot_odds_count, self.tot_odds_change/self.tot_odds_count, self.adjust_limit 
            #         self.adjust_state = 1

            # # if uao and not td_id_count%uao_every and td_id_count != uao_last:
            # #     uao_last = td_id_count
            # #     for cl in clients_by_socket.values():
            # #         cl.update_avg_odds()

        if self.adjust_state:
            if self.adjust_state == 1:
                if self.queue_limit != 0:
                    self.old_queue_limit = self.queue_limit
                self.queue_limit = 0
                if len(self.items_processing) == 0:
                    for us in self.p.iter_all_unique_states():
                        if us.handling_client != None and us.gamestate != 4:
                            us.handling_client.adjust_model(us.id, self.lo_limit, self.hi_limit, self.aod_limit, self.timestamp)
                            us.hand_counts_reduce = None
                        #for cl in clients_by_socket.values():
                        #cl.reduce_model(lo_limit, hi_limit, aod_limit, timestamp)
                    self.timestamp = 0
                    self.td_id_count = 0
                    self.queue_limit = 0
                    self.tot_odds_count = np.zeros(2, dtype=np.float64)
                    self.tot_odds_change = np.zeros(2, dtype=np.float64)

                    self.adjust_state = 2


            if self.adjust_state == 2:
                for us in self.p.iter_all_unique_states():
                    if us.handling_client != None and us.gamestate != 4 and us.hand_counts_reduce == None:
                        break
                else:
                    if self.no_expand:
                        self.adjust_state = 0
                    else:
                        self.adjust_state = 3

            if self.adjust_state == 3:
                tot_hc_start = 0
                tot_hc_stop = 0
                for us in self.p.iter_all_unique_states():
                    if us.handling_client != None and us.gamestate != 4:
                        tot_hc_start += us.hand_counts_reduce[0]
                        tot_hc_stop += us.hand_counts_reduce[1]
                self.lo_limit *= (float(tot_hc_stop)/float(tot_hc_start))/self.target_reduce_ratio

                ratio = float(self.target_hand_count)/float(tot_hc_stop)
                print "reduce start:", tot_hc_start, "stop:", tot_hc_stop, "new limit:", self.lo_limit, "ratio:", ratio
                for us in self.p.iter_all_unique_states():
                    if us.handling_client != None and us.gamestate != 4:
                        us.handling_client.expand_model(us.id, ratio)
                        us.hand_counts_expand = None
                self.adjust_state = 4
            if self.adjust_state == 4:
                tot_hc_start = 0
                tot_hc_stop = 0
                for us in self.p.iter_all_unique_states():
                    if us.handling_client != None and us.gamestate != 4:
                        if us.hand_counts_expand == None:
                            break
                        else:
                            tot_hc_start += us.hand_counts_expand[0]
                            tot_hc_stop += us.hand_counts_expand[1]
                else:
                    self.adjust_state = 0
                    self.queue_limit = self.old_queue_limit
                    print "expand start:", tot_hc_start, "stop:", tot_hc_stop, "target:", self.target_hand_count
        else:
            with self.dic_lock:
                qlen = len(self.items_processing)
            while self.queue_limit > qlen:
                td_id = self.calc_one_iteration()
                with self.dic_lock:
                    qlen = len(self.items_processing)
                self.timestamp+=1
                if self.td_id_count > 0 and self.adjust_every > 0 and not self.td_id_count%self.adjust_every:
                    self.adjust_state = 1


if __name__ == "__main__":
    c = ctrl(sys.argv[1])
    print "ctrl reated"
    
    while True:
        c.perform_one_loop(0.01)
            
