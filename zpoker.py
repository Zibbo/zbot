from ctypes import *
import numpy as np
import os
import card_table as ct
import copy
import sys
import random
import time
import math 
import ConfigParser
#import scipy.weave
import cPickle
import Queue
import threading
import mmap
import zlib
import pdb
import defs

import lzp
import datatypes as dt



def plrmodel_iter(root):
    yield root.contents
    if bool(root.contents.r):
        for x in plrmodel_iter(root.contents.r):
            yield x
    if bool(root.contents.c):
        for x in plrmodel_iter(root.contents.c):
            yield x

    
class zpoker:
    def __init__(self, gamedir = ""):
        #self.zlib = cdll.LoadLibrary("libzpoker.dylib")
        #self.zlib.precalc_conversions()
        
        
        self.SAMPLES = 1081
        self.DATA_SAMPLES = 1081
        self.HANDS = 1326
        self.FLOPS = 22100
        self.TURNS = 270725
        self.RIVERS = 2598960
        self.board_combos = (1,self.FLOPS, self.TURNS, self.RIVERS)
        self.board_type_count = (1,self.FLOPS, self.FLOPS*52, self.TURNS*52)

        self.keep_ref = []
        
        self.get_board_slots_all_gs_prev_board = [-1]*5
        self.get_board_slots_all_gs_prev_slots = [-1]*4

#        self.flop_types_raw = None
#        self.preflop_types = None
#        self.flop_types = None
#        self.turn_types = None
#        self.preflop_slots = None
#        self.flop_slots = None
#        self.turn_slots = None

        self.traverse_jbs_off = None
        self.traverse_jbs_data_file = None
        self.traverse_jbs_data_mmap = None

        self.ctoi2 = lzp.ctoi2 #(c_int*52*52).in_dll(lzp.lib, "cards_to_int_2")
        self.ctoi3 = lzp.ctoi3 #(c_int*52*52*52).in_dll(lzp.lib, "cards_to_int_3")
        self.ctoi4 = lzp.ctoi4 #(c_int*52*52*52*52).in_dll(lzp.lib, "cards_to_int_4")

        self.itoc2 = lzp.itoc2 #(cards_2*self.HANDS).in_dll(lzp.lib, "int_to_cards_2")
        self.itoc3 = lzp.itoc3 #(cards_3*self.FLOPS).in_dll(lzp.lib, "int_to_cards_3")
        self.itoc4 = lzp.itoc4 #(cards_4*self.TURNS).in_dll(lzp.lib, "int_to_cards_4")

        self.preflop_alltypes = None
        self.types = [None, None, None, None]
        self.diffs = [None, None, None, None]
        self.diffs_order = [None, None, None, None]
        self.slots = [None, None, None, None]
        self.b_types = [None, None, None, None]
        self.b_diffs = [None, None, None, None]
        self.b_slots = [None, None, None, None]
        self.gs_switch_hand = [None, None, None, None]
        self.gs_odds_hand = [None, None, None, None]
        self.gs_switch_board = [None, None, None, None]
        self.gs_odds_board = [None, None, None, None]
        self.path_odds = [None]*4
        self.gs_switch_path = []
        self.hand_wtl_odds = None
        
        
        self.i = dt.gameinfo()
        self.i.types = (None, None, None, None)
        self.i.diffs = (None, None, None, None)
        self.i.diffs_order = (None, None, None, None)
        self.i.slots = (None, None, None, None)
        self.i.b_types = (None, None, None, None)
        self.i.b_diffs = (None, None, None, None)
        self.i.b_slots = (None, None, None, None)
        self.i.gs_switch_hand = (None, None, None, None)
        self.i.gs_odds_hand = (None, None, None, None)
        self.i.gs_switch_board = (None, None, None, None)
        self.i.gs_odds_board = (None, None, None, None)
        self.i.hand_wtl_odds = None
        self.i.n_public_types = 0
        self.i.n_private_types = 0
        
        #self.i.gs_trans = (None, None, None, None)
        
        self.gamedir = gamedir
        if self.gamedir != "":
            if self.gamedir[-1] != "/":
                self.gamedir += "/"
            cfg = ConfigParser.RawConfigParser()
            cfg.read(self.gamedir + 'config.cfg')
            self.i.n_types[0] = cfg.getint("n_types", "preflop")
            self.i.n_types[1] = cfg.getint("n_types", "flop")
            self.i.n_types[2] = cfg.getint("n_types", "turn")
            self.i.n_types[3] = cfg.getint("n_types", "river")

            self.i.n_rtypes[0] = cfg.getint("n_rtypes", "preflop")
            self.i.n_rtypes[1] = cfg.getint("n_rtypes", "flop")
            self.i.n_rtypes[2] = cfg.getint("n_rtypes", "turn")
            self.i.n_rtypes[3] = cfg.getint("n_rtypes", "river")

            self.i.n_btypes[0] = cfg.getint("n_btypes", "preflop")
            self.i.n_btypes[1] = cfg.getint("n_btypes", "flop")
            self.i.n_btypes[2] = cfg.getint("n_btypes", "turn")
            self.i.n_btypes[3] = cfg.getint("n_btypes", "river")

            self.i.n_htfb[0] = cfg.getint("hand_types_for_board", "preflop")
            self.i.n_htfb[1] = cfg.getint("hand_types_for_board", "flop")
            self.i.n_htfb[2] = cfg.getint("hand_types_for_board", "turn")
            self.i.n_htfb[3] = cfg.getint("hand_types_for_board", "river")
            
            self.i.gauss_width[0] = cfg.getfloat("gauss_width", "preflop")
            self.i.gauss_width[1] = cfg.getfloat("gauss_width", "flop")
            self.i.gauss_width[2] = cfg.getfloat("gauss_width", "turn")
            self.i.gauss_width[3] = cfg.getfloat("gauss_width", "river")
            
            self.i.n_plr = cfg.getint("misc", "n_plr")
            
            self.i.maxbets = cfg.getint("misc", "maxbets")
            self.i.sb = cfg.getfloat("misc", "sb")
            self.i.bb = cfg.getfloat("misc", "bb")
            
            #LOAD TYPES
            self.type_filenames = [[],[]]
            self.i.n_type_types[1] = cfg.getint("types_info", "n_public_types")
            self.i.n_type_types[0] = cfg.getint("types_info", "n_private_types")
            self.i.type_types[1] = (dt.type_type*self.i.n_type_types[1])()
            self.i.type_types[0] = (dt.type_type*self.i.n_type_types[0])()
            print self.i.n_type_types[1], self.i.n_type_types[0], self.i.type_types[1], self.i.type_types[0]
            for i in xrange(self.i.n_type_types[1]):
                filenames = {}
                new_t = self.i.type_types[1][i]
                new_t.id = i
                new_t.public = 1
                new_t.info = pointer(self.i)
                new_t.style = cfg.getint("public_type_"+str(i), "style")
                new_t.gamestate = cfg.getint("public_type_"+str(i), "gamestate")
                new_t.n_types = cfg.getint("public_type_"+str(i), "n_types")
                new_t.n_items_per_type = cfg.getint("public_type_"+str(i), "n_items_per_type")
                new_t.local = cfg.getint("public_type_"+str(i), "local")
                #new_t.diffs_order_filename = cfg.get("public_type_"+str(i), "diffs_order_filename")
                #new_t.diffs_filename = cfg.get("public_type_"+str(i), "diffs_filename")
                #new_t.types_filename = cfg.get("public_type_"+str(i), "types_filename")
                #new_t.slots_filename = cfg.get("public_type_"+str(i), "slots_filename")
                new_t.diffs_order = None
                new_t.diffs = None
                new_t.types = None
                new_t.slots = None
                get_slot_func_name = cfg.get("public_type_"+str(i), "get_slot_func")
                gen_type_func_name = cfg.get("public_type_"+str(i), "gen_type_func")
                #new_t.get_slot = eval("addressof(lzp." + get_slot_func_name+")")
                new_t.get_slot = eval("cast(lzp." + get_slot_func_name+", c_void_p)")
                if gen_type_func_name != "":
                    new_t.gen_type = eval("cast(lzp." + gen_type_func_name+", c_void_p)")
                filenames["diffs_order"] = cfg.get("public_type_"+str(i), "diffs_order_filename")
                filenames["diffs"] = cfg.get("public_type_"+str(i), "diffs_filename")
                filenames["diffs_lookup"] = cfg.get("public_type_"+str(i), "diffs_lookup_filename")
                filenames["types"] = cfg.get("public_type_"+str(i), "types_filename")
                filenames["slots"] = cfg.get("public_type_"+str(i), "slots_filename")
                self.type_filenames[1].append(filenames)
                
                #new_t.print_fields()
            for i in xrange(self.i.n_type_types[0]):

                filenames = {}
                new_t = self.i.type_types[0][i]
                new_t.id = i
                new_t.public = 0
                new_t.info = pointer(self.i)
                new_t.style = cfg.getint("private_type_"+str(i), "style")
                new_t.gamestate = cfg.getint("private_type_"+str(i), "gamestate")
                new_t.n_types = cfg.getint("private_type_"+str(i), "n_types")
                new_t.n_items_per_type = cfg.getint("private_type_"+str(i), "n_items_per_type")
                new_t.local = cfg.getint("private_type_"+str(i), "local")
                # new_t.diffs_order_filename = cfg.get("private_type_"+str(i), "diffs_order_filename")
                # new_t.diffs_filename = cfg.get("private_type_"+str(i), "diffs_filename")
                # new_t.types_filename = cfg.get("private_type_"+str(i), "types_filename")
                # new_t.slots_filename = cfg.get("private_type_"+str(i), "slots_filename")
                new_t.diffs_order = None
                new_t.diffs = None
                new_t.types = None
                new_t.slots = None

                get_slot_func_name = cfg.get("private_type_"+str(i), "get_slot_func")
                gen_type_func_name = cfg.get("private_type_"+str(i), "gen_type_func")
                #new_t.get_slot = eval("addressof(lzp." + get_slot_func_name+")")
                new_t.get_slot = eval("cast(lzp." + get_slot_func_name+", c_void_p)")
                if gen_type_func_name != "":
                    new_t.gen_type = eval("cast(lzp." + gen_type_func_name+", c_void_p)")
                
                
                filenames["diffs_order"] = cfg.get("private_type_"+str(i), "diffs_order_filename")
                filenames["diffs"] = cfg.get("private_type_"+str(i), "diffs_filename")
                filenames["diffs_lookup"] = cfg.get("private_type_"+str(i), "diffs_lookup_filename")
                filenames["types"] = cfg.get("private_type_"+str(i), "types_filename")
                filenames["slots"] = cfg.get("private_type_"+str(i), "slots_filename")
                self.type_filenames[0].append(filenames)
                
                #new_t.print_fields()
            
        else:
            print "no config"
            sys.exit(0)
            self.i.n_types = (169,8192,8192,1024)
            self.i.n_rtypes[0] = 32
            self.i.n_rtypes[1] = 64
            self.i.n_rtypes[2] = 128
            self.i.n_rtypes[3] = 3 
            self.i.gauss_width[0] = 0.1
            self.i.gauss_width[1] = 0.1
            self.i.gauss_width[2] = 0.1
            self.i.gauss_width[3] = 0.1
        
        self.precalc_and_index_all_unique_states()

        np.set_printoptions(threshold=2000, linewidth=400, precision=5, suppress=True)


    #C FUNCTION WRAPS

    def gen_types_for_flop(self, flop, use_file_if_possible = True):
        #r = self.i.rtype[1]
        if use_file_if_possible:
            path = self.gamedir + "flop_types_raw/"
            if os.path.isdir(path):
                return np.fromfile(path+str(flop)+".ftype", dtype=np.float64).reshape((self.HANDS, self.i.n_rtypes[1]))
        
        tmp_types = (c_double*(self.i.n_rtypes[1]*self.HANDS))()
        lzp.lib.gen_types_for_flop(pointer(self.i), pointer(tmp_types), flop)
        return np.array(tmp_types, dtype=np.float64).reshape((self.HANDS, self.i.n_rtypes[1]))

    def gen_types_for_turn(self, flop, turn): 
        #r = self.i.rtype[2]
        tmp_types = np.zeros((self.i.n_rtypes[2], self.HANDS), dtype=np.float64)
        #tmp_types = (c_double*(self.i.n_rtypes[2]*self.HANDS))()
        lzp.lib.gen_types_for_turn(pointer(self.i), tmp_types.ctypes.data, flop, turn)
        #return np.array(tmp_types, dtype=np.float64).reshape((self.HANDS, self.i.n_rtypes[2]))
        return tmp_types
    
    def gen_all_preflop_types(self, use_file_if_possible = True):
        #r = self.i.rtype[0]
        filename = self.gamedir + "hand_types_gs" + str(0) + "_n"+ str(self.HANDS)  + "_r" + str(self.i.n_rtypes[0])  + ".ftype"
        if use_file_if_possible:
            if os.path.isfile(filename):
                self.preflop_alltypes =  np.fromfile(filename, np.float64).reshape((self.HANDS, self.i.n_rtypes[0]))
                return self.preflop_alltypes

        tmp_types = (c_double*(self.i.n_rtypes[0]*self.HANDS))()
        
        tmp_river_n_types = self.i.n_types[3] #otetaan talteen, kun pitaa laskun ajaksi ladata eri hand typet riverille. Jotain jarkea tahan joskus sit.
        self.i.n_types[3] = self.i.n_rtypes[0]
        self.load_hand_types(3)
        self.load_hand_diffs(3)
        
        lzp.lib.gen_all_preflop_types(pointer(self.i), pointer(tmp_types))
        self.preflop_alltypes = np.array(tmp_types, dtype=np.float64).reshape((self.HANDS, self.i.n_rtypes[0]))
        self.preflop_alltypes.tofile(filename)
        
        self.i.n_types[3] = tmp_river_n_types
        self.load_hand_types(3)
        self.load_hand_diffs(3)

        return self.preflop_alltypes

    def gen_random_turn_type(self):
        tmp_type = (c_double*(self.i.n_rtypes[2]))()
        lzp.lib.gen_random_turn_type(pointer(self.i), pointer(tmp_type))
        retval = np.array(tmp_type, dtype=np.float64).reshape((self.i.n_rtypes[2]))
        return retval

    def gen_random_river_type(self):
        tmp_type = np.zeros(self.i.n_rtypes[3], dtype=np.float32)
        #tmp_type = (c_double*self.i.n_rtypes[3])()
        lzp.lib.gen_random_river_type(pointer(self.i), tmp_type.ctypes.data_as(POINTER(c_float)))
        #retval = np.array(tmp_type, dtype=np.float64).reshape((self.i.n_rtypes[3]))
        #return retval
        return tmp_type
    
    def gen_single_river_type(self, board, hand):
        tmp_type = (c_double*self.i.n_rtypes[3])()
        if type(hand) != int:
            hand = self.ctoi2[hand[0]][hand[1]]
        lzp.lib.gen_single_river_type(pointer(self.i), pointer(tmp_type), self.ctoi3[board[0]][board[1]][board[2]], board[3], board[4], hand)
        retval = np.array(tmp_type, dtype=np.float64).reshape((self.i.n_rtypes[3]))
        return retval

    def get_difference(self, t1, t2):
        n_slots = t1.size
        return lzp.lib.get_difference(t1.ctypes.data, t2.ctypes.data, n_slots)

    def get_difference_pow2(self, t1, t2):
        n_slots = t1.size
        return lzp.lib.get_difference_pow2(t1.ctypes.data, t2.ctypes.data, n_slots)

    #def get_diff_cuda(self, t1, t2):
    #    n_slots = t1.size
    #    return lzp.lib.get_diff_cuda(t1.ctypes.data, t2.ctypes.data, n_slots)
    

    def calc_diffs_for_one_type(self, types, type, diffs, n_types, n_slots_per_type):
        lzp.lib.calc_diffs_for_one_type(types.ctypes.data, type.ctypes.data, diffs.ctypes.data, n_types, n_slots_per_type)

    def try_new_type(self, gs, new_type, smallest, replace_i):
        types = self.i.types[gs]
        diffs = self.i.diffs[gs]
        t = new_type.ctypes.data
        n_types = self.i.n_types[gs]
        n_slots_per_type = self.i.wl_slots*self.i.tie_slots

        retval = lzp.lib.try_new_type(types, diffs, t, n_types, n_slots_per_type, smallest, replace_i)
        return retval


    def calc_diffs(self, gs, t):
        n_types = self.i.n_types[gs]
        if n_types == 0:
            return -1
        n_slots_per_type = self.i.wl_slots * self.i.tie_slots
        #n_slots_per_type = len(types[0])*len(types[0][0])
        #   print n_types, n_slots_per_type
        types = self.i.types[gs]
        diffs = self.i.diffs[gs]
        #print "start diff calc", types_as_c, diffs_as_c, n_types, n_slots_per_type, t
        retval = lzp.lib.calc_diffs(types, diffs, n_types, n_slots_per_type, t)
        #print "done", retval
        return retval

    def get_slot(self, gs, t):
        slot = lzp.lib.get_slot(self.i.types[gs], t.ctypes.data, self.i.n_types[gs], self.i.n_rtypes[gs])
        return slot

    def get_slot_new(self, types, type, n_types, n_slots_per_type):
        slot = lzp.lib.get_slot(types, type, n_types, n_slots_per_type)
        return slot

    def get_slots_for_all(self, gs, slots_target, types):
        if slots_target == None:
            slots_target = np.zeros((self.HANDS), dtype = np.int16)
        lzp.lib.get_slots_for_all(slots_target.ctypes.data, types.ctypes.data, self.i.types[gs], slots_target.size, self.i.n_types[gs], self.i.n_rtypes[gs])
        return slots_target

    def get_slots_for_river(self, flop_i, turn_i, river_i, slots = None):
        if slots == None:
            slots = np.zeros((self.HANDS), dtype = np.int16)
        lzp.lib.get_slots_for_river(pointer(self.i), slots.ctypes.data, flop_i, turn_i, river_i)
        return slots
    
    def get_closest_match(self, gs):
        diffs_as_c = self.diffs[gs].ctypes.data
        n_types = self.diffs[gs].shape[0]
        print n_types
        t1_c = c_int()
        t2_c = c_int()
        
        small = lzp.lib.get_closest_match(diffs_as_c, n_types, pointer(t1_c), pointer(t2_c))
        #print small, t1_c, t2_c
        return small, [(t1_c.value, t2_c.value)]
    
    def generate_mapping_from_diffs(self, gs, reduce_to):
        mapping = np.arange(self.i.n_types[gs], dtype=np.int32)
        lzp.lib.generate_mapping_from_diffs(self.i.diffs[gs], mapping.ctypes.data, self.i.n_types[gs], reduce_to)
        return mapping
            
    def gen_board_type(self, gs, flop, turn = -1, river = -1, bt = None):
        if bt == None:
            bt = np.zeros((self.i.n_htfb[gs-1], self.i.n_htfb[gs]), dtype = np.float64)
        #for x in self.diffs_order[gs-1]:
        #    print x
        lzp.lib.gen_board_type(pointer(self.i), bt.ctypes.data, gs, flop, turn, river)

        return bt


    def get_slots_wtl_stats_for_river(self, wtl_stats, flop, turn, river):
        return lzp.lib.get_slots_wtl_stats_for_river(pointer(self.i), wtl_stats.ctypes.data, flop, turn, river)


    def find_two_smallest(self, diffs, diffs_order, new_type):
        lzp.lib.find_two_smallest(diffs.ctypes.data, diffs_order.ctypes.data, len(diffs), new_type)


    def pair_and_minimize_diffs(self, diffs, diffs_order):
        n_types = len(diffs)
        new_diffs = np.zeros((n_types/2, n_types/2), dtype = np.float64)
        lzp.lib.pair_and_minimize_diffs(diffs.ctypes.data, diffs_order.ctypes.data, new_diffs.ctypes.data, n_types)


    #PURE PYTHON


    def load_types_for_type(self, t):
        filename = self.type_filenames[t.public][t.id]["types"]
        if filename == "":
            return
        #print "loading types", self.gamedir, filename, t.n_types,t.n_items_per_type
        try:
            np_types = np.fromfile(self.gamedir+filename, np.float32).reshape((t.n_types, t.n_items_per_type))
            self.keep_ref.append(np_types)
            t.types = np_types.ctypes.data_as(POINTER(c_float))
        except IOError:
            t.types = None
            print "file not found:", filename

    def load_diffs_order_for_type(self, t, memory_map = False):
        filename = self.type_filenames[t.public][t.id]["diffs_order"]
        if filename == "":
            return
        #print "loading diffs order", self.gamedir, filename
        if memory_map:
            np_diffs_order = np.memmap(self.gamedir+filename, np.int16, "r", shape = (t.n_types, t.n_types))
        else:
            np_diffs_order = np.fromfile(self.gamedir+filename, np.int16).reshape((t.n_types, t.n_types))
        self.keep_ref.append(np_diffs_order)
        t.diffs_order = np_diffs_order.ctypes.data_as(POINTER(c_int16))

    def load_diffs_for_type(self, t, memory_map = False):
        filename = self.type_filenames[t.public][t.id]["diffs"]
        if filename == "":
            return
        #print "loading diffs", self.gamedir, filename
        if memory_map:
            np_diffs = np.memmap(self.gamedir+filename, np.float32, "r", shape = (t.n_types, t.n_types))
        else:
            np_diffs = np.fromfile(self.gamedir+filename, np.float32).reshape((t.n_types, t.n_types))
        self.keep_ref.append(np_diffs)
        t.diffs = np_diffs.ctypes.data_as(POINTER(c_float))
        self.load_diffs_order_for_type(t, memory_map)

    def load_diffs_lookup_for_type(self, t):
        t.print_fields()
        filename = self.type_filenames[t.public][t.id]["diffs_lookup"]
        if filename == "":
            return
        #print "loading diffs lookup", self.gamedir, filename
        np_diffs_lookup = np.fromfile(self.gamedir+filename, np.uint64).reshape((t.n_types, 10, (t.n_types-1)/64+1))
        self.keep_ref.append(np_diffs_lookup)
        t.diffs_lookup = np_diffs.ctypes.data_as(POINTER(c_uint64))

        
    def load_slots_for_type(self, t):
        filename = self.type_filenames[t.public][t.id]["slots"]
        if filename == "":
            return
        #print "loading slots", self.gamedir, filename
        #np.memmap(filename, dtype='float32', mode='w+', shape=(3,4))
        np_slots = np.memmap(self.gamedir+filename, dtype=np.int16, mode="r")
        #pdb.set_trace()
        #np_slots = np.fromfile(self.gamedir+filename, np.int16)
        self.keep_ref.append(np_slots)
        t.slots = np_slots.ctypes.data_as(POINTER(c_int16))
        
