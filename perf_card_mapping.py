import time
import numpy as np
import suit_iso
import defs
import lzp
import pdb
import datatypes as dt
import card_table as ct
import ctypes as C

class node:
    def __init__(self, board, path, mapping_cache, nfo_cache, rng):
        #self.seat, self.gamestate, self.bets, self.potsize, self.stake, self.toraise, self.tocall, self.actions =
        self.nfo = get_data_from_path(path, nfo_cache)
        if self.nfo.gamestate <= 3:
            try:
                self.mapping = mapping_cache[tuple(board)]
            except KeyError:
                self.mapping = get_slot_mapping(board)
                mapping_cache[tuple(board)] = self.mapping
            self.regs = np.zeros((3,self.mapping.max()), dtype=np.float64)
        else:
            self.mapping = None
            #self.regs = None
        self.valid_actions = np.zeros(3, dtype=np.int32)
        
        self.valid_actions[np.array(self.nfo.actions)] = 1
        self.rng = rng
        
    def calc_odds_from_regs(self):
        odds = np.zeros((3,defs.HANDS), dtype=np.float64)
        lzp.get_odds(odds, self.regs, self.mapping, self.valid_actions, self.regs.shape[1], defs.HANDS, self.rng)
        return odds

    def update_regrets(self, ev):
        lzp.update_regs(self.regs, self.odds, ev, self.mapping, self.valid_actions, self.regs.shape[1], self.odds.shape[1], self.rng)
        

def get_river_slot_mapping(board):
    hand_values = np.zeros((defs.HANDS), dtype=dt.hand_hv2_np)
    flop_i = ct.ctoi3[board[0],board[1],board[2]]
    #print flop_i
    lzp.get_hand_hv2(board.ctypes.data_as(C.POINTER(C.c_int8)), hand_values.ctypes.data_as(C.POINTER(dt.hand_hv2)))

    #lzp.lib.get_river_hand_hv2_all_hands(flop_i, board[3], board[4], hand_values.ctypes.data_as(C.POINTER(dt.hand_hv2)))
    mapping = np.zeros(defs.HANDS, dtype=np.int32)-1
    mapping[hand_values["sample_i"]] = hand_values["hv"]
    mapping[mapping == 65535] = -1
    return mapping

def get_slot_mapping(board):
    if len(board) == 5:
        return get_river_slot_mapping(board)
    dc = get_dead_cards_mask(board)
    items = dc.sum()
    mapping = np.zeros(defs.HANDS, dtype=np.int32)-1
    mapping[dc] = np.arange(items, dtype=np.int32)
    return mapping
    
def map_cards_to_slots(board):
    mapping = suit_iso.get_mapping(suit_iso.get_suits(board))
    m = mapping[ct.itoc2[np.arange(defs.HANDS)]/13]*13+ct.itoc2[np.arange(defs.HANDS)]%13
    m[np.where(m[:,0] == m[:,1]),0]+=13
    cm = ct.ctoi2[m[:,0], m[:,1]]
    cm[np.any(np.in1d(ct.itoc2, board).reshape(1326,2), axis=1)] = -1
    cm = (defs.HANDS-1)-cm
    cm[cm==defs.HANDS] = -1
    return cm

def get_dict_key(cards, path):
    return (tuple(suit_iso.morph_cards_2(cards)), path)

def get_data_from_path(path, cache = None):
    if cache != None:
        try:
            return cache[path]
        except KeyError:
            pass
    potsize = 1.5
    seat = 1
    gamestate = 0
    bets = 1
    tocall = 0.5
    toraise = 1.5
    stake = [1.0, 0.5]
    actions = (True, True, True)

    for act in path:
        if act == "0":
            potsize += toraise
            stake[seat] += toraise
            tocall = toraise-tocall
            toraise = tocall*2
            seat = (seat+1)%2
            bets += 1
        elif act == "1":
            potsize += tocall
            stake[seat] += tocall
            tocall = 0.0
            if (bets == 0 and gamestate > 0 and seat == 0) or (bets == 1 and seat == 1 and gamestate == 0):
                seat = (seat+1)%2
            else:
                gamestate += 1
                seat = 0
                bets = 0
            
            toraise = (gamestate/2+1.0)
        elif act == "2":
            gamestate += 10
            seat = (seat+1)%2
            toraise = 0.0
            tocall = 0.0
        if gamestate >= 4:
            actions = (False, False, False)
            break
        if tocall == 0.0:
            actions = (True, True, False)
        elif bets == 4:
            actions = (False, True, True)
        else:
            actions = (True, True, True)
    retval = dt.path_nfo(seat, gamestate, bets, potsize, tuple(stake), toraise, tocall, actions)
    
    if cache != None:
        cache[path] = retval
    return retval



