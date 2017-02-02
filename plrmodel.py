import ctypes as C
import sys
import zpoker
import numpy as np
import select
import random
import copy
import time
import pdb 
import os
import defs

import datatypes as dt
#import libplrmodel as lpm
import lzp
import card_table as ct



class unique_state:
    def __init__(self, zp, state, sb = 0.5, bb = 1.0, cap = 4.0):
        #self.dll_pmodel = dll
        self.zp = zp
        self.id = zp.us_to_int[state]
        self.model_path = ""
        #self.gamestate, self.bets, self.acted, self.not_acted, self.to_act = state
        self.gamestate = state[0]
        self.to_act = state[1]
        self.cur_seat = state[2]
        self.bets = state[3:]

        self.state = state
        self.cap = cap
        self.sb = sb
        self.bb = bb
        if self.gamestate == 0 and sum(self.bets) < 1.5:
            self.betsize = self.bb
            self.to_call = (self.bets[-1] - self.bets[0])*self.betsize
            self.to_raise = self.to_call + self.sb
        else:
            self.betsize = (self.gamestate/2+1)*self.bb
            self.to_call = (self.bets[-1] - self.bets[0])*self.betsize
            self.to_raise = self.to_call + self.betsize
        if self.bets[-1] == self.cap:
            self.to_raise = 1000

        # self.type_order = [0,13,23,12,22,11,21,33,32,31,100]
        # i = 0
        # self.next_type = {}
        # while i < len(self.type_order)-1:
        #     self.next_type[self.type_order[i]] = self.type_order[i+1]
        #     i+=1
        
        self.action_cost = [self.to_raise, self.to_call, 0]
        #print "action cost", self.state, self.action_cost
        self.n_plr = len(self.bets)
        self.next_states = []
        self.prev_states = {}
        #print self.gamestate, self.n_plr, self.bets, self.acted, self.not_acted, self.to_act
        self.unique_root_node = dt.unique_root()
        self.unique_root_node.root_idx = self.id
        self.unique_root_node.n_plr = self.n_plr
        self.unique_root_node.gamestate = self.gamestate
        self.unique_root_node.to_act = self.to_act
        self.unique_root_node.cur_seat = self.cur_seat
        self.unique_root_node.bets = C.cast(C.pointer((C.c_double*self.n_plr)()), C.POINTER(C.c_double))
        self.unique_root_node.action_cost[0] = self.to_raise
        self.unique_root_node.action_cost[1] = self.to_call
        self.unique_root_node.action_cost[2] = 0
        
        self.types_order = [[],[]]
        for p in xrange(2):
            for i in xrange(self.zp.i.n_type_types[p]):
                if (self.zp.i.type_types[p][i].gamestate < self.gamestate and not self.zp.i.type_types[p][i].local) or self.zp.i.type_types[p][i].gamestate == self.gamestate:
                    self.types_order[p].append(self.zp.i.type_types[p][i].id)
            self.unique_root_node.n_type_types[p] = len(self.types_order[p])
            self.types_order[p] = np.array(self.types_order[p], dtype=np.int32)
            self.unique_root_node.type_types_order[p] = self.types_order[p].ctypes.data_as(C.POINTER(C.c_int32))
            
        # private_types_order = []
        # for i in xrange(self.zp.i.n_private_types):
        #     if self.zp.i.private_types[i].gamestate == self.gamestate:
        #         private_types_order.append(self.zp.i.private_types[i].id)

        # self.unique_root_node.n_public_types = len(public_types_order)
        # self.unique_root_node.n_private_types = len(private_types_order)

        
        # self.public_types_order = np.array(public_types_order, dtype=np.int32)
        # self.unique_root_node.public_types_order = self.public_types_order.ctypes.data_as(C.POINTER(C.c_int32))

        # self.private_types_order = np.array(private_types_order, dtype=np.int32)
        # self.unique_root_node.private_types_order = self.private_types_order.ctypes.data_as(C.POINTER(C.c_int32))

        for i in xrange(self.n_plr):
            self.unique_root_node.bets[i] = self.bets[i]
            
       # if self.gamestate == 4:
       #     self.plrmodel_trees = [None]*self.n_plr
       # else:
       #     self.plrmodel_trees = [None]

    def iter_model_tree_rec(self, pmn, leaf_only):
        if pmn.contents.t.contents.public:
            if not leaf_only:
                yield pmn
            for i in xrange(pmn.contents.len):
                iter_model_tree_rec(C.cast(pmn.contents.next_list[i], C.POINTER(dt.plrmodel_node)), leaf_only)
        else:
            yield pmn

    def iter_model_tree(self, leaf_only = True):
        self.iter_model_tree_rec(self.unique_root_node.model_tree, leaf_only)

    def set_model_filename(self, path):
        if path[-1] != "/":
            path += "/"
        #self.model_filename = path + str(self.id) + "_" + str(self.state) + ".model_tree"
        self.model_filename = path + str(self.state) + ".model_tree"

    def set_model_path(self, path):
        if path[-1] != "/":
            path += "/"
        #self.model_filename = path + str(self.id) + "_" + str(self.state) + ".model_tree"
        self.model_path = path

        
    def save_model_tree(self, path):
        if path[-1] != "/":
            path += "/"
        #filename = path + str(self.id) + "_" + str(self.state) + ".model_tree"
        filename = path + str(self.state) + ".model_tree"
        f = file(self.model_filename, "wb")
        lzp.lib.save_plrmodel_tree(f.fileno(), self.unique_root_node.model_tree)
        f.close()


    def save_struct(self):
        if self.model_path[-1] != "/":
            self.model_path += "/"
        #filename = self.model_path + str(self.id) + "_" + str(self.state) + ".model_tree"
        filename = self.model_path + str(self.state) + ".struct"
        f = file(filename, "wb")
        lzp.save_plrmodel_struct(f.fileno(), self.unique_root_node.model_tree)
        f.close()

    def save_regs(self):
        if self.model_path[-1] != "/":
            self.model_path += "/"
        #filename = self.model_path + str(self.id) + "_" + str(self.state) + ".model_tree"
        filename = self.model_path + str(self.state) + ".regs"
        f = file(filename, "wb")
        lzp.lib.save_plrmodel_regs(f.fileno(), self.unique_root_node.model_tree)
        f.close()

    def save_avg_odds(self):
        if self.model_path[-1] != "/":
            self.model_path += "/"
        filename = self.model_path + str(self.state) + ".avg_odds"
        f = file(filename, "wb")
        lzp.lib.save_plrmodel_avg_odds(f.fileno(), self.unique_root_node.model_tree)
        f.close()

    def save_byte_odds(self, source = "AVG"):
        if self.model_path[-1] != "/":
            self.model_path += "/"
        if source == "AVG":
            filename = self.model_path + str(self.state) + ".byte_odds_avg"
            f = file(filename, "wb")
            lzp.lib.save_plrmodel_byte_odds_from_avg(f.fileno(), self.unique_root_node.model_tree)
            f.close()
        elif source == "REGS":
            filename = self.model_path + str(self.state) + ".byte_odds_regs"
            f = file(filename, "wb")
            lzp.lib.save_plrmodel_byte_odds_from_regs(f.fileno(), self.unique_root_node.model_tree)
            f.close()


    def load_struct(self):
        self.free_model_tree()
        if self.model_path[-1] != "/":
            self.model_path += "/"
        #filename = self.model_path + str(self.id) + "_" + str(self.state) + ".model_tree"
        filename = self.model_path + str(self.state) + ".struct"
        #print "loading", filename
        f = file(filename, "rb")
        self.unique_root_node.model_tree = lzp.load_plrmodel_struct(C.pointer(self.zp.i),f.fileno(), C.pointer(self.unique_root_node))
        f.close()

    def load_regs(self):
        if self.model_path[-1] != "/":
            self.model_path += "/"
        filename = self.model_path + str(self.state) + ".regs"
        #print "loading", filename
        f = file(filename, "rb")
        lzp.load_plrmodel_regs(self.unique_root_node.model_tree, f.fileno())
        f.close()
    
    def load_avg_odds(self):
        if self.model_path[-1] != "/":
            self.model_path += "/"
        filename = self.model_path + str(self.state) + ".avg_odds"
        #print "loading", filename
        f = file(filename, "rb")
        lzp.load_plrmodel_avg_odds(self.unique_root_node.model_tree, f.fileno())
        f.close()

    def load_byte_odds(self, source="AVG"):
        if self.model_path[-1] != "/":
            self.model_path += "/"
        if source == "AVG":
            filename = self.model_path + str(self.state) + ".byte_odds_avg"
        elif source == "REGS":
            filename = self.model_path + str(self.state) + ".byte_odds_regs"
        #print "loading", filename
        f = file(filename, "rb")
        lzp.load_plrmodel_byte_odds(self.unique_root_node.model_tree, f.fileno())
        f.close()


    def mmap_regs(self):
        if self.model_path[-1] != "/":
            self.model_path += "/"
        filename = self.model_path + str(self.state) + ".regs"
        #print "loading", filename
        try:
            self.regs_mmap = np.memmap(filename, np.float64, mode="r+")
        except IOError:
            hands = self.get_hand_count()
            self.regs_mmap = np.memmap(filename, np.float64, mode="w+", shape = (hands*3,))
            self.regs_mmap.fill(0)

    def mmap_avg_odds(self):
        if self.model_path[-1] != "/":
            self.model_path += "/"
        filename = self.model_path + str(self.state) + ".avg_odds"
        #print "loading", filename
        try:
            self.avg_odds_mmap = np.memmap(filename, np.float64, mode="r+")
        except IOError:
            hands = self.get_hand_count()
            self.avg_odds_mmap = np.memmap(filename, np.float64, mode="w+", shape = (hands*3,))
            self.avg_odds_mmap.fill(0)

    def mmap_byte_odds(self, source):
        if self.model_path[-1] != "/":
            self.model_path += "/"
        if source == "AVG":
            filename = self.model_path + str(self.state) + ".byte_odds_avg"
        elif source == "REGS":
            filename = self.model_path + str(self.state) + ".byte_odds_regs"
        #print "loading", filename
        try:
            self.byte_odds_mmap = np.memmap(filename, np.uint8, mode="r+")
        except IOError:
            hands = self.get_hand_count()
            self.byte_odds_mmap = np.memmap(filename, np.uint8, mode="w+", shape = (hands*2,))
            self.byte_odds_mmap.fill(0)
            
    def set_regs_from_mmap(self):
        lzp.set_regs_from_mmap(self.unique_root_node.model_tree, self.regs_mmap.ctypes.data_as(C.POINTER(C.c_double)))
        
    def set_avg_odds_from_mmap(self):
        lzp.set_avg_odds_from_mmap(self.unique_root_node.model_tree, self.avg_odds_mmap.ctypes.data_as(C.POINTER(C.c_double)))

    def set_byte_odds_from_mmap(self):
        lzp.set_byte_odds_from_mmap(self.unique_root_node.model_tree, self.byte_odds_mmap.ctypes.data_as(C.POINTER(C.c_uint8)))

    def load_model_tree(self, path):
        self.free_model_tree()
        if path[-1] != "/":
            path += "/"
        #filename = path + str(self.id) + "_" + str(self.state) + ".model_tree"
        #filename = path + str(self.state) + ".model_tree"
        print "loading", filename
        f = file(filename, "rb")
        self.unique_root_node.model_tree = lzp.load_plrmodel_tree(C.pointer(self.zp.i),f.fileno(), C.pointer(self.unique_root_node))
        f.close()

    def save_model_tree_fd(self, fd):
        lzp.lib.save_plrmodel_tree(fd, self.unique_root_node.model_tree)
        
    def load_model_tree_fd(self, fd):
        self.free_model_tree()
        self.unique_root_node.model_tree = lzp.lib.load_plrmodel_tree(C.pointer(self.zp.i), fd, C.pointer(self.unique_root_node))

    def free_model_tree(self):
        if self.unique_root_node.model_tree:
            lzp.lib.free_plrmodel_tree(self.unique_root_node.model_tree)

    def get_action_odds_from_file(self, path, pub_types, priv_types, src):
        odds = np.zeros(3, dtype=np.float32)
        f = file(self.model_filename, "rb")
        #print "getting action odds", odds
        ao = lzp.get_action_odds_from_file(f.fileno(), C.pointer(self.zp.i), pub_types.ctypes.data_as(C.POINTER(C.c_int16)), priv_types.ctypes.data_as(C.POINTER(C.c_int16)), odds.ctypes.data_as(C.POINTER(C.c_float)), src)
        f.close()
        #print "got", odds
        return odds

    def get_hand_count(self):
        return lzp.count_hands(self.unique_root_node.model_tree)

    def count_zero_avg_hands(self):
        return lzp.count_zero_avg_hands(self.unique_root_node.model_tree)

    def count_zero_regs_hands(self):
        return lzp.count_zero_regs_hands(self.unique_root_node.model_tree)

    def prune_zero_avg_hands(self):
        return lzp.prune_zero_avg_hands(self.unique_root_node.model_tree)

    def prune_zero_regs_hands(self):
        return lzp.prune_zero_regs_hands(self.unique_root_node.model_tree)

    def recount_data_index(self):
        return lzp.recount_data_index(self.unique_root_node.model_tree, 0)
    
    def gen_plrmodel_tree(self):
        i = 0
        print "gamestate", self.gamestate
        if self.gamestate < 4:
            self.unique_root_node.model_tree = lzp.gen_minimal_plrmodel_tree(C.pointer(self.zp.i),C.pointer(self.unique_root_node), 0) 
            self.unique_root_node.model_tree.slot_in_prev = self.id
            #lzp.reset_visits_and_count_hands(self.unique_root_node.model_tree)
            #lzp.recursive_reset_plrmodel_data(self.unique_root_node.model_tree, 0xffffffffffffffff)
            lzp.recount_pub_nodes(self.unique_root_node.model_tree)

        
    def recursive_generate_all_unique_states(self, states_dict):
        states = self.zp.get_next_unique_states(self.state)
        #print self.state, "next_state", states
        self.next_states = []
        #print "states len", len(states)
        for i,s in enumerate(states):
            #print i,s
            if s != None:
                if s not in states_dict:
                    #print i,s
                    new_state = unique_state(self.zp, s)
                    states_dict[s] = new_state
                    #self.unique_root_node.next[i] = C.pointer(new_state.unique_root_node)
                    new_state.recursive_generate_all_unique_states(states_dict)
                self.unique_root_node.next[i] = C.pointer(states_dict[s].unique_root_node)
                self.next_states.append(states_dict[s])
                states_dict[s].prev_states[self.state] = self
            else:
                self.next_states.append(None)
                self.unique_root_node.next[i] = None
            #print "states after loop", states
        #self.gen_plrmodel_tree()


    def expand_type_max(self, public, id):
        lzp.expand_type_max(self.unique_root_node.model_tree, public, id)



    def reset_model(self, flags):
        lzp.recursive_reset_plrmodel_data(self.unique_root_node.model_tree, flags)

        
    def print_plrmodel_tree(self):
        lzp.print_plrmodel_tree(self.unique_root_node.model_tree, 0)
    
    def divide_all_same_type(self, type, gamestate):
        lzp.lib.divide_all_same_type(self.unique_root_node.model_tree, type, gamestate)


    def get_odds(self, pub_types, priv_types, odds, rng):
        lzp.get_action_odds(C.pointer(self.unique_root_node), pub_types.ctypes.data_as(C.POINTER(C.c_int16)), priv_types[self.gamestate], odds.ctypes.data_as(C.POINTER(C.c_double)), rng)
        