#    def calc_diffs_order_for_type(self, t):
#        assert(t.diffs != None)
#        np_diffs_order = self.calc_diffs_order_from_diffs(t.np_diffs)
#        self.keep_ref.append(np_diffs_order)
#        t.diffs_order = np_diffs_order.ctypes.data_as(POINTER(c_int16))
#        print np_diffs_order
#        print t.diffs_order[0]
#        print np_diffs_order.ctypes.data

    def set_info_pointers(self):
        for i in xrange(4):
            self.i.diffs[i] = self.i.type_types[0][i].diffs
            self.i.diffs_order[i] = self.i.type_types[0][i].diffs_order
            self.i.types[i] = self.i.type_types[0][i].types
            self.i.slots[i] = self.i.type_types[0][i].slots

        for i in xrange(1,4):
            self.i.b_types[i] = self.i.type_types[1][10-i].types
            self.i.b_slots[i] = self.i.type_types[1][10-i].slots
            self.i.b_diffs[i] = self.i.type_types[1][10-i].diffs
            self.i.b_diffs_order[i] = self.i.type_types[1][10-i].diffs_order

        
    def get_types_from_hand_info(self):
        pass

    def load_diffs_lookup_for_all_types(self):
        for i in xrange(self.i.n_type_types[0]):
            t = self.i.type_types[0][i]
            if t.style == 2:
                self.load_diffs_lookup_for_type(t)

    def load_diffs_for_all_types(self, memory_map = False):
        for i in xrange(self.i.n_type_types[0]):
            t = self.i.type_types[0][i]
            if t.style == 2:
                self.load_diffs_for_type(t, memory_map)
#                self.calc_diffs_order_for_type(t)
        for i in xrange(self.i.n_type_types[1]):
            t = self.i.type_types[1][i]
            if t.style == 2:
                self.load_diffs_for_type(t, memory_map)
#                self.calc_diffs_order_for_type(t)
                
    def load_types_for_all_types(self):
        for pub in xrange(2):
            for i in xrange(self.i.n_type_types[pub]):
                t = self.i.type_types[pub][i]
                self.load_types_for_type(t)

    def load_slots_for_all_types(self):
        for pub in xrange(2):
            for i in xrange(self.i.n_type_types[pub]):
                t = self.i.type_types[pub][i]
                self.load_slots_for_type(t)

                
    def load_hand_types(self, gs):        
        filename = self.gamedir + "hand_types_gs" + str(gs) + "_n"+ str(self.i.n_types[gs])  + "_r" + str(self.i.n_rtypes[gs])  + ".ftype"
        try:
            self.types[gs] = np.fromfile(filename, np.float32).reshape((self.i.n_types[gs], self.i.n_rtypes[gs]))
        except:
            print "load_hand_types error", sys.exc_info()
            print "hand type file not fould", filename
            return False
        self.i.types[gs] = self.types[gs].ctypes.data_as(POINTER(c_float))

    def save_hand_types(self, gs):
        filename = self.gamedir + "hand_types_gs" + str(gs) + "_n"+ str(self.i.n_types[gs])  + "_r" + str(self.i.n_rtypes[gs])  + ".ftype"
        self.types[gs].tofile(filename)

    def load_hand_diffs(self, gs):
        filename = self.gamedir + "hand_diffs_gs" + str(gs) + "_n"+ str(self.i.n_types[gs])  + "_r" + str(self.i.n_rtypes[gs]) + ".ftype"
        try:
            self.diffs[gs] = np.fromfile(filename, np.float32).reshape((self.i.n_types[gs], self.i.n_types[gs]))
        except:
            print "hand diffs file not fould", filename
            if self.types[gs] != None:
                print "generating diffs from types"
                diffs = np.zeros((self.i.n_types[gs], self.i.n_types[gs]), dtype = np.float64)
                i = 0
                while i < self.i.n_types[gs]:
                    print i
                    j = i
                    while j < self.i.n_types[gs]:
                        d = ((self.types[gs][i] - self.types[gs][j])**2).sum()
                        diffs[i,j] = d
                        diffs[j,i] = d
                        j+=1
                    i+=1
                self.diffs[gs] = diffs
                return True
            return False
        self.i.diffs[gs] = self.diffs[gs].ctypes.data_as(POINTER(c_float))
        self.calc_diffs_order_from_diffs_gs(gs)
        totdiff = 0
        i = 0
        while i < len(self.diffs[gs]):
            assert i == self.diffs_order[gs][i,-1]
            assert i != self.diffs_order[gs][i,-2]
            #print i, self.diffs_order[gs][i,-2]
            totdiff += self.diffs[gs][i,self.diffs_order[gs][i,0]]
            i+=1
        print "min totdiff", totdiff
        return True

    def save_hand_diffs(self, gs):
        filename = self.gamedir + "hand_diffs_gs" + str(gs) + "_n"+ str(self.i.n_types[gs])  + "_r" + str(self.i.n_rtypes[gs]) + ".ftype"
        self.diffs[gs].tofile(filename)
    
    def load_hand_slots(self, gs):
        if gs == 3:
            return True
        filename = self.gamedir + "slots_hand_gs" + str(gs) + "_n"+ str(self.i.n_types[gs])  + "_r" + str(self.i.n_rtypes[gs]) + ".slots"
        if gs == 0:
            self.slots[gs] = np.fromfile(filename, np.int16).reshape((self.HANDS))
        else:
            self.slots[gs] = np.fromfile(filename, np.int16).reshape((self.board_combos[gs], self.HANDS))
        
        self.i.slots[gs] = self.slots[gs].ctypes.data_as(POINTER(c_short))
        return True

    def save_hand_slots(self, gs):
        if self.slots[gs] == None:
            return False
        filename = self.gamedir + "slots_hand_gs" + str(gs) + "_n"+ str(self.i.n_types[gs])  + "_r" + str(self.i.n_rtypes[gs]) + ".slots"
        self.slots[gs].tofile(filename)

    def load_board_types(self, gs):
