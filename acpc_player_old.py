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

def parse_matchstate(ms, p):
    global priv_types
    global pub_types
    ms_spl = ms.split(":")
    #print >>sys.stderr,  ms_spl
    cmd, pos, hand_n, betting, cards = ms_spl
    bets = betting.split("/")
    #print >>sys.stderr,  bets
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
    if not hi.cur_us or hi.cur_us.contents.gamestate == 4:
        #print >>sys.stderr,  "No unique state"
        return None
    pos = int(pos)
    hand_n = int(hand_n)

    if hi.cur_us.contents.root_idx == 2:
        #new hand
        pub_types = np.zeros(p.zp.i.n_type_types[1], dtype=np.int16)
        pub_types.fill(-1)
        #test_pub_types = np.zeros(p.zp.i.n_type_types[1], dtype=np.int16)
        priv_types = np.zeros(p.zp.i.n_type_types[0], dtype=np.int16)
        priv_types.fill(-1)
    else:
        pub_types[:7] = -1

    if hi.cur_us.contents.cur_seat != pos:
        #print >>sys.stderr,  "NOT MY TURN"
        return None
    print >>sys.stderr
    print >>sys.stderr,  "ACTS", bets
    #print >>sys.stderr,  "MY TURN"
    cards_spl = cards.split("/")
    hole_cards = cards_spl[0]
    board = cards_spl[1:]
    bc = []
    print >>sys.stderr,  "BOARD",
    for b in board:
        print >>sys.stderr,  b,
        while len(b) >= 2:
            bc.append(ct.stoi[b[:2]])
            b = b[2:]
    print >>sys.stderr

    for i, c in enumerate(bc):
        hi.board[i] = c
    
    hole_spl = hole_cards.split("|")
    self_hand = hole_spl[pos]

    print >>sys.stderr,  "HOLE", self_hand

    assert(len(self_hand) == 4)
    self_hand = [ct.stoi[self_hand[:2]], ct.stoi[self_hand[2:]]]

    for i, c in enumerate(self_hand):
        hi.hole_cards[i] = c

    lzp.set_st_from_hand_info(C.pointer(p.zp.i), C.pointer(hi), pub_types.ctypes.data_as(C.POINTER(C.c_int16)),priv_types.ctypes.data_as(C.POINTER(C.c_int16)))
    #lzp.set_mutable_types_from_path(test_pub_types.ctypes.data_as(C.POINTER(C.c_int16)), test_path.ctypes.data_as(C.c_char_p), test_path_i, C.pointer(p.root_state.unique_root_node))
    #print >>sys.stderr,  pub_types
    #print >>sys.stderr,  test_pub_types
    action_odds = p.us_by_id(hi.cur_us.contents.root_idx).get_action_odds_from_file(p.model_path, pub_types, priv_types, src)

    #print >>sys.stderr,  self_hand, board
    print >>sys.stderr,  "ACTION ODDS", action_odds
#    hs = p.zp.get_hand_slot(bc, self_hand)
#    print >>sys.stderr,  "self hand", self_hand, hs
#    board_slots = p.zp.get_board_slots_all_gs(bc)
#    print >>sys.stderr,  board_slots
    
#    us = p.get_unique_state_from_path(bets)    
    return action_odds

gamedir = "testipeli/"
p = plrmodel.plrmodel(gamedir)
p.reset_model_path(sys.argv[3])
#p.reset_model_path(gamedir + "plrmodel_model_2011_11_17_15_39_39")
#p.load_model(gamedir + "plrmodel_combo_sb_plain_6774719_2011_06_23_12_15_39", 0)
#p.load_model(gamedir + "plrmodel_combo_sb_plain_6774719_2011_06_23_12_15_39", 1)

p.zp.load_types_for_all_types()
p.zp.load_diffs_for_all_types()
p.zp.load_slots_for_all_types()
p.zp.set_info_pointers()

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print >>sys.stderr,  "connecting to", sys.argv[1], sys.argv[2]
s.connect((sys.argv[1], int(sys.argv[2])))

src = int(sys.argv[4])

print >>sys.stderr,  s.send("VERSION:1.0.0\n")

pub_types = np.zeros(p.zp.i.n_type_types[1], dtype=np.int16)
pub_types.fill(-1)
#test_pub_types = np.zeros(p.zp.i.n_type_types[1], dtype=np.int16)
priv_types = np.zeros(p.zp.i.n_type_types[0], dtype=np.int16)
priv_types.fill(-1)
   
ms = s.recv(4096)
while ms:
    while "\r\n" in ms:
        #print >>sys.stderr,  "received", [ms]
        ms_spl = ms.split("\r\n", 1)
        #print >>sys.stderr,  "ms_spl", ms_spl
        if len(ms_spl) > 1:
            cur_ms = ms_spl[0]
            ms = ms_spl[1]
            
            cur_ms = cur_ms.rstrip()
            #print >>sys.stderr,  "striped", [cur_ms]
            action_odds = parse_matchstate(cur_ms,p)
            #us = p.get_unique_state_from_path(bets)
            
            #            print >>sys.stderr,  pos, us.utg, us.state
            if action_odds != None:
                r = random.random()
                if r < action_odds[0]:
                    action = cur_ms + ":r\n"
                    print >>sys.stderr,  r, "raising"
                elif r < action_odds[0] + action_odds[1]:
                    action = cur_ms + ":c\n"
                    print >>sys.stderr,  r, "calling"
                else:
                    action = cur_ms + ":f\n"
                    print >>sys.stderr,  r, "folding"

                #print >>sys.stderr,  "sending", [action]
                s.send(action)
                

    ms += s.recv(4096)