def rec_walk(path, c = None):
    d = get_data_from_path(path, c)
    #print d, path
    for i,act in enumerate(d[-1]):
        if act:
            rec_walk(path+str(i), c)

def gen_nodes(path, board, node_cache, nfo_cache, mapping_cache):
    n = node(board, path, mapping_cache, nfo_cache, None)
    if n.nfo.gamestate <= 3:
        node_key = (tuple(board),path)
    else:
        node_key = path
    node_cache[node_key] = n
    if n.nfo.gamestate < 3:
        print node_key
    #print d, path
    for i,act in enumerate(n.nfo.actions):
        if act:
            if i == 1 and (n.nfo.bets > 0 or n.nfo.seat == 1):
                next_cards, multi = get_next_card_list(board)
                for c in next_cards:
                    new_board = np.append(board,c)
                    gen_nodes(path+str(i), new_board, node_cache, nfo_cache, mapping_cache)
            else:
                gen_nodes(path+str(i), board, node_cache, nfo_cache, mapping_cache)
            

def get_next_card_list(board, mapping = None):
    if mapping == None:
        suits = suit_iso.get_suits(board)
        mapping = suit_iso.get_mapping(suits)
        board = suit_iso.morph_cards(board, mapping)

    n_suits = len(set(mapping))
    l = np.arange(n_suits*13)
    mult = np.ones(n_suits*13, dtype=np.float64)
    mult[(n_suits-1)*13:] = 4-n_suits+1
    #print l, board
    l = np.setxor1d(l, board)
    #for x in board:
    #    l[x] = -1
    return l, mult

def get_card_mask(c):
    return np.all(ct.itoc2 != c, axis=1)

def get_dead_cards_mask(cards):
    
    mask = get_card_mask(cards[0])
    for c in cards[1:]:
        mask = np.logical_and(get_card_mask(c), mask)
    return mask

def update_cfr(board, path, pov_seat, hw, prev_gamestate, node_cache, mapping_cache, nfo_cache=None):
    node_key = (tuple(board), path)
    try:
        n = node_cache[node_key]
    except KeyError:
        n = node(board, path, mapping_cache)
        node_cache[n] = n

    if prev_gamestate != n.gamestate:
        if n.gamestate >= 4:
            return None
            pass
        card_list, mult = get_next_card_list(board)
        l = len(card_list)
        new_hw = hw/(l-2)
        ev = np.zeros(defs.HANDS, dtype=np.float64)
        for c,m in zip(card_list, mult):
            new_hw *= get_dead_cards_mask(c)
            new_board = np.append(board, c)
            ev += update_cfr(new_board, path, pov_seat, new_hw, n.gamestate, node_cache, mapping_cache)*m
        return ev
    
    #get odds
    odds = n.calc_odds_from_regs()
    if n.seat != pov_seat:
        #calc new hws
        new_hw = [hw*odds[0], hw*odds[1], hw*odds[2]]
        ev = np.zeros(defs.HANDS, dtype=np.float64)
        #sum ev from branches
        for i in xrange(3):
            ev += update_cfr(board, path+str(i), pov_seat, new_hw[i], n.gamestate, node_cache, mapping_cache)
        return ev
    else:
        #get ev from branches
        ev = np.zeros((3,defs.HANDS), dtype=np.float64)
        #sum ev from branches
        for i in xrange(3):
            ev[i] = update_cfr(board, path+str(i), pov_seat, new_hw[i], n.gamestate, node_cache, mapping_cache)
        #update regrets
        n.update_regrets(ev)
        #odds = n.calc_odds_from_regs() #does this help?
        return (ev*odds).sum(axis=1)
    
if __name__ == "__main__":

    node_cache = {}
    nfo_cache = {}
    mapping_cache = {}

    

    bo = np.array((32,11,2), dtype=np.int8)
    bo = suit_iso.morph_cards_2(bo)
    gen_nodes("01", bo, node_cache, nfo_cache, mapping_cache)
    
    mapping2 = get_slot_mapping(bo)
    pdb.set_trace()
    c = {}
    t1 = time.time()
    rec_walk("01",c)
    t2 = time.time()

    print len(c)
    showdown = 0
    fold = 0
    other = 0
    for x in c.items():
        if x[1][1] == 4:
            showdown += 1
        elif x[1][1] >= 10:
            fold += 1
        else:
            other += 1
    print other, showdown, fold
    #rec_walk("",c)
    #t3 = time.time()
    #rec_walk("",c)
    #t4 = time.time()
    #print t2-t1, t3-t2, t4-t3

    print get_next_card_list(np.array((51,10,28)))
