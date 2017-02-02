import socket
import select
import sys
import plrmodel
import card_table as ct
import random
import datatypes as dt
import ctypes as C
import lzp
import numpy as np
import pdb

def parse_matchstate(ms, p):
    global priv_types
    global pub_types
    ms_spl = ms.split(":")
    #print ms_spl
    cmd, pos, hand_n, betting, cards = ms_spl
    bets = betting.split("/")
    #print bets
    hi = dt.hand_info()
    hi.root_us = C.pointer(p.root_state.unique_root_node)
    hi.cur_us = C.pointer(p.root_state.unique_root_node)
    
    for i in xrange(4):
        hi.acts_i[i] = 0
    lzp.add_action_to_hi(C.pointer(hi), 0)
    lzp.add_action_to_hi(C.pointer(hi), 0)

    #test_path = np.zeros(256, dtype=np.int8)
    #test_path_i = 2
    for i, street in enumerate(bets):
        for act in street:
            print act,
            act_i = 1
            if act == "r":
                act_i = 0
            elif act == "c":
                act_i = 1
            elif act == "f":
                act_i = 2
            lzp.add_action_to_hi(C.pointer(hi), act_i)
            #test_path[test_path_i] = act_i
            #test_path_i+=1
            #us = p.get_unique_state_from_path(bets)
        print "|",
    print 
    if not hi.cur_us or hi.cur_us.contents.gamestate == 4:
        #print "No unique state"
        return None
    pos = int(pos)
    hand_n = int(hand_n)

    #   if hi.cur_us.contents.root_idx == 2:
    #   #new hand
    #   pub_types = np.zeros(p.zp.i.n_type_types[1], dtype=np.int16)
    #   pub_types.fill(-1)
    #   #test_pub_types = np.zeros(p.zp.i.n_type_types[1], dtype=np.int16)
    #   priv_types = np.zeros(p.zp.i.n_type_types[0], dtype=np.int16)
    #   priv_types.fill(-1)
    #else:
    #    pub_types[:7] = -1

    if hi.cur_us.contents.cur_seat != pos:
        #print "NOT MY TURN"
        return None

    #print "MY TURN"
    cards_spl = cards.split("/")
    hole_cards = cards_spl[0]
    board = cards_spl[1:]
    bc = []
    print cards_spl
    for b in board:
        while len(b) >= 2:
            if b[:2] == "10":
                bc.append(ct.stoi[b[:3]])
                b = b[3:]
            else:
                bc.append(ct.stoi[b[:2]])
                b = b[2:]
            #print 

    for i, c in enumerate(bc):
        hi.board[i] = c
        
    hole_spl = hole_cards.split("|")
    self_hand = hole_spl[pos]
    assert(len(self_hand) == 4)
    self_hand = [ct.stoi[self_hand[:2]], ct.stoi[self_hand[2:]]]

    for i, c in enumerate(self_hand):
        hi.hole_cards[i] = c

    
    pub_types.fill(-1)
    priv_types.fill(-1)
    lzp.set_st_from_hand_info(C.pointer(p.zp.i), C.pointer(hi), pub_types.ctypes.data_as(C.POINTER(C.c_int16)),priv_types.ctypes.data_as(C.POINTER(C.c_int16)))
    #lzp.set_mutable_types_from_path(test_pub_types.ctypes.data_as(C.POINTER(C.c_int16)), test_path.ctypes.data_as(C.c_char_p), test_path_i, C.pointer(p.root_state.unique_root_node))
    #print pub_types
    #print test_pub_types
    #print pub_types, priv_types
    pub_types[0] = 0
    #action_odds = p.us_by_id(hi.cur_us.contents.root_idx).get_action_odds_from_file(p.model_path, pub_types, priv_types, src)
    us = p.us_by_id(hi.cur_us.contents.root_idx)
    pmn = lzp.get_first_matching_situ(us.unique_root_node.model_tree, pub_types.ctypes.data_as(C.POINTER(C.c_int16)), 0, None, 0)
    slot_i = lzp.get_closest_local_type(pmn, priv_types[hi.cur_us.contents.gamestate])
    #print pmn.contents.data_index, slot_i
    #    pdb.set_trace()

    # r = us.avg_odds_mmap[pmn.contents.data_index*3+slot_i]
    # c = us.avg_odds_mmap[pmn.contents.data_index*3+pmn.contents.len+slot_i]
    # f = us.avg_odds_mmap[pmn.contents.data_index*3+pmn.contents.len*2+slot_i]

    # tot = r+c+f
    # r/=tot
    # c/=tot
    # f/=tot
    # action_odds_avg = (r,c,f)
    r = float(us.byte_odds_mmap[pmn.contents.data_index*2+ slot_i*2])
    c = float(us.byte_odds_mmap[pmn.contents.data_index*2+ slot_i*2+1])
    f = 255.0-r-c
    
    if f < 0:
        print >>sys.stderr, "ZERO INFO", r,c,f, ms
        r = 0.0
        c = 1.0
        f = 0.0
    else:
        tot = r+c+f
        r/=tot
        c/=tot
        f/=tot

    # pdb.set_trace()
    # if us.gamestate == 3:
    #     td = np.zeros(1, dtype=dt.types_data_np)
    #     deck = np.array(bc, dtype=np.int8)
    #     lzp.gen_types_data(C.pointer(p.zp.i), deck[:5].ctypes.data_as(C.POINTER(C.c_int8)), td.ctypes.data)
    #     assert(tuple(td["public_types"][0][7:10]) == tuple(pub_types[7:10]))
    #     pdb.set_trace()


    action_odds = (r,c,f)

    #print action_odds_avg
    #print action_odds
    #print
    #print self_hand, board
    #print action_odds
