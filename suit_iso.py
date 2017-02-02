import card_table as ct
import copy
import numpy as np

def sort_key(card):
    return card[1]

def sort_flop(cards):
    #cards[:3] =sorted(cards[:3], key=lambda x: x%13)
    flop_order = np.argsort(cards%13)
    sorted_flop = cards[np.concatenate((flop_order, np.arange(5)[3:len(cards)]))]
    return sorted_flop
    
def morph_flop(suits):
    #mapping = [None]*4
    mapping = np.zeros(4, dtype=np.int16)-1
    s = copy.copy(suits[:3])
    if s[0]==s[1]==s[2]:
        mapping[s[0]] = 0         
    elif s[0]!=s[1] and s[1]!=s[2] and s[0]!=s[2]:
        #s.sort()
        assert len(s)==3
        for i,x in enumerate(s):
            #print i,x
            mapping[x] = i
    else:
        if s[0] == s[1]:
            mapping[s[0]] = 0
            mapping[s[2]] = 1
        elif s[0] == s[2]:
            mapping[s[0]] = 0
            mapping[s[1]] = 1
        elif s[1] == s[2]:
            mapping[s[1]] = 0
            mapping[s[0]] = 1
        else:
            sys.exit()
    return mapping

def morph_turn(suits, old_mapping):
    s = suits
    mapping = np.copy(old_mapping)
    if mapping[s[3]] == -1:
        new_suit = len(set(mapping))-1
        mapping[s[3]] = new_suit
    return mapping

def morph_river(suits, old_mapping):
    s = suits[4]
    mapping = np.copy(old_mapping)
    if mapping[s] == -1:
        new_suit = len(set(mapping))-1
        mapping[s] = new_suit
    return mapping

def get_mapping(suits):
    mapping = morph_flop(suits)
    if len(suits) > 3:
        mapping = morph_turn(suits,mapping)
        if len(suits) > 4:
            mapping = morph_river(suits,mapping)
    next_suit = len(set(mapping))-1

    i = 0
    while i < len(mapping):
        if mapping[i] == -1:
            mapping[i] = next_suit
        i+=1
    
    return mapping

def get_suits(cards):
    return cards/13
#suits = []
    
#   for c in cards:
#        suits.append(c/13)
#    return suits

def morphed_card(c, mapping):
    s = c/13
    new_s = mapping[s]
    return c%13+new_s*13
    
def morph_cards(old_cards, mapping):
    cards = np.copy(old_cards)
    for i in xrange(len(cards)):
        cards[i] = morphed_card(cards[i], mapping)
    return cards

def morph_cards_2(cards):
    s = get_suits(cards)
    mapping = get_mapping(s)
    return morph_cards(cards, mapping)

def get_mapping_from_cards(cards):
    return get_mapping(get_suits(cards))

if __name__ == "__main__":
    combo_dic = {}
    flop_dic = {}
    i = 0
    i1 = 51
    while i1 >= 0:
        i2 = i1-1
        while i2 >= 0:
            i3 = i2-1
            while i3 >= 0:
                print "FLOP", i, len(combo_dic)
                fc = np.array((i1,i2,i3), dtype=np.int8)
                fc = sort_flop(fc)
                fs = get_suits(fc)
                fm = morph_flop(fs)
                fc = morph_cards(fc, fm)
                fc.sort()
                try:
                    flop_dic[tuple(fc)] += 1
                except:
                    flop_dic[tuple(fc)] = 1
                print len(flop_dic)
                i4 = 51
                while i4 >= 0:
                    if i4 == i1 or i4 == i2 or i4 == i3:
                        i4-=1
                        continue
                    i5 = 51
                    while i5 >= 0:
                        if i5 == i1 or i5 == i2 or i5 == i3 or i5 == i4:
                            i5-=1
                            continue

                        cards = [i1,i2,i3,i4,i5]
                        sort_flop(cards)
                        suits = get_suits(cards)
                        mapping = get_mapping(suits)
                        #print cards, suits, mapping
                        morphed = morph_cards(cards, mapping)
                        morphed[:3] = sorted(morphed[:3])
                        key = tuple(morphed)
                        try:
                            combo_dic[key] += 1
                        except:
                            combo_dic[key] = 1
                        i+=1
                        i5-=1
                    i4-=1
                i3-=1
            i2-=1
        i1-=1