#        filename = self.gamedir + "board_types_gs" + str(gs) + "_s" + str(self.i.n_btypes[gs]) + "_tb"+ str(self.i.n_types[gs-1]) + "_te"+ str(self.i.n_types[gs]) + ".ftype"
        filename = self.gamedir + "board_types_gs" + str(gs) + "_s" + str(self.i.n_btypes[gs]) + "_tb"+ str(self.i.n_htfb[gs-1]) + "_te"+ str(self.i.n_htfb[gs]) + ".ftype"
        self.b_types[gs] =  np.fromfile(filename, np.float32).reshape((self.i.n_btypes[gs], self.i.n_htfb[gs-1], self.i.n_htfb[gs]))
        self.i.b_types[gs] =  self.b_types[gs].ctypes.data_as(POINTER(c_float))

        return True
    
    def save_board_types(self, gs):
        
        filename = self.gamedir + "board_types_gs" + str(gs) + "_s" + str(self.i.n_btypes[gs]) + "_tb"+ str(self.i.n_htfb[gs-1]) + "_te"+ str(self.i.n_htfb[gs]) + ".ftype"
        self.b_types[gs].tofile(filename)
        
    def load_board_diffs(self, gs):
        
        filename = self.gamedir + "board_diffs_gs" + str(gs) + "_s" + str(self.i.n_btypes[gs]) + "_tb"+ str(self.i.n_htfb[gs-1]) + "_te"+ str(self.i.n_htfb[gs]) + ".ftype"
        self.b_diffs[gs] =  np.fromfile(filename, np.float32).reshape((self.i.n_btypes[gs], self.i.n_btypes[gs]))
        self.i.b_diffs[gs] =  self.b_diffs[gs].ctypes.data_as(POINTER(c_float))

        return True

    def save_board_diffs(self, gs):
        filename = self.gamedir + "board_diffs_gs" + str(gs) + "_s" + str(self.i.n_btypes[gs]) + "_tb"+ str(self.i.n_htfb[gs-1]) + "_te"+ str(self.i.n_htfb[gs]) + ".ftype"
        
        self.b_diffs[gs].tofile(filename)
        return True


    def load_board_slots(self, gs):
        filename = self.gamedir + "board_slots_gs" + str(gs) + "_s" + str(self.i.n_btypes[gs]) + "_tb"+ str(self.i.n_htfb[gs-1]) + "_te"+ str(self.i.n_htfb[gs]) + ".slots"
#        filename = self.gamedir + "board_slots_gs" + str(gs) + "_s" + str(self.i.n_btypes[gs]) + "_tb"+ str(self.i.n_types[gs-1]) + "_te"+ str(self.i.n_types[gs]) + ".slots"
#        filename = self.gamedir + "board_slots_gs" + str(gs) + "_s" + str(self.i.n_btypes[gs]) + "_tb"+ str(self.i.n_types[gs-1]) + "_te"+ str(self.i.n_types[gs]) + "_wlsl" + str(r.wl_slots) + "_tiesl" + str(r.tie_slots) + "_wlsc" + str(r.wl_scale) + "_tiesc" + str(r.tie_scale) + "_wlw" + str(r.wl_width) + "_tiew" + str(r.tie_width) + ".ftype"
        self.b_slots[gs] =  np.fromfile(filename, np.int16).reshape((self.board_type_count[gs]))
        self.i.b_slots[gs] =  self.b_slots[gs].ctypes.data_as(POINTER(c_short))

        return True
    
    def save_board_slots(self, gs):
        filename = self.gamedir + "board_slots_gs" + str(gs) + "_s" + str(self.i.n_btypes[gs]) + "_tb"+ str(self.i.n_htfb[gs-1]) + "_te"+ str(self.i.n_htfb[gs]) + ".slots"
#        filename = self.gamedir + "board_slots_gs" + str(gs) + "_s" + str(self.i.n_btypes[gs]) + "_tb"+ str(self.i.n_types[gs-1]) + "_te"+ str(self.i.n_types[gs]) + ".slots"
#        filename = "board_slots_gs" + str(gs) + "_s" + str(self.i.n_btypes[gs]) + "_tb"+ str(self.i.n_types[gs-1]) + "_te"+ str(self.i.n_types[gs]) + "_wlsl" + str(r.wl_slots) + "_tiesl" + str(r.tie_slots) + "_wlsc" + str(r.wl_scale) + "_tiesc" + str(r.tie_scale) + "_wlw" + str(r.wl_width) + "_tiew" + str(r.tie_width) + ".ftype"
        self.b_slots[gs].tofile(filename)


    def load_gs_hand_switch_tables(self, gs):
        filename = self.gamedir + "gs_hand_switch_table_gs" + str(gs) + "_s" + str(self.i.n_btypes[gs]) + "_htb"+ str(self.i.n_types[gs-1]) + "_hte"+ str(self.i.n_types[gs]) + ".hand_switch"
        self.gs_switch_hand[gs] = np.fromfile(filename, np.float64).reshape((self.i.n_btypes[gs]*2, self.i.n_types[gs-1]*2, self.i.n_types[gs]*2))
        self.i.gs_switch_hand[gs] =  self.gs_switch_hand[gs].ctypes.data_as(POINTER(c_double))
        
    def save_gs_hand_switch_tables(self, gs):
        filename = self.gamedir + "gs_hand_switch_table_gs" + str(gs) + "_s" + str(self.i.n_btypes[gs]) + "_htb"+ str(self.i.n_types[gs-1]) + "_hte"+ str(self.i.n_types[gs]) + ".hand_switch"
        self.gs_switch_hand[gs].tofile(filename)
        
    def load_gs_hand_odds_tables(self, gs):
        filename = self.gamedir + "gs_hand_odds_table_gs" + str(gs) + "_s" + str(self.i.n_btypes[gs]) + "_ht"+ str(self.i.n_types[gs]) + ".hand_odds"
        self.gs_odds_hand[gs] = np.fromfile(filename, np.float64).reshape((self.i.n_btypes[gs]*2, self.i.n_types[gs]*2))
        self.i.gs_odds_hand[gs] =  self.gs_odds_hand[gs].ctypes.data_as(POINTER(c_double))
        #if gs == 0:
        #    print  self.gs_odds_hand[gs]
        #    sys.exit()

    def save_gs_hand_odds_tables(self, gs):
        filename = self.gamedir + "gs_hand_odds_table_gs" + str(gs) + "_s" + str(self.i.n_btypes[gs]) + "_ht"+ str(self.i.n_types[gs]) + ".hand_odds"
        self.gs_odds_hand[gs].tofile(filename)

    def load_gs_board_switch_tables(self, gs):
        slots = ""
        shape = []
        for x in range(1,gs+1):
            slots += str(self.i.n_btypes[x]) + "_"
            shape.append(self.i.n_btypes[x]*2)

        filename = self.gamedir + "gs_board_switch_table_gs" + str(gs) + "_s" + slots + ".board_switch"
        self.gs_switch_board[gs] = np.fromfile(filename, np.float64).reshape(shape)
        self.i.gs_switch_board[gs] =  self.gs_switch_board[gs].ctypes.data_as(POINTER(c_double))
        
    def save_gs_board_switch_tables(self, gs):
        slots = ""
        for x in range(1,gs+1):
            slots += str(self.i.n_btypes[x]) + "_"

        filename = self.gamedir + "gs_board_switch_table_gs" + str(gs) + "_s" + slots + ".board_switch"
        self.gs_switch_board[gs].tofile(filename)
        
    def load_gs_board_odds_tables(self, gs):
        filename = self.gamedir + "gs_board_odds_table_gs" + str(gs) + "_s" + str(self.i.n_btypes[gs]) + ".board_odds"
        self.gs_odds_board[gs] = np.fromfile(filename, np.float64).reshape((self.i.n_btypes[gs]*2))
        self.i.gs_odds_board[gs] =  self.gs_odds_board[gs].ctypes.data_as(POINTER(c_double))

    def save_gs_board_odds_tables(self, gs):
        filename = self.gamedir + "gs_board_odds_table_gs" + str(gs) + "_s" + str(self.i.n_btypes[gs]) + ".board_odds"
        self.gs_odds_board[gs].tofile(filename)


    def load_gs_switch_path(self):
        filename = self.gamedir + "gs_switch_path_nplr" + str(self.i.n_plr) + "_sb" + str(self.i.sb) + "_bb" + str(self.i.bb) + "_maxbets" + str(self.i.maxbets) + ".path_switch"
        self.gs_switch_path = np.fromfile(filename, np.float64).reshape((self.n_us,4,4,8))
        self.i.gs_switch_path = self.gs_switch_path.ctypes.data_as(POINTER(c_double))
#        f = open(filename, "rb")
#        self.gs_switch_path = cPickle.load(f)
#        f.close()

    def save_gs_switch_path(self):
        filename = self.gamedir + "gs_switch_path_nplr" + str(self.i.n_plr) + "_sb" + str(self.i.sb) + "_bb" + str(self.i.bb) + "_maxbets" + str(self.i.maxbets) + ".path_switch"
        self.gs_switch_path.tofile(filename)
        #f = open(filename, "wb")
        #self.gs_switch_path = cPickle.dump(self.gs_switch_path, f)
        #f.close()

    def load_path_odds(self):
        filename = self.gamedir + "path_odds"+ "_nplr" + str(self.i.n_plr) + "_sb" + str(self.i.sb) + "_bb" + str(self.i.bb) + "_maxbets" + str(self.i.maxbets) + ".path_odds"
        self.path_odds = np.fromfile(filename, np.float64).reshape((4,4,4,8))
        self.i.path_odds =  self.path_odds.ctypes.data_as(POINTER(c_double))

    def save_path_odds(self):
        filename = self.gamedir + "path_odds"+ "_nplr" + str(self.i.n_plr) + "_sb" + str(self.i.sb) + "_bb" + str(self.i.bb) + "_maxbets" + str(self.i.maxbets) + ".path_odds"
        self.path_odds.tofile(filename)

    def load_hand_wtl_odds(self):
        filename = self.gamedir + "hand_wtl_odds"+ "_nbtypes" + str(self.i.n_btypes[3]) + "_nhtypes" + str(self.i.n_types[3]) + ".hand_wtl_odds"
        self.hand_wtl_odds = np.fromfile(filename, np.float64).reshape((self.i.n_types[3]*2, self.i.n_types[3]*2, 3))
        self.i.hand_wtl_odds =  self.hand_wtl_odds.ctypes.data_as(POINTER(c_double))

    def save_hand_wtl_odds(self):
        filename = self.gamedir + "hand_wtl_odds"+ "_nbtypes" + str(self.i.n_btypes[3]) + "_nhtypes" + str(self.i.n_types[3]) + ".hand_wtl_odds"    
        self.hand_wtl_odds.tofile(filename)
    
    def load_jbs(self):
        filename_off = self.gamedir + "jbs_bs_" + str(self.i.n_btypes[1]) + "_" + str(self.i.n_btypes[2]) + "_" + str(self.i.n_btypes[3]) + "_hs_" + str(self.i.n_types[0]) + "_" + str(self.i.n_types[1]) + "_" + str(self.i.n_types[2]) + "_" + str(self.i.n_types[3]) + ".offset"
        filename_data = self.gamedir + "jbs_bs_" + str(self.i.n_btypes[1]) + "_" + str(self.i.n_btypes[2]) + "_" + str(self.i.n_btypes[3]) + "_hs_" + str(self.i.n_types[0]) + "_" + str(self.i.n_types[1]) + "_" + str(self.i.n_types[2]) + "_" + str(self.i.n_types[3]) + ".data"

        self.jbs_off = np.fromfile(filename_off, dtype=np.int64).reshape(-1,2)
        self.jbs_data_file = file(filename_data, "r+b")
        self.jbs_data_mmap = mmap.mmap(self.jbs_data_file.fileno(), 0)
        
    def get_random_jbs(self, n_plr):
        jbs_i = random.randint(0, len(self.jbs_off)-1)
        #data_str = self.jbs_data_mmap[self.jbs_off[jbs_i][0]:self.jbs_off[jbs_i].sum()]
        data_str = zlib.decompress(self.jbs_data_mmap[self.jbs_off[jbs_i][0]:self.jbs_off[jbs_i].sum()])
        board_data = np.fromstring(data_str[:4*2], dtype=np.int16)
        hand_data =  np.fromstring(data_str[4*2:4*2+self.HANDS*4*2], dtype=np.int16).reshape(-1,4)
        handval_data = np.fromstring(data_str[4*2+self.HANDS*4*2:], dtype=np.uint32)
        if n_plr == -1:
            return board_data, hand_data, handval_data
        
        hands = np.zeros((n_plr), dtype=np.int32)
        hands_retval = np.zeros((n_plr, 4), dtype=np.int16)
        hands.fill(-1)
        for x in xrange(n_plr):
            rnd = random.randint(0,self.HANDS-1)
            while rnd in hands or hand_data[rnd][3] == -1:
                rnd = random.randint(0,self.HANDS-1)
            hands[x] = rnd
            hands_retval[x] = hand_data[rnd]

        return board_data, hands_retval, handval_data[hands]

    # def load_traverse_jbs(self):
    #     filename_off = self.gamedir + "jbs_for_traverse_bs_" + str(self.i.n_btypes[1]) + "_" + str(self.i.n_btypes[2]) + "_" + str(self.i.n_btypes[3]) + "_hs_" + str(self.i.n_types[0]) + "_" + str(self.i.n_types[1]) + "_" + str(self.i.n_types[2]) + "_" + str(self.i.n_types[3]) + ".offset"
    #     filename_data = self.gamedir + "jbs_for_traverse_bs_" + str(self.i.n_btypes[1]) + "_" + str(self.i.n_btypes[2]) + "_" + str(self.i.n_btypes[3]) + "_hs_" + str(self.i.n_types[0]) + "_" + str(self.i.n_types[1]) + "_" + str(self.i.n_types[2]) + "_" + str(self.i.n_types[3]) + ".data"

    #     self.traverse_jbs_off = np.fromfile(filename_off, dtype=np.int64).reshape(-1,2)
    #     self.traverse_jbs_data_file = file(filename_data, "r+b")
    #     self.traverse_jbs_data_mmap = mmap.mmap(self.traverse_jbs_data_file.fileno(), 0)
        
    # def get_random_traverse_jbs(self):
    #     if self.traverse_jbs_off == None:
    #         self.load_traverse_jbs()
    #     #pdb.set_trace()
    #     jbs_i = random.randint(0, len(self.traverse_jbs_off)-1)
    #     #data_str = self.jbs_data_mmap[self.jbs_off[jbs_i][0]:self.jbs_off[jbs_i].sum()]
    #     data_str = zlib.decompress(self.traverse_jbs_data_mmap[self.traverse_jbs_off[jbs_i][0]:self.traverse_jbs_off[jbs_i].sum()])
    #     board_data = np.fromstring(data_str[:4*2], dtype=np.int16)
    #     hand_data =  np.fromstring(data_str[4*2:4*2+self.HANDS*4*2], dtype=np.int16).reshape(-1,4)
    #     handval_data = np.fromstring(data_str[4*2+self.HANDS*4*2:], dtype=dt.hand_hv_np)
    #     #print handval_data
    #     #print len(handval_data)
    #     #print handval_data.itemsize
    #     #sys.exit()
    #     return board_data, hand_data, handval_data
        
    def load_traverse_jbs(self):
        self.jbs_filename = "jbs_data/jbs"
        filename_off = self.gamedir + self.jbs_filename + ".offset"
        filename_data = self.gamedir + self.jbs_filename + ".data"
