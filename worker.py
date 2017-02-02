import datatypes as dt
import lzp
import ctypes as C
import threading as T
import plrmodel
import sys
import time
import pdb
import socket
import select
import struct
import defs
import os


class worker:
    def __init__(self, server_addr, pm, n_workers = 1):
        
        self.pm = pm
        self.server_addr = server_addr
    
        self.sender_args = dt.sender_thread_args()
        self.worker_args = dt.worker_thread_args()
        self.receiver_args = dt.receiver_thread_args()
        self.control_args = dt.thread_control_args()
        self.control_args.s_args = C.pointer(self.sender_args)
        self.control_args.w_args = C.pointer(self.worker_args)
        self.control_args.r_args = C.pointer(self.receiver_args)
        self.control_args.root_us = C.pointer(self.pm.root_state.unique_root_node)
        self.control_args.n_worker_thread = n_workers
        
        lzp.setup_trees_and_queues(C.pointer(self.control_args))
        lzp.setup_thread_args(C.pointer(self.control_args))
        lzp.start_threads(C.pointer(self.control_args))
        
        self.control_thread = T.Thread(target=self.run_control_thread)

        self.control_thread.start()
        
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

    def run_control_thread(self):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.bind(("",0))
        self.s.connect(self.server_addr)
        port = self.receiver_args.listen_port
        msg = struct.pack("IH", 0xffffffff, port)
        self.s.send(msg)
        
        read_list = [self.s]
        
        while True:
            sel_ret = select.select(read_list, [], [], 0.3)
            for sock in sel_ret[0]:
                dat = sock.recv(4, socket.MSG_WAITALL)
                if len(dat) != 4:
                    self.s.close()
                    print "Disconnected, exiting"
                    sys.exit(0)
                in_msg = struct.unpack("I", dat)[0]
                print "mew message:", in_msg
                if in_msg == 1: #new types data
                    print "new types data"
                    lzp.recv_new_types_data(C.pointer(self.control_args), self.s.fileno())

                if in_msg == 2:
                    dat = sock.recv(22, socket.MSG_WAITALL)
                    us_id, addr, port = struct.unpack("i16sH", dat)
                    if addr[:9] == "127.0.0.1":
                        addr = self.server_addr[0]
                    
                    print "setting handler", us_id, addr, port
                    self.pm.set_handler(us_id, addr, port)
                    #us = self.pm.states_dict[us_id]
                    
                elif in_msg == 3:
                    frm = "=IQ"
                    msg_len = struct.calcsize(frm)
                    dat = sock.recv(msg_len, socket.MSG_WAITALL)
                    us_id, flags = struct.unpack(frm, dat)
                    #self.send_ctrl_messge(3,msg)
                    p.us_by_id(us_id).reset_model(flags)
                    
                elif (in_msg == 10): # LOCK QUEUES
                    if (self.queues_locked == False):
                        out_msg = struct.pack("=II", in_msg, 1)
                        self.queues_locked = True
                    else:
                        out_msg = struct.pack("II", in_msg, 0) # already locked
                    self.s.send(out_msg)

                elif (in_msg == 11): # UNLOCK QUEUES
                    if (self.queues_locked == True):
                        lzp.unlock_queues()
                        out_msg = struct.pack("=II", in_msg, 1)
                        self.queues_locked = False
                    else:
                        out_msg = struct.pack("=II", in_msg, 0) #already unlocked
                    self.s.send(out_msg)

                elif (in_msg == 13): # GET AVG FITNESS
                    dat = sock.recv(8, socket.MSG_WAITALL)
                    timestamp = struct.unpack("Q", dat)[0]
                    print "timestamp", timestamp
                    ret = p.get_avg_fitness(timestamp)
                    for item in ret:
                        print "sending fitness", item
                        out_msg = struct.pack("IId", in_msg, item[0], item[1])
                        self.s.send(out_msg)

                elif (in_msg == 14): # GET AVG FITNESS
                    print "recv update_avg_odds"
                    ret = float(p.update_avg_odds())
                    print "retval", ret
                    out_msg = struct.pack("=Id", in_msg, ret)
                    self.s.send(out_msg)

                        
                elif in_msg == 20: #reduce models
                    dat = sock.recv(36, socket.MSG_WAITALL)
                    us_id, lo_limit, hi_limit, aod_limit, timestamp = struct.unpack("=idddQ", dat)
                    print "Reducing", us_id, p.zp.int_to_us[us_id]
                    print "Locked work queue, items:", self.lock_work_queue()
                    #time.sleep(0.1)
                    start_hc, end_hc = p.us_by_id(us_id).reduce_model_tree(lo_limit, hi_limit, aod_limit, timestamp)
                    #p.adjust_models(lo_limit, hi_limit, aod_limit, timestamp)
                    self.unlock_work_queue()
                    out_msg = struct.pack("=IIII", in_msg, us_id, start_hc, end_hc)
                    self.s.send(out_msg)
                        
                    
                elif in_msg == 21: #expand models
                    dat = sock.recv(12, socket.MSG_WAITALL)
                    us_id, multiplier = struct.unpack("=Id", dat)
                    print "Expanding", us_id, p.zp.int_to_us[us_id]
                    print "Locked work queue, items:", self.lock_work_queue()
                    #time.sleep(2)
                    start_hc, end_hc = p.us_by_id(us_id).expand_model_tree(multiplier)
                    out_msg = struct.pack("IIII", in_msg, us_id, start_hc, end_hc)
                    self.s.send(out_msg)
                    
                    #p.expand_models(multiplier)
                    self.unlock_work_queue()

                elif in_msg == 100: #send model
                    dat = sock.recv(4, socket.MSG_WAITALL)
                    us_id = struct.unpack("=I", dat)[0]
                    print "saving model", us_id
       
                    while lzp.zp_queue_get_item_count(self.control_args.receiver_to_worker_queue) > 0:
                        time.sleep(0.1)
                    self.lock_work_queue()
                    print "open file tmpmodel"
                    f = file("tmp_model", "wb")
                    print "start saving model"
                    p.us_by_id(us_id).save_model_tree_fd(f.fileno())
                    print "close file"
                    f.close()
                    print "unlock work queue"
                    self.unlock_work_queue()
                    print "get file size"
                    filesize = os.path.getsize("tmp_model")
                    print "form out msg"
                    out_msg = struct.pack("=III", in_msg, us_id, filesize)
                    print "send out msg"
                    self.s.send(out_msg)
                    print "open file for sending"
                    f = file("tmp_model", "rb")
                    print "start sending"
                    out_msg = f.read(1024)
                    while len(out_msg) > 0:
                        self.s.send(out_msg)
                        out_msg = f.read(1024)
                    print "delete tmpmodel file"
                    os.remove("tmp_model")
                    print "DONE"

                elif in_msg == 101: #recv model
                    dat = sock.recv(4, socket.MSG_WAITALL)
                    us_id = struct.unpack("=I", dat)[0]
                    p.us_by_id(us_id).load_model_tree_fd(sock.fileno())
                    
            item_count = lzp.zp_queue_get_item_count(self.control_args.receiver_to_worker_queue)
            print item_count
            out_msg = struct.pack("II", 12, item_count)
            self.s.send(out_msg)

            lzp.clean_old_types_data(C.pointer(self.control_args), 120.0)
        
    # def init_queues(self):
    #     self.receiver_to_worker_queue = lzp.queue_init()
    #     self.worker_to_sender_queue = lzp.queue_init()

    # def init_trees(self):
    #     self.us_tree = lzp.init_us_tree(C.pointer(self.root_us.unique_root_node))
    #     self.td_tree = lzp.init_td_tree()
    #     self.wm_tree = lzp.init_wm_tree()

    # def set_worker_args(self):
    #     self.worker_args.in_queue = self.receiver_to_worker_queue
    #     self.worker_args.out_queue = self.worker_to_sender_queue
    #     self.worker_args.us = C.pointer(self.root_us.unique_root_node)
    #     self.worker_args.us_tree = self.us_tree
    #     self.worker_args.wm_tree = self.wm_tree
    #     self.worker_args.cont = 1
        
    # def set_receiver_args(self):
    #     self.receiver_args.out_queue = self.receiver_to_worker_queue
    #     self.receiver_args.td_tree = self.td_tree
    #     self.receiver_args.bind_port = 11113
    #     self.receiver_args.cont = 1

    # def set_sender_args(self):
    #     self.sender_args.loopback_queue = self.receiver_to_worker_queue
    #     self.sender_args.in_queue = self.worker_to_sender_queue
    #     self.sender_args.us_tree = self.us_tree
    #     self.sender_args.print_fields()
    #     self.sender_args.cont = 1

    # def run_worker_thread(self):
    #     lzp.worker_thread(C.pointer(self.worker_args))

    # def run_receiver_thread(self):
    #     lzp.receiver_thread(C.pointer(self.receiver_args))

    # def run_sender_thread(self):
    #     lzp.sender_thread(C.pointer(self.sender_args))



if __name__ == "__main__":
    gamedir = sys.argv[1]
    #model_name = sys.argv[2]
    server_addr = ""
    if len(sys.argv) > 3:
        server_addr = sys.argv[3]
    p = plrmodel.plrmodel(gamedir)
    #p.model_name = model_name
    p.gen_plrmodel_tree_for_all_us()
    p.zp.load_diffs_for_all_types()
    #p.expand_models_test()
    #p.expand_models_until_n_hands_random(10000, 0)
#    pdb.set_trace()
    w = worker((server_addr, 11111), p, int(sys.argv[2]))
    w.control_thread.join()
    
