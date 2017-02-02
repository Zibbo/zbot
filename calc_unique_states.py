import libzpoker
import sys
import matplotlib.pyplot as plt
import random
import card_table as ct
import time




gamedir = sys.argv[1]
zp = libzpoker.libzpoker(gamedir)




states = [{},{},{},{}]
old_lens = [0,0,0,0]

while True:
    hs = zp.get_new_hand_state(3, 0.5, 1, 4)
    while True:
        possible = zp.get_available_actions_from_hand(hs)
        if possible == (False, False, False):
            break
        while True:
            act = random.randint(0,2)
            if possible[act] == True:
                break
        zp.add_actions_to_hand(str(act), hs)
        state = zp.get_unique_state_from_hand(hs)
        if state == (-1,-1,-1,-1):
            break
        try:
            states[hs["gamestate"]][state] += 1
        except:
            states[hs["gamestate"]][state] = 1
    new_lens = [len(states[0]),len(states[1]),len(states[2]),len(states[3])]
    if new_lens != old_lens:
        print new_lens
        old_lens = new_lens
    
sys.exit(0)
gs = int(sys.argv[2])

n_types_start = int(sys.argv[3])
n_types_stop = int(sys.argv[4])

b_types = int(sys.argv[5])

zp.i.n_types[gs-1] = n_types_start
zp.i.n_types[gs] = n_types_stop

zp.i.n_btypes[gs] = b_types

if gs == 3:
    zp.load_hand_types(gs)

if not zp.load_hand_diffs(gs):
    sys.exit(0)
if not zp.load_hand_diffs(gs-1):
    sys.exit(0)
if not zp.load_hand_slots(gs):
    sys.exit(0)
if not zp.load_hand_slots(gs-1):
    sys.exit(0)


b1 = ['6d', '2d', 'Kh']
b2 = ['5d', '4d', 'Kh']

flop_i_1 = zp.ctoi3[ct.stoi[b1[0]]][ct.stoi[b1[1]]][ct.stoi[b1[2]]]
flop_i_2 = zp.ctoi3[ct.stoi[b2[0]]][ct.stoi[b2[1]]][ct.stoi[b2[2]]]

t1 = zp.gen_board_type(gs, flop_i_1, ct.stoi["6h"], ct.stoi["Qd"])
t2 = zp.gen_board_type(gs, flop_i_2, ct.stoi["5h"], ct.stoi["Qs"])



print t1
print t2
print (abs(t1-t2)**2).sum()