#        filename_off = self.gamedir + "jbs_for_traverse_bs_" + str(self.i.n_btypes[1]) + "_" + str(self.i.n_btypes[2]) + "_" + str(self.i.n_btypes[3]) + "_hs_" + str(self.i.n_types[0]) + "_" + str(self.i.n_types[1]) + "_" + str(self.i.n_types[2]) + "_" + str(self.i.n_types[3]) + ".offset"
#        filename_data = self.gamedir + "jbs_for_traverse_bs_" + str(self.i.n_btypes[1]) + "_" + str(self.i.n_btypes[2]) + "_" + str(self.i.n_btypes[3]) + "_hs_" + str(self.i.n_types[0]) + "_" + str(self.i.n_types[1]) + "_" + str(self.i.n_types[2]) + "_" + str(self.i.n_types[3]) + ".data"

        self.traverse_jbs_off = np.fromfile(filename_off, dtype=np.int64).reshape(-1,2)
        self.traverse_jbs_data_file = file(filename_data, "r+b")
        #self.traverse_jbs_data_mmap = mmap.mmap(self.traverse_jbs_data_file.fileno(), 0)
    
    def get_random_traverse_jbs(self, samples = defs.SAMPLES, n_items = 1):
        if self.traverse_jbs_off == None:
            self.load_traverse_jbs()
        retval = []
        #pdb.set_trace()
        jbs_i = random.randint(0, len(self.traverse_jbs_off)-n_items)
        data_start = self.traverse_jbs_off[jbs_i][0]
        data_stop = self.traverse_jbs_off[jbs_i+n_items-1].sum() #viimeisen mukaan otettavan aloitus kohta + pituus
        #data_len = self.traverse_jbs_off[jbs_i:jbs_i+n_items][:,1].sum()
        self.traverse_jbs_data_file.seek(data_start)
        data_str_comp = self.traverse_jbs_data_file.read(data_stop-data_start)
        #data_str_comp = self.traverse_jbs_data_mmap[data_start:data_stop]
        data_i = 0
        for offset in self.traverse_jbs_off[jbs_i:jbs_i+n_items]:
            data_str = zlib.decompress(data_str_comp[data_i:data_i+offset[1]])
            data_i += offset[1]
                                    
            #data_str_comp = self.traverse_jbs_data_mmap[self.traverse_jbs_off[jbs_i][0]:self.traverse_jbs_off[jbs_i].sum()]
            #data_str = zlib.decompress(data_str_comp)
            #data_str = zlib.decompress(self.traverse_jbs_data_mmap[self.traverse_jbs_off[jbs_i][0]:self.traverse_jbs_off[jbs_i].sum()])
            idx = np.arange(defs.DATA_SAMPLES)
            if (samples < defs.DATA_SAMPLES):
                np.random.shuffle(idx)
                idx = idx[:samples]
                idx.sort()
            board_data = np.fromstring(data_str[:4*2], dtype=np.int16)
            #hand_data =  np.fromstring(data_str[4*2:4*2+self.SAMPLES*4*2], dtype=np.int16).reshape(-1,4)
            hand_data =  np.fromstring(data_str[4*2:4*2+defs.DATA_SAMPLES*4*2], dtype=np.int16).reshape(4,defs.DATA_SAMPLES)[:,idx]
            handval_data = np.fromstring(data_str[4*2+defs.DATA_SAMPLES*4*2:], dtype=dt.hand_hv2_np)
            if (samples < defs.DATA_SAMPLES):
                hvd_sort_idx = np.argsort(handval_data["sample_i"]) #hv data sample_i mukaan sortattuna
                hvs_sort_idx = hvd_sort_idx[idx] # siita erotellaa sampleen kuuluvat itemit
                                             
                handval_data["sample_i"][hvs_sort_idx] = np.arange(samples, dtype=np.int16) #korjataan sample_i vastaamaan uutta todellisuutta

                hvs_sort_idx.sort() # sortataan indexit uudestaan vanhaan jarjestykseen
                handval_data = handval_data[hvs_sort_idx] #luodaan uusi handval_data
            retval.append((board_data, hand_data, handval_data))
        #print handval_data
        #print len(handval_data)
        #print handval_data.itemsize
        #sys.exit()
        return retval
        
    def load_jbs_for_simu(self):
        filename_off = self.gamedir + "jbs_for_simu_bs_" + str(self.i.n_btypes[1]) + "_" + str(self.i.n_btypes[2]) + "_" + str(self.i.n_btypes[3]) + "_hs_" + str(self.i.n_types[0]) + "_" + str(self.i.n_types[1]) + "_" + str(self.i.n_types[2]) + "_" + str(self.i.n_types[3]) + ".offset"
        filename_data = self.gamedir + "jbs_for_simu_bs_" + str(self.i.n_btypes[1]) + "_" + str(self.i.n_btypes[2]) + "_" + str(self.i.n_btypes[3]) + "_hs_" + str(self.i.n_types[0]) + "_" + str(self.i.n_types[1]) + "_" + str(self.i.n_types[2]) + "_" + str(self.i.n_types[3]) + ".data"

        self.jbs_for_simu_off = np.fromfile(filename_off, dtype=np.int64).reshape(-1,2)
        self.jbs_for_simu_data_file = file(filename_data, "r+b")
        self.jbs_for_simu_data_mmap = mmap.mmap(self.jbs_for_simu_data_file.fileno(), 0)
        
    def get_random_jbs_for_simu(self):
        jbs_i = random.randint(0, len(self.jbs_for_simu_off)-1)
        #data_str = self.jbs_data_mmap[self.jbs_off[jbs_i][0]:self.jbs_off[jbs_i].sum()]
        data_str = zlib.decompress(self.jbs_for_simu_data_mmap[self.jbs_for_simu_off[jbs_i][0]:self.jbs_for_simu_off[jbs_i].sum()])
        return np.fromstring(data_str, dtype=dt.hand_hv_flop)
        

    def load_all_for_calc(self):
        print "loading path odds table"
        self.load_jbs()
        self.load_jbs_for_simu()
        self.load_hand_slots(0)
        # self.load_path_odds()
        # self.load_gs_switch_path()
        # self.load_hand_wtl_odds()
        # for gs in xrange(4):
        #     if gs > 0:
        #         print "loading hand switch table", gs
        #         self.load_gs_hand_switch_tables(gs)
        #         print "loading board odds table", gs
        #         self.load_gs_board_odds_tables(gs)
        #         print "loading board switch table", gs
        #         self.load_gs_board_switch_tables(gs)
        #     print "loading hand odds table", gs
        #     self.load_gs_hand_odds_tables(gs)
           

    def get_diffs_for_new_type(self, new_types, new_type, diffs, tmp_diff, to_remove):
        n_new_types = len(new_types)        
        smallest = 1000000000
        i = 0
        while i < n_new_types:
            if i == to_remove:
                tmp_diff[i] = 1000000000
            else:
                tmp_diff[i] = diffs[new_type, new_types[i]]
                #new_diffs[i, new_type] = new_diffs[new_type,i]
                if tmp_diff[i] < smallest:
                    smallest = tmp_diff[i]
            i+=1
        return smallest
    

    def calc_river_diffs(self):
        n_types = self.i.n_types[3]
        wl_slots = self.i.rtype[3].wl_slots
        tie_slots = self.i.rtype[3].tie_slots

        diffs = np.zeros((self.i.n_types[3], self.i.n_types[3]), dtype=np.float64)
        
        i = 0
        while i < n_types:
            coord_1 = [(i%wl_slots)+0.5, (i/wl_slots)+0.5]
            j = 0
            while j < n_types:
                coord_2 = [(j%wl_slots)+0.5, (j/wl_slots)+0.5]
                if i == j:
                    diffs[i,j] = 1000000000
                else:
                    diffs[i,j] = math.sqrt(float(((coord_1[0] - coord_2[0])**2 + (coord_1[1] - coord_2[1])**2)))
                j+=1
            i+=1
        return diffs

    def calc_diffs_order_from_diffs(self, diffs):
        i = 0 
        diffs_order = np.zeros(diffs.shape, dtype = np.int16)
        tot_max_diff = 0
        n_types = len(diffs)
        while i < n_types:
            diffs_order[i] = np.argsort(diffs[i])
            tot_max_diff += diffs[i][diffs_order[i][-2]]
            #print i, self.diffs[gs][i]
            i+=1
        return diffs_order


    def calc_diffs_order_from_diffs_gs(self, gs):
        self.diffs_order[gs] = self.calc_diffs_order_from_diffs(self.diffs[gs])
        self.i.diffs_order[gs] = self.diffs_order[gs].ctypes.data_as(POINTER(c_short))

    def precalc_slots_preflop(self):
        if self.preflop_alltypes == None:
            self.gen_all_preflop_types()
        slots = np.zeros((self.HANDS), dtype = np.int16)
        for i,t in enumerate(self.preflop_alltypes):
            slots[i] = self.get_slot(0, t)
            print slots[i]
        self.slots[0] = slots
        self.i.slots[0] = slots.ctypes.data_as(POINTER(c_short))
        tmpt = {}
        for i, x in enumerate(slots):
            try:
                tmpt[x].append(ct.cards_to_str(ct.itoc2[i]))
            except:
                tmpt[x] = []
                tmpt[x].append(ct.cards_to_str(ct.itoc2[i]))