#    hs = p.zp.get_hand_slot(bc, self_hand)
#    print "self hand", self_hand, hs
#    board_slots = p.zp.get_board_slots_all_gs(bc)
#    print board_slots
    
#    us = p.get_unique_state_from_path(bets)    
    return action_odds

gamedir = sys.argv[3]
src = sys.argv[6]

raise_adjust = float(sys.argv[7])
call_adjust = float(sys.argv[8])
fold_adjust = float(sys.argv[9])

p = plrmodel.plrmodel(gamedir)

p.reset_model_path(sys.argv[4])
p.load_struct(seat=0)
p.recount_data_index(seat=0)
p.mmap_byte_odds(seat=0, source=src)

p.reset_model_path(sys.argv[5])
p.load_struct(seat=1)
p.recount_data_index(seat=1)
p.mmap_byte_odds(seat=1, source=src)

p.set_byte_odds_from_mmap()
#p.mmap_avg_odds()
#p.set_avg_odds_from_mmap()

#p.reset_model_path(gamedir + "plrmodel_model_2011_11_17_15_39_39")
#p.load_model(gamedir + "plrmodel_combo_sb_plain_6774719_2011_06_23_12_15_39", 0)
#p.load_model(gamedir + "plrmodel_combo_sb_plain_6774719_2011_06_23_12_15_39", 1)

#ep.zp.load_types_for_all_types()
p.zp.load_diffs_for_all_types(memory_map = True)
#sys.exit()
p.zp.load_slots_for_all_types()
p.zp.set_info_pointers()

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#print "connecting to", sys.argv[1], sys.argv[2]
#s.connect((sys.argv[1], int(sys.argv[2])))


#print s.send("VERSION:2.0.0\r\n")

pub_types = np.zeros(p.zp.i.n_type_types[1], dtype=np.int16)
pub_types.fill(-1)
#test_pub_types = np.zeros(p.zp.i.n_type_types[1], dtype=np.int16)
priv_types = np.zeros(p.zp.i.n_type_types[0], dtype=np.int16)
priv_types.fill(-1)

