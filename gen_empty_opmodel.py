import libzpoker
import sys
import matplotlib.pyplot as plt
import random
import card_table as ct
import time
from ctypes import *

class unique_root(Structure):
    pass
unique_root._fields_ = [("next", POINTER(unique_root)*3),
                        ("model_tree", c_void_p)]


def get_next_unique_state(dll_plrmodel, zp, act_str, states):
    hs = zp.get_new_hand_state(2, 0.5, 1, 4)
    for a in act_str:
        zp.add_actions_to_hand(a, hs)
    if hs["gamestate"] == 4:
        if "showdown" not in states[hs["gamestate"]]:
            states[hs["gamestate"]]["showdown"] = [4, None, None, None, None]
            ur = unique_root()
            ur.next[0] = None
            ur.next[1] = None
            ur.next[2] = None
            states[hs["gamestate"]]["showdown"][4] = unique_root()
        return states[hs["gamestate"]]["showdown"]
            
    possible = zp.get_available_actions_from_hand(hs)
    if possible == (False, False, False):
        return None
    state = zp.get_unique_state_from_hand(hs)
    if state in states[hs["gamestate"]]:
        return states[hs["gamestate"]][state]
    else:
        states[hs["gamestate"]][state] = [hs["gamestate"], None, None, None, None]
    print possible, hs["gamestate"]
    ur = unique_root()
    if possible[0] == True:
        states[hs["gamestate"]][state][1] = get_next_unique_state(dll_plrmodel, zp, act_str + "0", states)
        if states[hs["gamestate"]][state][1] != None:
            ur.next[0] = pointer(states[hs["gamestate"]][state][1][4])
    
    if possible[1] == True:
        states[hs["gamestate"]][state][2] = get_next_unique_state(dll_plrmodel, zp, act_str + "1", states)
        if states[hs["gamestate"]][state][2] != None:
            ur.next[1] = pointer(states[hs["gamestate"]][state][2][4])
    
    if possible[2] == True:
        states[hs["gamestate"]][state][3] = get_next_unique_state(dll_plrmodel, zp, act_str + "2", states)
        if states[hs["gamestate"]][state][3] != None:
            ur.next[2] = pointer(states[hs["gamestate"]][state][3][4])
    
    
    ur.model_tree = dll_plrmodel.generate_empty_plrmodel_tree(None, None, 0, hs["gamestate"], hs["gamestate"])

    states[hs["gamestate"]][state][4] = ur
    
    return states[hs["gamestate"]][state]
    

gamedir = sys.argv[1]
zp = libzpoker.libzpoker(gamedir)
dll_plrmodel = cdll.LoadLibrary("libplrmodel.dylib")
dll_plrmodel.generate_empty_plrmodel_tree.restype = c_void_p


states = [{},{},{},{},{}]
old_lens = [0,0,0,0]
act_str = ""
get_next_unique_state(dll_plrmodel, zp, act_str, states)

#print states