#            print ct.cards_to_str(ct.itoc2[i]), x
        for x in tmpt.items():
            print x

    def precalc_slots_flop(self):
        slots = np.zeros((self.FLOPS, self.HANDS), dtype = np.int16)

        for i, slot in enumerate(slots):
            print "start1", i
            types = self.gen_types_for_flop(i)
            print "start2", i
            self.get_slots_for_all(1, slots[i], types)
            print types[0]
            print slots[i]
        self.slots[1] = slots
        self.i.slots[1] = slots.ctypes.data_as(POINTER(c_short))

    def precalc_slots_turn(self):
        test_slot = 1500
        slots = np.zeros((self.TURNS, self.HANDS), dtype = np.int16)
        i = 0
        i1 = 51
        while i1 >= 0:
            i2 = i1-1
            while i2 >= 0:
                i3 = i2-1
                while i3 >= 0:
                    flop = self.ctoi3[i1][i2][i3]
                    i4 = i3-1
                    while i4 >= 0:
                        types = self.gen_types_for_turn(flop, i4)
                        self.get_slots_for_all(2,slots[i], types)
                        print i1, i2, i3, i4, i
                        if not i%100:
                            print slots[i]
                            print i, self.ctoi4[i1][i2][i3][i4]
                    #print turn_slots[i]
                            for y in np.where(slots[i] == test_slot)[0]:
                                print flop, i4,y
                                print ct.cards_to_str(ct.itoc3[flop]+[i4]), ct.cards_to_str(ct.itoc2[y])
                                break
                    #print flop, ct.ctoi3[i1][i2][i3], zp.ctoi3[i1][i2][i3], i1, i2, i3, i4
                        i += 1
                        i4 -= 1
                    i3 -= 1
                i2 -= 1
            i1 -= 1

        self.slots[2] = slots
        self.i.slots[2] = slots.ctypes.data_as(POINTER(c_short))

    def precalc_slots_river(self):
        #test_slot = 1500
        #slots = np.zeros((self.TURNS, self.HANDS), dtype = np.int16)
        #f = file("river_slots", "wb")
        f = file("river_slots", "r+b")
        i = 0
        i1 = 51
        while i1 >= 0:
            i2 = i1-1
            while i2 >= 0:
                i3 = i2-1
                while i3 >= 0:
                    flop = self.ctoi3[i1][i2][i3]
                    i4 = i3-1
                    while i4 >= 0:
                        i5 = i4-1
                        while i5 >= 0:
                            if i > 2587000:
                                slots = self.get_all_hand_slots_river([i1,i2,i3, i4, i5])
                                f.write(slots.tostring())
                            else:
                                f.seek(1326*2, 1)
                            i += 1
                            if not i%1000 or 1:
                                print i
                            i5 -= 1
                        i4 -= 1
                    i3 -= 1
                i2 -= 1
            i1 -= 1

        f.close()

    def precalc_and_save_slots(self, gs):
        if gs == 0:
            self.precalc_slots_preflop()
        elif gs == 1:
            self.precalc_slots_flop()
        elif gs == 2:
            self.precalc_slots_turn()
        elif gs == 3:
            self.precalc_slots_river()
            return True
        else:
            return False
    
        self.save_hand_slots(gs)
        return True

    def print_hands_from_type(self, gs, type_i):
        if gs == 0:
            n_slots = self.HANDS
        elif gs == 1:
            n_slots = self.FLOPS*self.HANDS
        elif gs == 2:
            n_slots = self.TURNS*self.HANDS
        else:
            return False

        index_list = np.where(self.slots[gs] == type_i)
        #print index_list
        #print "len", len(index_list[0])
        
        #for random_slot in index_list[0][:10]:
        if gs == 0:
            for x in index_list[0]:
                print ct.cards_to_str(ct.itoc2[x]),
            
        elif gs == 1:
            for x in xrange(20):
                s = random.randint(0, len(index_list[0]))
            
                flop = index_list[0][s]
                hand = index_list[1][s]
            #flop = random_slot/self.HANDS
            #hand = random_slot%self.HANDS
                print ct.cards_to_str(ct.itoc3[flop]), ct.cards_to_str(ct.itoc2[hand]),
            n_slots = self.FLOPS*self.HANDS
        elif gs == 2:
            turn = random_slot/self.HANDS
            hand = random_slot%self.HANDS
            cards_c = self.itoc4[turn]
            cards = [cards_c.c1,cards_c.c2,cards_c.c3,cards_c.c4]
            print ct.cards_to_str(cards), ct.cards_to_str(ct.itoc2[hand]),
        else:
            return False
        print ""
            
    def print_random_sample_of_a_type_old(self, gs, type_i):
        if gs == 0:
            n_slots = self.HANDS
        elif gs == 1:
            n_slots = self.FLOPS*self.HANDS
        elif gs == 2:
            n_slots = self.TURNS*self.HANDS
        else:
            return False

        random_slot = random.randint(0,n_slots-1)
        while self.slots[gs][random_slot] != type_i:
            random_slot = random.randint(0,n_slots-1)

        if gs == 0:
            print ct.cards_to_str(ct.itoc2[random_slot])
        elif gs == 1:
            flop = random_slot/self.HANDS
            hand = random_slot%self.HANDS
            print ct.cards_to_str(ct.itoc3[flop]), ct.cards_to_str(ct.itoc2[hand])
            n_slots = self.FLOPS*self.HANDS
        elif gs == 2:
            turn = random_slot/self.HANDS
            hand = random_slot%self.HANDS
            cards_c = self.itoc4[turn]
            cards = [cards_c.c1,cards_c.c2,cards_c.c3,cards_c.c4]
            print ct.cards_to_str(cards), ct.cards_to_str(ct.itoc2[hand])
        else:
            return False


    def reduce_types(self, diffs, n_types, reduce_to, types = None):
        if types == None:
            types = range(n_types)
        def get_random_type():
            i = random.randint(0, n_types - 1)
            return types[i]
            #return self.types[gs][random_type]

        def distance_function(t1,t2):
#            if diffs[t1,t2] == 1000000000:
            if t1 == t2:
                return 0.0
            return diffs[t1,t2]
            
        new_diffs, new_diffs_order, new_types = self.get_group_with_biggest_diffs(reduce_to, [], get_random_type, distance_function, True, type_unit=np.int16, diffs_unit = np.float32)
        print "Build pyramid"
        new_diffs, new_types = self.build_pyramid(new_diffs, new_types)
        print "recalc diffs order"
        new_diffs_order = self.calc_diffs_order_from_diffs(new_diffs)
        print "done"
        return new_diffs, new_diffs_order, new_types

    def build_pyramid(self, diffs, types):
        print "Building pyramid"
        new_order = self.build_pyramid2(range(len(diffs)), diffs)
        print "Calc new diffs & reorder"
        new_diffs = copy.deepcopy(diffs)
        new_types = copy.deepcopy(types)
        i1 = 0
        while i1 < len(new_order):
            i2 = i1
            while i2 < len(new_order):
                ni1 = new_order[i1]
                ni2 = new_order[i2]
                new_diffs[i1, i2] = diffs[ni1,ni2]
                new_diffs[i2, i1] = diffs[ni2,ni1]
                i2+=1
            i1+=1
        i1 = 0
        while i1 < len(new_order):
            new_types[i1] = types[new_order[i1]]
            #new_types[new_order[i1]] = types[i1]
            i1+=1
#        print types.sum(), new_types.sum()
#        print diffs.sum(), new_diffs.sum()
        return new_diffs, new_types

    def build_pyramid2(self, type_list, diffs, final_i1 = -1, final_i2 = -1):
        def sort_key_function(i):
#            global largest_diff, final_i1, final_i2, diffs
            d1 = diffs[i, final_i1]
            if d1 == 1000000000:
                d1 = 0
            d2 = diffs[i, final_i2]
            if d2 == 1000000000:
                d2 = 0
            return d1-d2
            return math.sqrt(d1) + math.sqrt(largest_diff - d2) 
            
 #       n_types = len(diffs)
 #       diffs_order = self.calc_diffs_order_from_diffs(diffs)
        #print len(type_list)
        if len(type_list) <= 2:
            return type_list
        largest_diff = 0
        largest_diff_i = -1
        if final_i1 == -1 and final_i2 == -1:
            i1 = 0
            i2 = 0
            while i1 < len(type_list):
                i2 = i1+1
                while i2 < len(type_list):
                    if diffs[type_list[i1], type_list[i2]] >= largest_diff:
                        largest_diff = diffs[type_list[i1], type_list[i2]]
                        final_i1 = type_list[i1]
                        final_i2 = type_list[i2]
                        #print largest_diff, final_i1, final_i2
                    i2+=1
                i1+=1
        elif final_i2 == -1:
            i1 = 0
            while i1 < len(type_list):
                if type_list[i1] != final_i1:
                    if diffs[type_list[i1], final_i1] >= largest_diff:
                        largest_diff = diffs[type_list[i1], final_i1]
                        final_i2 = type_list[i1]
                        #print largest_diff, final_i1, final_i2
                i1+=1

        elif final_i1 == -1:
            i1 = 0
            while i1 < len(type_list):
                if type_list[i1] != final_i2:
                    if diffs[type_list[i1], final_i2] >= largest_diff:
                        largest_diff = diffs[type_list[i1], final_i2]
                        final_i1 = type_list[i1]
                        #print largest_diff, final_i1, final_i2
                i1+=1

#        print largest_diff, final_i1, final_i2
        type_list.sort(key = sort_key_function)
#        print type_list
#        for i, x in enumerate(type_list):
#            print diffs[x,final_i1], diffs[x, final_i2], diffs[x,final_i1] - diffs[x, final_i2], x, i
        #sys.exit(0)
#        print "START", type_list
        types1 = self.build_pyramid2(type_list[:len(type_list)/2], diffs, final_i1, -1)
        types2 = self.build_pyramid2(type_list[len(type_list)/2:], diffs, -1, final_i2)
#        print "STOP", type_list
#        print "TYPES1", types1
#        print "TYPES2", types2
        return types1+types2
        # for i,x in enumerate(diffs_order):
#             if diffs[i,x[-2]] >= largest_diff:
#                 largest_diff = diffs[i,x[-2]]
#                 largest_diff_i = i
#                 i1 = i
#                 i2 = x[-2]
#                 print largest_diff
#                 print largest_diff_i, x[-2]

#         print i1, i2



    def get_hand_types(self, gs):
        if gs == 0:
            def gen_random_type():
                if self.preflop_alltypes == None:
                    self.gen_all_preflop_types()
                return self.preflop_alltypes[random.randint(0,self.HANDS-1)]
        
        elif gs == 1:
            def gen_random_type():
                #r = self.i.rtype[gs]
                path = self.gamedir + "flop_types_raw/"
                c = random.sample(range(52), 5)
                flop_n = self.ctoi3[c[0]][c[1]][c[2]]
                hand_n = self.ctoi2[c[3]][c[4]]
                flop = np.memmap(path+str(flop_n)+".ftype", np.float32, mode = "r", shape = (self.HANDS, self.i.n_rtypes[1]))
                retval = flop[hand_n]
                flop.close()
                return retval
        elif gs == 2:
            def gen_random_type():
                return self.gen_random_turn_type()
        elif gs == 3:
            def gen_random_type():    
                return self.gen_random_river_type()
                pass

#        def distance_function(t1,t2):
#            return ((t1-t2)**2).sum()
        
        def distance_function(types,new_type,diffs):
            n_types = self.i.n_types[gs]
            n_slots_per_type = self.i.n_rtypes[gs]
            self.calc_diffs_for_one_type(types, new_type, diffs, n_types, n_slots_per_type)
#        def distance_function(t1,t2):
#            return self.get_difference_pow2(t1,t2)

        diffs, diffs_order, types = self.get_group_with_biggest_diffs(self.i.n_types[gs], (self.i.n_rtypes[gs],), gen_random_type, distance_function, False, type_unit = np.float32)
        print types
        diffs, types = self.build_pyramid(diffs,types)
        print types
        self.diffs[gs] = diffs
        self.types[gs] = types
        self.i.diffs[gs] = self.diffs[gs].ctypes.data_as(POINTER(c_float))
        self.i.types[gs] = self.types[gs].ctypes.data_as(POINTER(c_float))
        self.calc_diffs_order_from_diffs_gs(gs)

    def get_board_types(self, gs):
        def random_board():
            b = random.sample(range(52), 5)
            flop_i = self.ctoi3[b[0]][b[1]][b[2]]
            turn_i = b[3]
            river_i = b[4]
            t1 = self.gen_board_type(gs, flop_i, turn_i, river_i)
            return t1
#        def distance_function(t1,t2):
#            return ((t1-t2)**2).sum()
        def distance_function(types,new_type,diffs):
            n_types = len(types)
            n_slots_per_type = new_type.size
#            for x in types:
#                print x.sum(),
#            print new_type.sum()
#            print "N_TYPES", n_types, "N_SLOTS_PER_TYPE", n_slots_per_type
            self.calc_diffs_for_one_type(types, new_type, diffs, n_types, n_slots_per_type)

        diffs, diffs_order, types = self.get_group_with_biggest_diffs(self.i.n_btypes[gs], (self.i.n_htfb[gs-1], self.i.n_htfb[gs]), random_board, distance_function, False)
        self.b_diffs[gs], self.b_types[gs] = self.build_pyramid(diffs,types)
        self.i.b_diffs[gs] =  self.b_diffs[gs].ctypes.data_as(POINTER(c_float))
        self.i.b_types[gs] =  self.b_types[gs].ctypes.data_as(POINTER(c_float))
        self.save_board_types(gs)
        self.save_board_diffs(gs)

    def ggwbd_recalc_diffs_order(self, diffs_order, diffs, new_type = None):
        #vittu mika hax taa funktio jo
        i = 0
        count = 0
        if not new_type:
            while i < len(diffs):

                diffs_order[i] = np.argsort(diffs[i])
                i+=1
        else:

            self.find_two_smallest(diffs, diffs_order, new_type)
            
#             miss_count = 0
#             for x in diffs_order[new_type]:
#                 if x == new_type:
#                     continue
#                 print "miss_start", miss_count
#                 if diffs_order[x,0] == new_type or diffs_order[x,1] == new_type:
#                     diffs_order[x] = np.argsort(diffs[x])
#                     print new_type, x, diffs_order[x,0], diffs_order[x,1]
#                     diffs_order[diffs_order[x,0]] = np.argsort(diffs[diffs_order[x,0]])
#                     diffs_order[diffs_order[x,1]] = np.argsort(diffs[diffs_order[x,1]])
            