class plrmodel:
    def __init__(self, gamedir = "testipeli", model_filename = ""):

        self.zp = zpoker.zpoker(gamedir)
        self.rng = lzp.get_rng()
        self.n_plr = self.zp.i.n_plr
        self.sb = self.zp.i.sb
        self.bb = self.zp.i.bb
        self.cap = self.zp.i.maxbets

        
        if self.n_plr == 2:
            next_to_act = 1
        else:
            next_to_act = 2
            
        
        if self.n_plr == 2:
            initial_state = (0, 2,1,0.0,0.0)
        else:
            initial_state = (0,self.n_plr, self.n_plr-2)+tuple([0.0]*self.n_plr)
        
        #if self.n_plr == 2:
        #    initial_state = (0, 2,1,0.5,1)
        #else:
        #    initial_state = (0,self.n_plr, self.n_plr-2)+tuple([0]*(self.n_plr-2))+(0.5,1)
        #self.root_state_id = (0, 1, 0, self.n_plr, next_to_act)        
        
        
        self.root_state_id = initial_state
        self.root_state = unique_state(self.zp, self.root_state_id, self.sb, self.bb, self.cap)
        self.states_dict = {}
        self.states_dict[self.root_state_id] = self.root_state
        self.root_state.recursive_generate_all_unique_states(self.states_dict)
        
        self.do_ev = False
        self.no_update = 1000
        self.odds_from = 0
        self.sd = dt.simu_data()
        self.sd.info = C.pointer(self.zp.i)
        for i in xrange(self.n_plr):
            self.sd.plr[i] = (1.0,1.0,0.0,0.0,0,1,1,0)

        self.model_path = ""
        self.model_name = ""
        self.model_time = ""

    def __del__(self):
        self.free_model()

    def reset_model_path(self, model_path):
        self.model_path = model_path
        for us in self.iter_all_unique_states():
            us.set_model_path(self.model_path)
    
    def reset_model_name(self, model_name = None, reset_time = False):
        if model_name != None:
            self.model_name = model_name
        if reset_time:
            self.model_time = time.strftime("_%Y_%m_%d_%H_%M_%S")
        self.model_path = self.zp.gamedir + "plrmodels/plrmodel_" + self.model_name+ time.strftime("_%Y_%m_%d_%H_%M_%S") + "/"    
        #self.model_path = self.zp.gamedir + "plrmodel_" +self.model_name+ self.model_time + "/"
        for us in self.iter_all_unique_states():
            us.set_model_path(self.model_path)
            
    def save_model(self, prefix = "", overwrite = True, seat = -1, gamestates = (0,1,2,3)):        
        if self.model_path == "" or not overwrite:
            self.reset_mode_name(reset_time=True)
            #self.model_path = self.zp.gamedir + "plrmodels/plrmodel_" + self.model_name+ time.strftime("_%Y_%m_%d_%H_%M_%S") + "/"
            #self.model_path = self.zp.gamedir + "plrmodel_" +self.model_name+ time.strftime("_%Y_%m_%d_%H_%M_%S") + "/"
        try:
            os.mkdir(self.model_path)
        except OSError:
            pass

        for us in self.iter_all_unique_states():
            if us.id >= self.n_plr and us.gamestate in gamestates:
                if seat == -1 or seat == us.utg:
                    us.save_model_tree(self.model_path)

    def save_struct(self, seat = -1, gamestates = (0,1,2,3)):        
        if self.model_path == "":
            return False
        try:
            os.mkdir(self.model_path)
        except OSError:
            pass

        for us in self.iter_all_unique_states():
            if us.id >= self.n_plr and us.gamestate in gamestates:
                if seat == -1 or seat == us.utg:
                    us.recount_data_index()
                    us.save_struct()

    def save_regs(self, seat = -1, gamestates = (0,1,2,3)):        
        if self.model_path == "":
            return False
        try:
            os.mkdir(self.model_path)
        except OSError:
            pass

        for us in self.iter_all_unique_states():
            if us.id >= self.n_plr and us.gamestate in gamestates:
                if seat == -1 or seat == us.utg:
                    us.save_regs()

    def save_avg_odds(self, seat = -1, gamestates = (0,1,2,3)):        
        if self.model_path == "":
            return False
        try:
            os.mkdir(self.model_path)
        except OSError:
            pass

        for us in self.iter_all_unique_states():
            if us.id >= self.n_plr and us.gamestate in gamestates:
                if seat == -1 or seat == us.utg:
                    us.save_avg_odds()

    def save_byte_odds(self, seat = -1, gamestates = (0,1,2,3), source = "AVG"):        
        if self.model_path == "":
            return False
        try:
            os.mkdir(self.model_path)
        except OSError:
            pass

        for us in self.iter_all_unique_states():
            if us.id >= self.n_plr and us.gamestate in gamestates:
                if seat == -1 or seat == us.utg:
                    us.save_byte_odds(source)


    def load_struct(self, seat = -1, gamestates = (0,1,2,3)):
        for us in self.iter_all_unique_states():
            if us.id >= self.n_plr and us.gamestate in gamestates:
                if seat == -1 or seat == us.cur_seat:
                    us.load_struct()

    def load_regs(self, seat = -1, gamestates = (0,1,2,3)):
        for us in self.iter_all_unique_states():
            if us.id >= self.n_plr and us.gamestate in gamestates:
                if seat == -1 or seat == us.cur_seat:
                    us.load_regs()

    def load_avg_odds(self, seat = -1, gamestates = (0,1,2,3)):
        for us in self.iter_all_unique_states():
            if us.id >= self.n_plr and us.gamestate in gamestates:
                if seat == -1 or seat == us.cur_seat:
                    us.load_avg_odds()

    def load_byte_odds(self, seat = -1, gamestates = (0,1,2,3), source="AVG"):
        for us in self.iter_all_unique_states():
            if us.id >= self.n_plr and us.gamestate in gamestates:
                if seat == -1 or seat == us.cur_seat:
                    us.load_byte_odds(source)
    
    def mmap_regs(self, seat = -1, gamestates = (0,1,2,3)):
        for us in self.iter_all_unique_states():
            if us.id >= self.n_plr and us.gamestate in gamestates:
                if seat == -1 or seat == us.cur_seat:
                    us.mmap_regs()
    
    def mmap_avg_odds(self, seat = -1, gamestates = (0,1,2,3)):
        for us in self.iter_all_unique_states():
            if us.id >= self.n_plr and us.gamestate in gamestates:
                if seat == -1 or seat == us.cur_seat:
                    us.mmap_avg_odds()

    def mmap_byte_odds(self, seat = -1, gamestates = (0,1,2,3), source="AVG"):
        for us in self.iter_all_unique_states():
            if us.id >= self.n_plr and us.gamestate in gamestates:
                if seat == -1 or seat == us.cur_seat:
                    us.mmap_byte_odds(source)

    def set_regs_from_mmap(self, seat = -1, gamestates = (0,1,2,3)):
        for us in self.iter_all_unique_states():
            if us.id >= self.n_plr and us.gamestate in gamestates:
                if seat == -1 or seat == us.cur_seat:
                    us.set_regs_from_mmap()
    
    def set_avg_odds_from_mmap(self, seat = -1, gamestates = (0,1,2,3)):
        for us in self.iter_all_unique_states():
            if us.id >= self.n_plr and us.gamestate in gamestates:
                if seat == -1 or seat == us.cur_seat:
                    us.set_avg_odds_from_mmap()

    def set_byte_odds_from_mmap(self, seat = -1, gamestates = (0,1,2,3)):
        for us in self.iter_all_unique_states():
            if us.id >= self.n_plr and us.gamestate in gamestates:
                if seat == -1 or seat == us.cur_seat:
                    us.set_byte_odds_from_mmap()



    def load_model(self, path, seat = -1, gamestates = (0,1,2,3)):
        self.model_path = path

        for us in self.iter_all_unique_states():
            if us.id >= self.n_plr and us.gamestate in gamestates:
                if seat == -1 or seat == us.cur_seat:
                    us.load_model_tree(self.model_path)

    def reset_model(self, flags, seat = -1, gamestates = (0,1,2,3)):
        for us in self.iter_all_unique_states():
            if us.id >= self.n_plr and us.gamestate in gamestates:
                if seat == -1 or seat == us.cur_seat:
                    us.reset_model(flags)
        
    def reset_regs(self, seat = -1):
        self.reset_model(defs.SITU_RESET_REGS, seat)
        
    def reset_avg_odds(self, seat = -1):
        self.reset_model(defs.SITU_RESET_AVG_ODDS, seat)


    def recount_data_index(self, seat=-1):
        for us in self.iter_unique_states(seat):
            us.recount_data_index()

    def count_zero_avg_hands(self, seat=-1):
        tot = 0
        for us in self.iter_unique_states(seat):
            tot += us.count_zero_avg_hands()
        return tot

    def count_zero_regs_hands(self, seat=-1):
        tot = 0
        for us in self.iter_unique_states(seat):
            tot += us.count_zero_regs_hands()
        return tot
    
    def prune_zero_avg_hands(self, seat=-1):
        tot = 0
        for us in self.iter_unique_states(seat):
            tot += us.prune_zero_avg_hands()
        return tot

    def prune_zero_regs_hands(self, seat=-1):
        tot = 0
        for us in self.iter_unique_states(seat):
            tot += us.prune_zero_regs_hands()
        return tot

    def free_model(self, seat = -1, gamestates = (0,1,2,3)):
        for us in self.iter_unique_states(seat, gamestates):
            us.free_model_tree()
    
    def us_by_id(self, us_id):
        return self.states_dict[self.zp.int_to_us[us_id]]

    
    def gen_plrmodel_tree_for_all_us(self):
        for us in self.iter_all_unique_states():
            us.gen_plrmodel_tree()
            
    def iter_all_situs(self):
        for us in self.states_dict.values():
            situs = us.get_all_situs()
            for pm in situs:
                for s in pm[1]:
                    yield(s)



                    #def thread_with_retval(self, target,args, Q):
                    # def exec_for_all_unique_states(self, target, args, seat = -1, gamestates = (0,1,2,3), prenodes = False):
        

    def iter_unique_states(self, seat = -1, gamestates = (0,1,2,3), prenodes = False):
        for us in self.states_dict.values():
            if seat == -1 or seat == us.cur_seat:
                if us.gamestate in gamestates:
                    if us.id > self.n_plr or prenodes == True:
                        yield us

    def iter_all_unique_states(self ):
        for us in self.states_dict.values():
            yield us
            
    def set_hand_hw_start(self, us):
        assert len(us.next_states) == 3
        
        if us.gamestate == 0 and sum(us.bets) < 1.5:
            lzp.lib.set_hand_hw_start(C.pointer(self.zp.i), C.pointer(self.ga), us.unique_root_node.model_tree, float(sum(us.bets)*us.betsize))
            #print "setting hand hw start", self.zp.us_to_int[us.state], us.gamestate, us.bets, us.utg, us.sb, us.bb, float(sum(us.bets))
        else:
            return
        for state in us.next_states:
            if state != None:
                self.set_hand_hw_start(state)

    def get_pub_types(self, path, board):
        pub_types = np.zeros(self.zp.i.n_type_types[1], dtype=np.int16)-1
        if len(board) >= 3:
            pub_types[9] = lzp.get_board_type(C.pointer(self.zp.i), 1, board.ctypes.data_as(C.POINTER(C.c_int16)))
        if len(board) >= 4:
            pub_types[8] = lzp.get_board_type(C.pointer(self.zp.i), 2, board.ctypes.data_as(C.POINTER(C.c_int16)))
        if len(board) >= 5:
            pub_types[7] = lzp.get_board_type(C.pointer(self.zp.i), 3, board.ctypes.data_as(C.POINTER(C.c_int16)))
        lzp.set_mutable_types_from_path(pub_types.ctypes.data_as(C.POINTER(C.c_int16)), "00"+path, len(path)+2, C.pointer(self.root_state.unique_root_node))
        return pub_types

    def get_priv_types(self, board):
        priv_types = [None]*4
        priv_types[0] = lzp.get_hand_types(C.pointer(self.zp.i), 0, board.ctypes.data_as(C.POINTER(C.c_int16)))
        if len(board) >= 3:
            priv_types[1] = lzp.get_hand_types(C.pointer(self.zp.i), 1, board.ctypes.data_as(C.POINTER(C.c_int16)))
        if len(board) >= 4:
            priv_types[2] = lzp.get_hand_types(C.pointer(self.zp.i), 2, board.ctypes.data_as(C.POINTER(C.c_int16)))
        if len(board) >= 5:
            priv_types[3] = lzp.get_hand_types(C.pointer(self.zp.i), 3, board.ctypes.data_as(C.POINTER(C.c_int16)))
        
        return priv_types
    
    def get_hw_for_path(self, path, board, seat, dead_cards=None):
        hw = np.ones((defs.HANDS), dtype=np.float64)
        odds = np.zeros((3, defs.HANDS), dtype=np.float64)
        pub_types = self.get_pub_types(path,board)
        priv_types = self.get_priv_types(board)
            
        if dead_cards != None:
            not_valid_cards_tmp = np.in1d(ct.itoc2[np.arange(defs.HANDS)], dead_cards)
            not_valid_cards_tmp.shape = (defs.HANDS, 2)
            not_valid_cards = np.where(not_valid_cards_tmp == True)[0]
            hw[not_valid_cards] = 0
        hw *= 1.0/hw.sum()
        
        cur_us = self.root_state.next_states[0].next_states[0]
        for act in path:
            act_i = int(act)
            if cur_us.cur_seat == seat:
                cur_us.get_odds(pub_types, priv_types, odds, self.rng)                
                hw *= odds[act_i]
            cur_us = cur_us.next_states[act_i]
        return hw

    def get_us_from_path(self, path):
        cur_us = self.root_state.next_states[0].next_states[0]
        potsize = 1.5
        stake = [1.0, 0.5]
        for act in path:
            act_i = int(act)
            potsize += cur_us.action_cost[act_i]
            stake[cur_us.cur_seat] += cur_us.action_cost[act_i]
            cur_us = cur_us.next_states[act_i]
            
        return cur_us, potsize,tuple(stake)

    def solve_ev(self, us, path, board, seat, hand, hw, handval_dict, handvals, potsize, stake, solutions_dic):
        if us.gamestate == 4:
            if us.to_act == 1: #FOLD
                if us.cur_seat == seat:
                    return (hw.sum()*(potsize-stake),)
                else:
                    return (hw.sum()*(-stake),)
            else: #SHOWDOWN
                return (lzp.get_showdown_ev_one_hand_gen(hw.ctypes.data_as(C.POINTER(C.c_double)), handvals.ctypes.data_as(C.POINTER(dt.hand_hv2)), potsize, stake, ct.ctoi2[hand[0], hand[1]], defs.HANDS),)
                
        if handvals == None and us.gamestate >= 3:
            handvals = handval_dict[tuple(board)]
        ev = 0.0

        if seat != us.cur_seat:
            odds = np.zeros((3, defs.HANDS), dtype=np.float64)
            pub_types = self.get_pub_types(path,board)
            priv_types = self.get_priv_types(board)
            us.get_odds(pub_types, priv_types, odds, self.rng)
            new_hw = np.zeros((3, defs.HANDS), dtype=np.float64)
            avg_odds = np.zeros(3, dtype=np.float64)
            for act_i in xrange(3):
                new_hw[act_i] = hw*odds[act_i]
                avg_odds[act_i] = new_hw[act_i].sum()
            avg_odds /= avg_odds.sum()
            
            for act_i in xrange(3):
                if us.next_states[act_i] != None:
                    ev += self.solve_ev(us.next_states[act_i], path+str(act_i), board, seat, hand, hw*odds[act_i], handval_dict, handvals, potsize+us.action_cost[act_i], stake, solutions_dic)[0]#*avg_odds[act_i]
            return (ev,)
                
                    
        else:
            ev = np.zeros(3, dtype=np.float64)
            for act_i in xrange(3):
                if us.next_states[act_i] != None:
                    ev[act_i] = self.solve_ev(us.next_states[act_i], path+str(act_i), board, seat, hand, hw, handval_dict, handvals, potsize+us.action_cost[act_i], stake+us.action_cost[act_i], solutions_dic)[0]
                else:
                    ev[act_i] = -100000.0
            solutions_dic[(path, tuple(board), tuple(hand))] = (ev[ev.argmax()], ev.argmax(), tuple(ev))
            return [ev[ev.argmax()], ev.argmax(), ev]
            
    def get_ev_for_hand(self, path, board, hand, solutions_dic):
        us, potsize, stake = self.get_us_from_path(path)
        seat = us.cur_seat
        hw = self.get_hw_for_path(path, board, (seat+1)%2, np.hstack((hand, board)))
        handval_dict = {}
        hw/=hw.sum()
        assert(us.gamestate == 3)
        handval = np.zeros((defs.HANDS), dtype = dt.hand_hv2_np)
        lzp.get_hand_hv2(board.ctypes.data_as(C.POINTER(C.c_int8)), handval.ctypes.data_as(C.POINTER(dt.hand_hv2)))
        handval_dict[tuple(board)] = handval
        return self.solve_ev(us, path, board, seat, hand, hw, handval_dict, None, potsize, stake[us.cur_seat], solutions_dic)