#action_odds = parse_matchstate("MATCHSTATE:0:49:rrc/rrrrc/rrc/crrr:AhAc|/Kd9c6s/4h/As",p)
#action_odds = parse_matchstate("MATCHSTATE:0:49:rrrc/rc/cr:7d6s/7c2h6c/8h",p)
#action_odds = parse_matchstate("MATCHSTATE:0:49:rrc/rc/rc/cr:7h7s|/KsJs5d/Kd/Jc/",p)
#action_odds = parse_matchstate("MATCHSTATE:0:49:rrc/rc/rc/cr:8h8s|/KsJs5d/Kd/Jc/",p)
#action_odds = parse_matchstate("MATCHSTATE:0:47:r:Jd9s|",p)
#action_odds = parse_matchstate("MATCHSTATE:1:47::|Jd9s",p)
#action_odds = parse_matchstate("MATCHSTATE:1:49:rc/rrr:|9h2s/9cTcKd/Ad/",p)
#action_odds = parse_matchstate("MATCHSTATE:0:49:cc/cc/cc/:Th7c|/2s2h2d/2c/3s/",p)
#print action_odds
#sys.exit()
#ms = s.recv(4096)
ms = ""
connected = False

action_dic = {}

action_dic[(0,0)] = 0.0
action_dic[(0,1)] = 0.0
action_dic[(0,2)] = 0.0
action_dic[(1,0)] = 0.0
action_dic[(1,1)] = 0.0
action_dic[(1,2)] = 0.0
action_dic[(2,0)] = 0.0
action_dic[(2,1)] = 0.0
action_dic[(2,2)] = 0.0

action_matrix = np.zeros((3,3), dtype=np.float64)
actions_sum = np.zeros(3, dtype=np.float64)
tot_actions = 0.0
while True:
    if not connected:
        print "connecting to", sys.argv[1], sys.argv[2]
        s.connect((sys.argv[1], int(sys.argv[2])))
        print s.send("VERSION:2.0.0\r\n")
        connected = True
        
    while "\r\n" in ms:
        #print "received", [ms]
        ms_spl = ms.split("\r\n", 1)
        #print "ms_spl", ms_spl
        if len(ms_spl) > 1:
            cur_ms = ms_spl[0]
            ms = ms_spl[1]
            
            cur_ms = cur_ms.rstrip()
            #print "striped", [cur_ms]
            action_odds = parse_matchstate(cur_ms,p)
            #us = p.get_unique_state_from_path(bets)
            #print action_odds
            # r*=raise_adjust
            # c*=call_adjust
            # f*=fold_adjust
            # tot = r+c+f
            # r/=tot
            # c/=tot
            # f/=tot
            # print r,c,f
            #            print pos, us.utg, us.state
            if action_odds != None:
                adjusted_odds = [0.0,1.0,0.0]
                adjusted_odds[0] = action_odds[0]*raise_adjust
                adjusted_odds[1] = action_odds[1]*call_adjust
                adjusted_odds[2] = action_odds[2]*fold_adjust
                
                tot = sum(adjusted_odds)
                adjusted_odds[0] /= tot
                adjusted_odds[1] /= tot
                adjusted_odds[2] /= tot
                print "ODDS:", action_odds, "ADJUSTED:", adjusted_odds
                r = random.random()
                if r < action_odds[0]:
                    action = cur_ms + ":r\r\n"
                    a1 = 0
                    #print r, "raising"
                elif r < action_odds[0] + action_odds[1]:
                    action = cur_ms + ":c\r\n"
                    a1 = 1
                    #print r, "calling"
                else:
                    action = cur_ms + ":f\r\n"
                    a1 = 2
                    #print r, "folding"

                if r < adjusted_odds[0]:
                    adjusted = cur_ms + ":r\r\n"
                    a2 = 0
                    #print r, "raising"
                elif r < adjusted_odds[0] + adjusted_odds[1]:
                    adjusted = cur_ms + ":c\r\n"
                    a2 = 1
                    #print r, "calling"
                else:
                    adjusted = cur_ms + ":f\r\n"
                    a2 = 2
                    #print r, "folding"

                tot_actions += 1.0
                #                action_dic[(a1,a2)] += 1
                action_matrix[a1,a2] += 1
                actions_sum[a2] += 1
                #for ak, av in action_dic.items():
                #    print ak, av/tot_actions, "||",
                #print
                am_sum = np.sum(action_matrix, axis=1).reshape(3,1)

                #if not 0 in am_sum:
                #    print action_matrix/am_sum
                #print actions_sum/np.sum(actions_sum)
                #print "sending", [action]
                #s.send(action)
                s.send(adjusted)
                

    ms += s.recv(4096)