#                 else:
#                     miss_count +=1
#                 print "miss_stop", miss_count
#                 print x, diffs[x,diffs_order[x, 0]], diffs[x,diffs_order[x,1]]
#                 assert diffs[x,diffs_order[x, 0]] <= diffs[x,diffs_order[x,1]]
#                 if miss_count > 50:
#                     break
#             diffs_order[new_type] = np.argsort(diffs[new_type])
#             print "new type order", new_type, diffs_order[new_type,0], diffs_order[new_type,1]
#             diffs_order[diffs_order[new_type,0]] = np.argsort(diffs[diffs_order[new_type,0]])
#             diffs_order[diffs_order[new_type,1]] = np.argsort(diffs[diffs_order[new_type,1]])
#             miss_count = 0
#             for i in diffs_order[new_type][2:]:
#                 assert diffs[i, new_type] == diffs[new_type,i]
#                 print diffs[i, new_type], diffs[i, diffs_order[i,1]]
#                 if diffs[i, new_type] <= diffs[i, diffs_order[i,1]]:
#                     diffs_order[i] = np.argsort(diffs[i])
#                 else:
#                     miss_count += 1
#                 print new_type, i, diffs_order[i,0], diffs_order[i,1], diffs[i,diffs_order[i, 0]], diffs[i,diffs_order[i,1]]
#                 assert diffs[i,diffs_order[i, 0]] <= diffs[i,diffs_order[i,1]]
#                 if miss_count > 50:
#                     break

        
    def get_group_with_biggest_diffs(self, group_size, type_shape, random_gen_function, distance_function, simple_distance_function=True, type_unit = np.int16, diffs_unit = np.float32):

        diffs = np.zeros((group_size, group_size), dtype=diffs_unit)
        #diffs_order = np.zeros((group_size, group_size), dtype=np.float64)
        diffs_order = np.zeros((group_size, group_size), dtype=np.int16)
        types_shape = [group_size] + list(type_shape)
        types_shape = tuple(types_shape)
        #types_shape = (group_size, type_shape[0], type_shape[1])
        print types_shape
        types = np.zeros(types_shape, dtype=type_unit)
        
        
        i = 0
        while i < group_size:
            types[i] = random_gen_function()
            i+=1
            print "generating random", i
            #pdb.set_trace()
        i = 0
        while i < group_size:
            if simple_distance_function:
                j = i+1
                while j < group_size:
                    diff = distance_function(types[i], types[j])
                    diffs[i,j] = diff
                    diffs[j,i] = diff
                    j+=1
            else:
                distance_function(types, types[i], diffs[i])
            #diffs[i,i] = 1000000000
            #diffs[i,i] = 0
            diffs[i,i] = 1000000000
            print "calculating diffs, ", i, ", done"
            i+=1
        print "1"
        self.ggwbd_recalc_diffs_order( diffs_order, diffs)
        print "2"
        lc = 0
        try: 
            while True:
                small_idx_list = []
                small = 1000000000
                i = 0
                
                
                while i < group_size:
                    d = diffs[i, diffs_order[i,0]]
                    
                    if d < small:
                        small = d
                        small_idx_list = [i]
                    elif d == small:
                        small_idx_list.append(i)
                        
                    i+=1
                #small = diffs[np.arange(2**14),diffs_order[:,0]].min()
                #small_idx_list = list(np.where(diffs[np.arange(2**14),diffs_order[:,0]] == small))
                #pdb.set_trace()
                if len(small_idx_list) == 1:
                    sm = small_idx_list[0]
                    close1 = diffs_order[sm,0]
                    close2 = diffs_order[sm,1]
                    print sm, close1, close2
                    print close1,  diffs_order[close1,0],diffs_order[close1,1]
                    print close2,  diffs_order[close2,0],diffs_order[close2,1]

                    print diffs[close1, sm], diffs[sm, close1]
                    print diffs[close1, diffs_order[close1,0]]
                    print diffs[close2, sm], diffs[sm, close2]
                    print diffs[close2, diffs_order[close2,0]]
                    for i,x in enumerate(diffs):
                        print i,x
                    sys.exit(0)
#print diffs_order[diffs_order[small_idx_list,0],0]
                    #print diffs_order[diffs_order[small_idx_list,0],1]
                    
                print "smallest", small, small_idx_list, "loopcount:", lc
                lc += 1
                small_idx = -1
                small_2 = 1000000000
                for idx in small_idx_list:
                    if diffs[idx, diffs_order[idx,1]] < small_2:
                        small_2 = diffs[idx, diffs_order[idx,1]]
                        small_idx = idx
                loop_count = 0
                loop_time = time.time()
                replaced_type_i = None
                while True:
                    loop_count += 1
                    new_type = random_gen_function()
                    one_type_diffs = np.zeros((group_size), dtype=np.float32)
                    new_small = 1000000000
                    new_small_idx = -1
                    new_small2 = 1000000000
                    new_small_idx2 = -1
                    if simple_distance_function:
                        for i, type in enumerate(types):
                            one_type_diffs[i] = distance_function(type, new_type)
                    else:
                        distance_function(types, new_type, one_type_diffs)

                    # t1 = time.time()
                    # new_small_idx = one_type_diffs.argmin()
                    # new_small = one_type_diffs[new_small_idx]
                    # new_small_idx2_1 = one_type_diffs[:new_small_idx].argmin()
                    # if new_small_idx < len(one_type_diffs)-1:
                    #     new_small_idx2_2 = one_type_diffs[new_small_idx:].argmin()
                    # else:
                    #     new_small_idx2_2 = new_small_idx2_1

                    # if one_type_diffs[new_small_idx2_1] <= one_type_diffs[new_small_idx2_2]:
                    #     new_small_idx2 = new_small_idx2_1
                    # else:
                    #     new_small_idx2 = new_small_idx2_2
                    # new_small2 = one_type_diffs[new_small_idx2]
                    # t2 = time.time()
                    # print new_small, new_small2, new_small_idx, new_small_idx2

                    # new_small = 1000000000
                    # new_small_idx = -1
                    # new_small2 = 1000000000
                    # new_small_idx2 = -1

                    # t3 = time.time()
                    for i, diff in enumerate(one_type_diffs):
                        if diff < new_small and i != small_idx:
                        #new_small2 = new_small
                        #new_small_idx2 = new_small_idx
                            if new_small < new_small2:
                                new_small2 = new_small
                                new_small_idx2 = new_small_idx
                            new_small_idx = i
                            new_small = diff
                        elif  diff < new_small2:
                            new_small_idx2 = i
                            new_small2 = diff
                    # t4 = time.time()
                    # print new_small, new_small2, new_small_idx, new_small_idx2
                    # print t2-t1, t4-t3
                    #print new_type    
                    #print new_small, small
                    #pdb.set_trace()
                    if new_small > small:
                        replaced_type_i = small_idx
                        types[small_idx] = new_type
                        diffs[small_idx] = one_type_diffs
                        diffs[:,small_idx] = one_type_diffs
                        diffs[small_idx, small_idx] = 1000000000

                        print "found bigger than smallest", new_small, small, loop_count, (time.time() - loop_time)/float(loop_count), replaced_type_i
                        loop_count = 0
                        loop_time = time.time()
                    elif diffs[new_small_idx, diffs_order[new_small_idx,0]] < new_small2:
                        replaced_type_i = new_small_idx
                        print "found bigger than closest", diffs[new_small_idx, diffs_order[new_small_idx,0]], new_small2, new_small, loop_count, (time.time() - loop_time)/float(loop_count), replaced_type_i
                        types[replaced_type_i] = new_type
                        diffs[replaced_type_i] = one_type_diffs
                        diffs[:,replaced_type_i] = one_type_diffs
                        diffs[replaced_type_i, replaced_type_i] = 1000000000
                        
                        loop_count = 0
                        loop_time = time.time()

                    else:
                        continue
                    self.ggwbd_recalc_diffs_order(diffs_order, diffs, replaced_type_i)
                    break
        except KeyboardInterrupt:
            pass
        return diffs, diffs_order, types

    def precalc_board_slots(self, gs):
        b_types = self.i.b_types[gs]
        n_btypes = self.i.n_btypes[gs]
        
        
        self.b_slots[gs] = np.zeros((self.board_type_count[gs]), dtype = np.int16)
        assert len(self.b_types[gs]) == n_btypes
        assert gs > 0 and gs < 4

        # if gs == 1:
#             self.b_slots[gs] = np.zeros((self.FLOPS), dtype = np.int16)
#         elif gs == 2:
#             self.b_slots[gs] = np.zeros((self.FLOPS*52), dtype = np.int16)
#         elif gs == 3:
#             self.b_slots[gs] = np.zeros((self.TURNS*52), dtype = np.int16)
        i = 0
        start_time = time.time()
        i1 = 51
        while i1 >= 0:
            i2 = i1-1
            while i2 >= 0:
                i3 = i2-1
                while i3 >= 0:
                    flop_i = self.ctoi3[i1][i2][i3]
                    print "new_flop", i, (time.time() - start_time)/(i+1)
                    if gs >= 2:
                        if gs == 2:
                            i4 = 51
                        else:
                            i4 = i3-1     
                        while i4 >= 0:
                            #print i4
                            if gs == 3: #GAMESTATE 3
                                i5 = 51
                                while i5 >= 0:
                                    if i5 != i1 and i5 != i2 and i5 != i3 and i5 != i4:
                                        type = self.gen_board_type(gs, flop_i, i4, i5)
                                        self.b_slots[gs][i] = self.get_slot_new(self.i.b_types[gs], type.ctypes.data, len(self.b_types[gs]), type.size)
                                        if self.b_slots[gs][i] == 0:
                                            print i, "0", ct.cards_to_str(ct.itoc3[flop_i]), ct.itos[i4],ct.itos[i5]
                                        elif self.b_slots[gs][i] == 1023:
                                            print i, "1023", ct.cards_to_str(ct.itoc3[flop_i]), ct.itos[i4],ct.itos[i5]
                                        elif self.b_slots[gs][i] == 300:
                                            print i, "300", ct.cards_to_str(ct.itoc3[flop_i]), ct.itos[i4],ct.itos[i5]
                                        elif self.b_slots[gs][i] == 700:
                                            print i, "700", ct.cards_to_str(ct.itoc3[flop_i]), ct.itos[i4],ct.itos[i5]
                                    else:
                                        self.b_slots[gs][i] = -1
                                    i+=1
                                    i5-=1
                            else: #GAMESTATE 2
                                if i4 != i1 and i4 != i2 and i4 != i3:
                                    type = self.gen_board_type(gs, flop_i, i4)
                                    self.b_slots[gs][i] = self.get_slot_new(self.i.b_types[gs], type.ctypes.data, len(self.b_types[gs]), type.size)
                                    if self.b_slots[gs][i] == 0:
                                        print i, "0", ct.cards_to_str(ct.itoc3[flop_i]), ct.itos[i4]
                                    elif self.b_slots[gs][i] == 1023:
                                        print i, "1023", ct.cards_to_str(ct.itoc3[flop_i]), ct.itos[i4]
                                    elif self.b_slots[gs][i] == 300:
                                        print i, "300", ct.cards_to_str(ct.itoc3[flop_i]), ct.itos[i4]
                                    elif self.b_slots[gs][i] == 700:
                                        print i, "700", ct.cards_to_str(ct.itoc3[flop_i]), ct.itos[i4]
                                            
                                    
                                else:
                                    self.b_slots[gs][i] = -1
                                i+=1
                            i4-=1
                    else: #GAMESTATE 1
                        #print i
                        type = self.gen_board_type(gs, flop_i)
                        self.b_slots[gs][i] = self.get_slot_new(self.i.b_types[gs], type.ctypes.data, len(self.b_types[gs]), type.size)
                        if self.b_slots[gs][i] == 0:
                            print i, "0", ct.cards_to_str(ct.itoc3[flop_i])
                        if self.b_slots[gs][i] == 1023:
                            print i, "1023", ct.cards_to_str(ct.itoc3[flop_i])
                        
                        i+=1
                        
                    i3-=1
                i2-=1
            i1-=1
        self.save_board_slots(gs)


    def expand_odds_table_and_normalize(self, table, normalize_last_dimension = True, expand_until_n_dim_left = 0):
        def copy_and_expand_all(d, s):
            if len(d.shape) > 1 + expand_until_n_dim_left:
                for i in xrange(s.shape[0]):
                    copy_and_expand_all(d[d.shape[0]/2+i],s[i])
            else:
                d[d.shape[0]/2:] = s
            i = d.shape[0]/2-1
            while i > 0:
                d[i] = (d[i*2]+d[i*2+1])/2
                #d[i] = d[i*2:i*2+2].sum()
                i-=1
            #print d
            #print d[1].sum(), d[d.shape[0]/2:].sum()
            #x = 1
            #while x < d.shape[0]:
            #    print repr(d[x:x*2].sum()),
            #    x*=2
            #print
            #print abs(d[1].sum() - d[d.shape[0]/2:].sum())
            #assert abs(d[1].sum() - d[d.shape[0]/2:].sum()) < 0.001

        def normalize(t):
            if len(t.shape) > 1:
                for dim in t:
                    normalize(dim)
            else:
                i = t.shape[0]/2
                #print t
                #print t[1], t[i:].sum()
                #assert t[1] == t[i:].sum()
                if t[1] != 0:
                    while i > 0:
                        t[i:i*2] = t[i:i*2]/t[i:i*2].sum()
                        i/=2
                    #print t

        new_shape = list(table.shape)
        i = 0
        while i < len(table.shape)-expand_until_n_dim_left:
            new_shape[i] = new_shape[i]*2
            i+=1
        
        new_table = np.zeros(new_shape, dtype = np.float64)
        
        copy_and_expand_all(new_table, table)
        if normalize_last_dimension:
            normalize(new_table)

        return new_table
#         if len(new_shape) == 3:
#             for i1, dim1 in enumerate(table):
#                 for i2, dim2 in enumerate(dim1):
#                     tmp_table = new_table[i1+table.shape[0], i2+table.shape[1]]
#                     tmp_table[table.shape[2]:] = dim2
#                     i3 = table.shape[2]-1
#                     while i3 > 0:
#                         new_table[i1+table.shape[0], i2+table.shape[1]][i3]
#                         tmp_table[i3] = tmp_table[i3*2:i3*2+2].sum()
#                         i3-=1
            
                        
#             i1 = new_table.shape[0]/2-1
#             while i1 > 0:
#                 dim1 = new_table[i1]
#                 i2 = new_table.shape[1]/2-1
#                 while i2 > 0:
#                     dim2 = dim1[i2]
#                     i3 = new_table.shape[2]/2-1
#                     while i3 > 0:
#                         dim2[i3] = dim2[i3*2:i3*2+2].sum()
#                         i3-=1
#                     dim1[i2] = dim1[i2*2:i2*2+2].sum()
#                     i2-=1
#                 new_table[i1] = new_table[i1*2:i1*2+2].sum()
#                 i1-=1
                        

    def precalc_gs_switch_and_odds_tables(self):
        def board_iter():
            i1 = 51
            while i1 >= 0:
                i2 = i1-1
                while i2 >= 0:
                    i3 = i2-1
                    while i3 >= 0:
                        i4 = 51
                        while i4 >= 0:
                            i5 = 51
                            while i5 >= 0:
                                if len(set((i1,i2,i3,i4,i5))) == 5:
                                    yield [i1,i2,i3,i4,i5]
                                i5 -= 1
                            i4-=1
                        i3-=1
                    i2-=1
                i1-=1
        def random_board_iter():
            deck = range(52)
            while 1:
                flop_b = random.sample(deck, 3)
                for i1 in xrange(10):
                    turn_b = flop_b + [random.randint(0,51)]
                    while len(set(turn_b)) < 4:
                        turn_b = flop_b + [random.randint(0,51)]
                    for i2 in xrange(10):
                        river_b = turn_b + [random.randint(0,51)]
                        while len(set(river_b)) < 5:
                            river_b = turn_b + [random.randint(0,51)]
                        yield river_b
        def random_board():           
            return random.sample(range(52), 5)
        
        switch_table = self.gs_switch_hand
        odds_table = self.gs_odds_hand
        boards_switch_table = self.gs_switch_board
        boards_odds_table = self.gs_odds_board
        switch_table2 = [None]*4
        odds_table2 = [None]*4
        for x in range(1,4):
            switch_table[x] = np.zeros((self.i.n_btypes[x], self.i.n_types[x-1], self.i.n_types[x]), dtype=np.float64)
            #switch_table2[x] = np.zeros((self.i.n_btypes[x], self.i.n_types[x-1], self.i.n_types[x]), dtype=np.float64)
            boards_odds_table[x] = np.zeros((self.i.n_btypes[x]), dtype=np.float64)
            print switch_table[x].shape
        boards_switch_table[1] = np.zeros((self.i.n_btypes[1]), dtype=np.float64)
        boards_switch_table[2] = np.zeros((self.i.n_btypes[1], self.i.n_btypes[2]), dtype=np.float64)
        boards_switch_table[3] = np.zeros((self.i.n_btypes[1], self.i.n_btypes[2], self.i.n_btypes[3]), dtype=np.float64)
        for x in range(4):
            odds_table[x] = np.zeros((self.i.n_btypes[x], self.i.n_types[x]), dtype=np.float64)
            #odds_table2[x] = np.zeros((self.i.n_btypes[x], self.i.n_types[x]), dtype=np.float64)
            print odds_table[x].shape
        getting_slots_time = 0
        loops_time = 0
        tot_time = 0
        visited_boards = {}
        board_queue = Queue.Queue(2)
        hands_queue = Queue.Queue(2)
        
        try:
            i = 0
            #while True:
            #for board in [[i1,i2,i3,i4,i5] for i1 in xrange(51,-1,-1) for i2 in xrange(i1-1,-1,-1) for i3 in xrange(i2-1,-1,-1) for i4 in xrange(51,-1,-1) for i5 in xrange(51,-1,-1) if len(set([i1,i2,i3,i4,i5])) == 5]:
            #bgen = board_iter()
            bgen = random_board_iter()
            #board = random_board()
            board = bgen.next()
            bid = threading.Thread(target=self.get_board_slots_all_gs, args = (board, board_queue))
            hid = threading.Thread(target=self.get_all_hand_slots_all_gs, args = (board, hands_queue))
            bid.start()
            hid.start()
            for board in bgen:
                #print board
            #while True:
                #print i,
                time1 = time.time()
                #board = random.sample(range(52), 5)
                #print "getting board slots"
                bid.join()
                hid.join()
                b_slots = board_queue.get()
                h_slots = hands_queue.get()
                #self.get_board_slots_all_gs(board, board_queue)
                #self.get_all_hand_slots_all_gs(board, hands_queue)
                #board = random_board()
                bid = threading.Thread(target=self.get_board_slots_all_gs, args = (board, board_queue))
                hid = threading.Thread(target=self.get_all_hand_slots_all_gs, args = (board, hands_queue))
                bid.start()
                hid.start()
                try:
                    visited_boards[(b_slots[1], b_slots[2])] += 1
                    if not i%1000:
                        print i, (b_slots[1], b_slots[2]), visited_boards[(b_slots[1], b_slots[2])] 
                except:
                    visited_boards[(b_slots[1], b_slots[2])] = 1
                    print "new board visit", (b_slots[1], b_slots[2]), len(visited_boards)
                #print "getting hand slots"
                
                #print "DONE"
                x = 1
                while x < 4:
                    boards_odds_table[x][b_slots[x]] += 1.0/1024.0
                    x+=1
                boards_switch_table[1][b_slots[1]] += 1.0/1024.0
                boards_switch_table[2][b_slots[1],b_slots[2]] += 1.0/1024.0
                boards_switch_table[3][b_slots[1],b_slots[2],b_slots[3]] += 1.0/1024.0
                
                time2 = time.time()
                x = 0
                while x < 4:
                    bs = b_slots[x]
                    if x != 0:
                        st = switch_table[x][bs]
                    
                    ot = odds_table[x][bs]
                    
                    hss = h_slots[x-1]
                    hse = h_slots[x]
                    
                    #print type(self.HANDS)
                    if x != 0:
                        lzp.lib.add_values_to_gs_switch_and_odds(st.ctypes.data, ot.ctypes.data, hss.ctypes.data, hse.ctypes.data, self.i.n_types[x])
                    else:
                        lzp.lib.add_values_to_gs_switch_and_odds(None, ot.ctypes.data, hss.ctypes.data, hse.ctypes.data, self.i.n_types[x])
                    x += 1
                time3 = time.time()
                i+=1
                #print i
                getting_slots_time += (time2-time1)
                loops_time += (time3-time2)
                tot_time += time3-time1
        except KeyboardInterrupt:
            pass
            #            y = 0
#            while y < self.HANDS:
#                odds_table[0][0, h_slots[0][y]] += 0.1
#                y += 1
        total_loops = i
        print visited_boards
        print odds_table[2].sum()
        #print odds_table2[2].sum()
        #for x in odds_table[2] - odds_table2[2]:
        #    print x.sum(),
        #print
        print "hods"
        odds_table[0] = self.expand_odds_table_and_normalize(odds_table[0])
        for i in xrange(1,4):
            print "hodds", i
            odds_table[i] = self.expand_odds_table_and_normalize(odds_table[i])
            print "hswitch", i
            switch_table[i] = self.expand_odds_table_and_normalize(switch_table[i])
            print "bodds", i
            boards_odds_table[i] = self.expand_odds_table_and_normalize(boards_odds_table[i])
            print "bswitch", i
            boards_switch_table[i] = self.expand_odds_table_and_normalize(boards_switch_table[i])
        #NORMALIZE

#         for st in switch_table[1:]:
#             for board_type in st:
#                 for hand_start in board_type:
#                     tot = hand_start.sum()
#                     if tot != 0:
#                         hand_start /= tot

#         for ot in odds_table[1:]:
#             for board_type in ot:
#                 tot = board_type.sum()
#                 if tot != 0:
#                     board_type /= tot
                    
#         for gs in range(1,4):
#             self.i.gs_switch[gs] =  self.gs_switch[gs].ctypes.data_as(POINTER(c_double))
#         for gs in range(4):
#             self.i.gs_odds[gs] = self.gs_odds[gs].ctypes.data_as(POINTER(c_double))


        print "getting_slots_time", getting_slots_time/total_loops
        print "loops time", loops_time/total_loops
        print "total time", tot_time


    #PATHS

    def precalc_and_index_all_unique_states(self):
        def recursive_add(us, int_to_us, us_to_int):
            if us not in us_to_int:
                int_to_us.append(us)
                us_to_int[us] = len(int_to_us)-1
                next_states = self.get_next_unique_states(us)
                for state in next_states:
                    if state != None:
                        recursive_add(state, int_to_us, us_to_int)

        #if self.i.n_plr == 2:
        #    us = (0, 2,1,0.5,1)
        #else:
        #    us = (0,self.i.n_plr, self.i.n_plr-2)+tuple([0]*(self.i.n_plr-2))+(0.5,1)
        if self.i.n_plr == 2:
            us = (0, 2,1,0.0,0.0)
        else:
            us = (0,self.n_plr, self.n_plr-2)+tuple([0.0]*self.n_plr)
        
        int_to_us = []
        us_to_int = {}
        recursive_add(us, int_to_us, us_to_int)
                    
        self.int_to_us = int_to_us
        self.us_to_int = us_to_int
        self.n_us = len(int_to_us)
        print self.n_us

    def reset_path_stats(self):
        assert self.i.n_plr == 2
#        for i in range(4):
        self.path_odds = np.zeros((4,2,2,4), dtype=np.float64)

        #self.gs_switch_path = [None]*self.n_us
        #for i in range(self.n_us):
        self.gs_switch_path = np.zeros((self.n_us, 2,2,4), dtype=np.float64)

    def get_path_info_from_unique_state(self, us):
        gs = us[0]
        to_act = us[1]
        utg = us[2]
        bets = list(us)[3:]
        action = last_aggr = maxbets = 0
    
        if to_act != 1:
            return None
        if (gs == 0 and bets[-1] > 1) or (gs != 0 and bets[-1] > 0):
            action = 1
            maxbets = bets[-1]-1
            if utg == 0:
                last_aggr = 1
        else:
            action = 0
    
        return (action, last_aggr, maxbets)

    def add_path_to_stats(self, path):
        if self.i.n_plr == 2:
            us = (0, 2,1,0.5,1)
        else:
            us = (0,self.i.n_plr, self.i.n_plr-2)+tuple([0]*(self.i.n_plr-2))+(0.5,1)
            
        for act in path:
            if us == None or us[0] == 4:
                break
            path_info = self.get_path_info_from_unique_state(us)
            if path_info != None:
                #print us, path_info
                self.path_odds[(us[0],)+path_info] += 1.0/1024.0
                self.gs_switch_path[(self.us_to_int[us],)+path_info] += 1.0/1024.0

            rcf_states = self.get_next_unique_states(us)
            if act == "0" or act == "r":
                us = rcf_states[0]
            elif act == "1" or act == "c":
                us = rcf_states[1]
            else:
                us = rcf_states[2]
        
    def get_next_unique_states(self, us):
        
        #us = self.state
        r=c=f=None
        gs = us[0]
        to_act = us[1]
        cur_seat = us[2]
        bets = list(us)[3:]
        tot_bets = sum(bets)
#        print bets
        n_plr = len(bets)
        bets_max = bets[-1]

        if gs == 0 and tot_bets < 1.5: #blindit laittamatta
            if cur_seat <= 1:# 0 or cur_seat == n_plr-1:
                r = (gs, n_plr, (cur_seat+1)%n_plr)+tuple(bets[1:]+[bets_max+0.5])
            else:
                print "NEVER HERE?????"
                sys.exit()
                c = (gs, n_plr, (cur_seat-1)%n_plr)+tuple(bets[1:]+[bets_max])
            #print "blindit", us, (r,c,f)
        else:    
            if gs == 4:
                return (r,c,f)
            if bets_max < self.i.maxbets: #raise
                r = (gs, n_plr-1, (cur_seat+1)%n_plr)+tuple(bets[1:]+[bets_max+1])
            if to_act > 1:
                c = (gs, to_act-1, (cur_seat+1)%n_plr)+tuple(bets[1:]+[bets_max])
            else:
                c = (gs+1, n_plr, 0)+tuple([0.0]*n_plr)
            if n_plr > 2:
                if to_act > 1:
                    f = (gs, to_act-1, cur_seat%(n_plr-1))+tuple(bets[1:])
                else:
                    f = (gs+1, n_plr-1,0)+tuple([0.0]*(n_plr-1))
            else:
                f = (4, 1, (cur_seat+1)%n_plr, 0.0)
        return (r,c,f)

    def get_new_hand_state(self, n_players, sb, bb, cap):
        hand = {}
        hand["np_start"] = n_players
        hand["np_now"] = n_players
        hand["cap"] = cap
        hand["sb"] = sb
        hand["bb"] = bb
        hand["pot"] = sb+bb
        hand["plr"] = []
        hand["bets"] = [-1.0]*4
        hand["bets"][0] = 1.0
        hand["acts"] = ["","","",""]
        hand["to_act"] = 0
        hand["gamestate"] = 0
        for i in xrange(n_players):
            hand["plr"].append({})

        for i,p in enumerate(hand["plr"]):
            p["acts"] = [["", 0,0,0],["", 0,0,0],["", 0,0,0],["", 0,0,0]]
            p["bets"] = [0,0,0,0]
            p["rmoney"] = [0,0,0,0]
            p["cmoney"] = [0,0,0,0]
            p["seat"] = i
            p["first_last_act"] = [[-1,-1],[-1,-1],[-1,-1],[-1,-1]]

        plr = hand["plr"]
        if n_players == 2:
            plr[1]["bets"][0] = 0.5 
            plr[0]["bets"][0] = 1 
            hand["to_act"] = 1

        else:
            plr[0]["bets"][0] = 0.5 
            plr[1]["bets"][0] = 1 
            hand["to_act"] = 2
         
        return hand


    def add_actions_to_hand(self, actions, hand):
        h = hand
        p = hand["plr"]
        gs = hand["gamestate"]
        cap = hand["cap"]
        
        for act in actions:
            assert gs < 4
            if act == "0":
                #print h["bets"][gs], cap
                assert h["bets"][gs] <= cap
                h["bets"][gs] += 1
                #print h["to_act"], gs, p[h["to_act"]]["acts"] 
                if p[h["to_act"]]["acts"][gs][0] == "":
                    p[h["to_act"]]["first_last_act"][gs][0] = 0
                    p[h["to_act"]]["first_last_act"][gs][1] = 0
                else:
                    p[h["to_act"]]["first_last_act"][gs][1] = 0
                p[h["to_act"]]["acts"][gs][0] += "0"
                h["acts"][gs] += "0"
                p[h["to_act"]]["acts"][gs][1] += 1
                #print p[h["to_act"]]["bets"], h["bets"][gs]
                add_bets = (h["bets"][gs] - p[h["to_act"]]["bets"][gs])*(1+gs/2)
                h["pot"] += add_bets
                p[h["to_act"]]["rmoney"][gs] += add_bets
                p[h["to_act"]]["bets"][gs] = h["bets"][gs]
                h["to_act"] = (h["to_act"] + 1)%h["np_now"]
                
            elif act == "1":
                h["acts"][gs] += "1"
                if p[h["to_act"]]["acts"][gs][0] == "":
                    p[h["to_act"]]["first_last_act"][gs][0] = 1
                    p[h["to_act"]]["first_last_act"][gs][1] = 1
                else:
                    p[h["to_act"]]["first_last_act"][gs][1] = 1

                p[h["to_act"]]["acts"][gs][0] += "1"
                p[h["to_act"]]["acts"][gs][2] += 1
                add_bets = (h["bets"][gs] - p[h["to_act"]]["bets"][gs])*(1+gs/2)
                h["pot"] += add_bets
                p[h["to_act"]]["cmoney"][gs] += add_bets
                #h["pot"] += (h["bets"][gs] - p[h["to_act"]]["bets"][gs])
                p[h["to_act"]]["bets"][gs] = h["bets"][gs]
                h["to_act"] = (h["to_act"] + 1)%h["np_now"]
            elif act == "2":
                h["acts"][gs] += "2"
                if p[h["to_act"]]["acts"][gs][0] == "":
                    p[h["to_act"]]["first_last_act"][gs][0] = 2
                    p[h["to_act"]]["first_last_act"][gs][1] = 2
                else:
                    p[h["to_act"]]["first_last_act"][gs][1] = 2

                p[h["to_act"]]["acts"][gs][0] += "2"
                p[h["to_act"]]["acts"][gs][3] += 1
                folded_player = p.pop(h["to_act"])
                p.append(folded_player)
                #if p[h["to_act"]]["acts"][gs][3] != 0:
                #    h["to_act"] = 0
                h["np_now"] -=1
                h["to_act"] = (h["to_act"])%h["np_now"]
            #print  p[h["to_act"]]["bets"][gs], h["bets"][gs], p[h["to_act"]]["acts"][gs][0], "jee"
            if p[h["to_act"]]["bets"][gs] == h["bets"][gs] and p[h["to_act"]]["acts"][gs][0] != "":
                #GAMESTATE CHANGE0
                h["gamestate"] += 1
                gs = hand["gamestate"]
                if gs < 4:
                    h["to_act"] = 0
                    h["bets"][gs] = 0.0
    
    def get_available_actions_from_hand(self, h):
        if h["gamestate"] >= 4 or h["np_now"] < 2:
            return (False, False, False)
        ap = h["to_act"]
        p = h["plr"][ap]
        gs = h["gamestate"]
        h_bets = h["bets"][gs]
        p_bets = p["bets"][gs]
        r = c = f = True
        if h_bets == h["cap"]:
            r = False
        #if p_bets == 0:
        #    f = False
        return (r,c,f)

    def get_path_type(self, h):
        """
        first act, last act
        raise: total money, total committed
        total call money
        """
        if h["gamestate"] >= 4 or h["np_now"] < 2:
            return ()
        ap = h["to_act"]
        p = h["plr"]
        gs = h["gamestate"]
        retval = []
        #print h
        for j in xrange(h["gamestate"]+1):
            sub_ret = []
            for i in xrange(h["np_now"]):
                #print "adding", i
                sub_ret += [p[i]["first_last_act"][j][0], p[i]["first_last_act"][j][1]]
            sub_ret += [h["bets"][j]]
            retval.append(sub_ret)
        retval.append(h["pot"])
        return retval



    def get_unique_state_from_hand(self, h):
        gs = h["gamestate"]
        if gs >= 4 or h["np_now"] < 2:
            return (-1, -1, -1, -1)
        to_act = 0
        acted = 0
        for i in xrange(h["np_now"]):
            p = h["plr"][i]
            if p["acts"][gs][0] == "" or p["bets"][gs] < h["bets"][gs]:
                to_act += 1
            else:
                acted += 1
            #print "PLR", p
        return (gs, h["bets"][gs], acted, to_act, h["to_act"])

    def get_path_diff(self, a1, a2):
        if len(a1) != len(a2):
            return -1
        d = np.zeros((4), dtype=np.float64)
        print a1, len(a1)
        print a2, len(a2)
        for i1 in xrange(len(a1)-1):
            
            diff = 0
            for i2 in xrange(len(a1[i1])):
                d[i1] += abs(a1[i1][i2] - a2[i1][i2])
        diff += abs(a1[-1] - a2[-1])
        print "diff", d,diff


    #GETIING SLOTS

    def get_board_slot_flop(self, board):
        flop_i = self.ctoi3[board[0]][board[1]][board[2]]
        if self.b_slots[1] != None:
            s = self.b_slots[1][flop_i]
        else:
            type = self.gen_board_type(1, flop_i)
            s = self.get_slot_new(self.i.b_types[1], type.ctypes.data, len(self.b_types[1]), type.size)
        return s

    def get_board_slot_turn(self, board):
        flop_i = self.ctoi3[board[0]][board[1]][board[2]]
        if self.b_slots[2] != None:
            turn_i = flop_i*52+51-board[3]
            s = self.b_slots[2][turn_i]
        else:
            type = self.gen_board_type(2, flop_i, board[3])
            s = self.get_slot_new(self.i.b_types[2], type.ctypes.data, len(self.b_types[2]), type.size)
        return s
    def get_board_slot_river(self, board):
        if self.b_slots[3] != None:
            turn_i = self.ctoi4[board[0]][board[1]][board[2]][board[3]]
            river_i = turn_i*52+51-board[4]
            s = self.b_slots[3][river_i]
        else:
            flop_i = self.ctoi3[board[0]][board[1]][board[2]]
            type = self.gen_board_type(3, flop_i, board[3], board[4])
            s = self.get_slot_new(self.i.b_types[3], type.ctypes.data, len(self.b_types[3]), type.size)
        return s


    def get_board_slot(self, board, gs = -1):
        l = len(board)
        if l < 3:
            return 0
        if type(board[0]) == str:
            nb = [0]*l
            for i,x in enumerete(board):
                nb[i] = ct.stoi[x]
            board = nb
        if l == 3:
            return self.get_board_slot_flop(board)
        if l == 4:
            return self.get_board_slot_turn(board)
        return self.get_board_slot_river(board)
            
    # def get_board_slots_all_gs(self, board):
    #     prev_board = self.get_board_slots_all_gs_prev_board
    #     prev_slots = self.get_board_slots_all_gs_prev_slots
    #     #print prev_board, board, prev_slots
    #     assert len(board) == 5
    #     b_slots = [0]*4
    #     if board[:3] != prev_board[:3]:
    #         b_slots[1] = self.get_board_slot(board[:3])
    #     else:
    #         b_slots[1] = prev_slots[1]
    #     if board[:4] != prev_board[:4]:
    #         b_slots[2] = self.get_board_slot(board[:4])
    #     else:
    #         b_slots[2] = prev_slots[2]
    #     if board != prev_board:
    #         b_slots[3] = self.get_board_slot(board)
    #     else:
    #         b_slots[3] = prev_slots[3]
    #     #b_slots[2] = self.get_board_slot(board[:4])
    #     #b_slots[3] = self.get_board_slot(board)
    #     self.get_board_slots_all_gs_prev_slots = copy.copy(b_slots)
    #     self.get_board_slots_all_gs_prev_board = copy.copy(board)

    #     return b_slots
    
    def get_board_slots_all_gs(self, board, q = None):
        prev_board = self.get_board_slots_all_gs_prev_board
        prev_slots = self.get_board_slots_all_gs_prev_slots
        #print prev_board, board, prev_slots
        assert len(board) == 5
        b_slots = [0]*4
        if board[:3] != prev_board[:3]:
            b_slots[1] = self.get_board_slot(board[:3])
        else:
            b_slots[1] = prev_slots[1]
        if board[:4] != prev_board[:4]:
            b_slots[2] = self.get_board_slot(board[:4])
        else:
            b_slots[2] = prev_slots[2]
        if board != prev_board:
            b_slots[3] = self.get_board_slot(board)
        else:
            b_slots[3] = prev_slots[3]
        #b_slots[2] = self.get_board_slot(board[:4])
        #b_slots[3] = self.get_board_slot(board)
        self.get_board_slots_all_gs_prev_slots = copy.copy(b_slots)
        self.get_board_slots_all_gs_prev_board = copy.copy(board)
        #print self.b_slots
        #print b_slots
        if q != None:
            q.put(b_slots)
        else:
            return b_slots

    def get_hand_slot_preflop(self, hand_i):
        if self.slots[0] != None:
            return self.slots[0][hand_i]
        else:
            print "preflop hand slots not loaded. Only precalc supported now"
            sys.exit(0)

    def get_hand_slot_flop(self, board, hand_i):
        flop_i = self.ctoi3[board[0]][board[1]][board[2]]
        if self.slots[1] != None:
            return self.slots[1][flop_i, hand_i]
        else:
            print "flop hand slots not loaded. Only precalcs supported now"
            sys.exit(0)

    def get_hand_slot_turn(self, board, hand_i):
        turn_i = self.ctoi4[board[0]][board[1]][board[2]][board[3]]
        if self.slots[2] != None:
            return self.slots[2][turn_i, hand_i]
        else:
            print "turn hand slots not loaded. Only precalcs supported now"
            sys.exit(0)

    def get_hand_slot_river(self, board, hand_i):
        if self.types[3] == None:
            print "river types not loaded, can't get slot"
            sys.exit(0)
        flop_i = self.ctoi3[board[0]][board[1]][board[2]]
        t = self.get_single_river_type(board, hand_i)
        s = get_slot(3, t)
        return s

    def get_hand_slot(self, board, hand):
        bl = len(board)
        hl = len(hand)
        if type(board[0]) == str:
            nb = [0]*bl
            for i,x in enumerete(board):
                nb[i] = ct.stoi[x]
            board = nb
        if type(hand[0]) == str:
            nh = [0]*hl
            for i,x in enumerete(hand):
                nh[i] = ct.stoi[x]
            hand = nh
        hand_i = self.ctoi2[hand[0]][hand[1]]
        if bl == 0:
            return get_hand_slot_preflop(hand_i)
        if bl == 3:
            return get_hand_slot_flop(board,hand_i)
        if bl == 4:
            return get_hand_slot_turn(board,hand_i)
        if bl == 5:
            return get_hand_slot_river(board,hand_i)
    
    def get_all_hand_slots_preflop(self):
        if self.slots[0] != None:
            return self.slots[0]
        else:
            print "ei0"
            sys.exit(0)


    def get_all_hand_slots_flop(self,board):
        flop_i = self.ctoi3[board[0]][board[1]][board[2]]
        if self.slots[1] != None:
            return self.slots[1][flop_i]
        else:
            print "ei1"
            sys.exit(0)

    def get_all_hand_slots_turn(self,board):
        turn_i = self.ctoi4[board[0]][board[1]][board[2]][board[3]]
        if self.slots[2] != None:
            return self.slots[2][turn_i]
        else:
            print "ei2"
            sys.exit(0)
    
    def get_all_hand_slots_river(self,board, slots = None):
        river_i
        flop_i = self.ctoi3[board[0]][board[1]][board[2]]
        return self.get_slots_for_river(flop_i, board[3], board[4], slots)

    def get_all_hand_slots(self, board):
        bl = len(board)
        if bl == 0:
            return self.get_all_hand_slots_preflop()

        if type(board[0]) == str:
            nb = [0]*bl
            for i,x in enumerete(board):
                nb[i] = ct.stoi[x]
            board = nb
        if bl == 3:
            return self.get_all_hand_slots_flop(board)
        if bl == 4:
            return self.get_all_hand_slots_turn(board)
        if bl == 5:
            return self.get_all_hand_slots_river(board)

    def get_all_hand_slots_all_gs(self, board, q = None):
        assert len(board) == 5
        h_slots = [None]*4
        h_slots[0] = self.get_all_hand_slots([])
        h_slots[1] = self.get_all_hand_slots(board[:3])
        h_slots[2] = self.get_all_hand_slots(board[:4])
        h_slots[3] = self.get_all_hand_slots(board)
        if q != None:
            q.put(h_slots)
        else:
            return h_slots

    

if __name__ == "__main__":
    zp = zpoker("testipeli")
    #zp.load_hand_types(2)
    #zp.load_hand_types(3)
    zp.load_hand_diffs(2)
    zp.load_hand_diffs(3)
    print zp.diffs_order[2][50]
#    t_types = zp.gen_types_for_turn(9433, 14)
#    t_slots = zp.get_slots_for_all(2, None, t_types)
#    r_slots = zp.get_slots_for_river(9433, 14, 13)
#    print t_slots
#    print r_slots
#    print t_slots[196]
#    print r_slots[196]
