
import sys



class game:
    def __init__(self, n_plr = 2, sb = 0.5, bb = 1.0, cap = 4):
        self.n_plr = n_plr
        self.sb = sb
        self.bb = bb
        self.cap = cap
        self.plr = []
        self.cur_hand = None


    def get_next_unique_state(self, us):
        r=c=f=None
        n_plr = us[2] + us[3]
        gs = us[0]
        bets = us[1]
        to_act = us[3]
        acted = us[2]
        next_act = us[4]
        if gs == 4:
            return (r,c,f)
        if us[1] < self.cap:
            r = (us[0], us[1]+1, 1, n_plr-1, (us[4] + 1)%n_plr)
        if to_act > 1:
            c = (gs, bets, acted+1, to_act-1, (next_act+1)%n_plr)
        else:
            c = (gs+1, 0, 0, n_plr, 0)
        if n_plr > 2:
            if to_act > 1:
                f = (gs, bets, acted, to_act-1, next_act)
            else:
                f = (gs+1, 0, 0, n_plr-1, 0)

        return (r, c, f)

    def get_next_unique_state2(self, us):
        r=c=f=None
        gs = us[0]
        to_act = us[1]
        utg = us[2]
        bets = list(us)[3:]
#        print bets
        n_plr = len(bets)
        bets_max = bets[-1]
        
        if gs == 4:
            return (r,c,f)
        if bets_max < self.cap: #raise
            r = (gs, n_plr-1, (utg-1)%n_plr)+tuple(bets[1:]+[bets_max+1])
        if to_act > 1:
            c = (gs, to_act-1, (utg-1)%n_plr)+tuple(bets[1:]+[bets_max])
        else:
            c = (gs+1, n_plr, 0)+tuple([0]*n_plr)
        if n_plr > 2:
            if to_act > 1:
                new_utg = 0
                if utg != 0:
                    new_utg = utg-1
                f = (gs, to_act-1, new_utg)+tuple(bets[1:])
            else:
                f = (gs+1, n_plr-1,0)+tuple([0]*(n_plr-1))
        return (r,c,f)

    def get_all_unique_states(self):
        def add_state(states, us):
            if us in states:
                return
            states[us] = 1
            next_states = self.get_next_unique_state(us)
            #print next_states
            for s in next_states:
                if s != None:
                    add_state(states, s)
                    
                    

        if self.n_plr == 2:
            next_to_act = 1
            initial_state = (0, 2,1,0.5,1)
        else:
            next_to_act = 2
            initial_state = (0,self.n_plr, self.n_plr-2)+tuple([0]*(self.n_plr-2))+(0.5,1)
        states = {}
        initial_state = (0, 1, 0, self.n_plr, next_to_act)
        
        add_state(states, initial_state)
        return states

    class hand:
        def __init__(self, n_plr = 2, sb = 0.5, bb = 1.0, cap = 4, players = []):
            self.sb = sb
            self.bb = bb
            self.cap = cap
            self.pot = sb + bb
            self.np_start = n_plr
            self.np_now = n_plr
            if players == []:
                self.plr = [player()]*n_plr
            else:
                self.plr = players
            self.bets = [-1.0]*4
            self.bets[0] = 1.0
            self.acts = [""]*4
            self.to_act = 0
            self.gamestate = 0
            if n_plr == 2:
                self.plr[1].bets = 0.5
                self.plr[0].bets = 1.0
                self.to_act = 1
            else:
                self.plr[0].bets = 0.5
                self.plr[1].bets = 1.0
                self.to_act = 2
                
        def add_actions(self, actions):
            gs = self.gamestate
            p = self.plr
            for act in actions:
                assert gs < 4
                if act == "0":
                    assert self.bets[gs] <= cap
                    self.bets[gs] += 1
                #print self.to_act, gs, p[self.to_act].acts 
                    if p[self.to_act].acts[gs][0] == "":
                        p[self.to_act].fl_act[gs][0] = 0
                        p[self.to_act].fl_act[gs][1] = 0
                    else:
                        p[self.to_act].fl_act[gs][1] = 0
                    p[self.to_act].acts[gs][0] += "0"
                    self.acts[gs] += "0"
                    p[self.to_act].acts[gs][1] += 1
                #print p[self.to_act].bets, self.bets[gs]
                    add_bets = (self.bets[gs] - p[self.to_act].bets[gs])*(1+gs/2)
                    self.pot += add_bets
                    #p[self.to_act]["rmoney"][gs] += add_bets
                    p[self.to_act].bets[gs] = self.bets[gs]
                    self.to_act = (self.to_act + 1)%self.np_now
                
                elif act == "1":
                    self.acts[gs] += "1"
                    if p[self.to_act].acts[gs][0] == "":
                        p[self.to_act].fl_act[gs][0] = 1
                        p[self.to_act].fl_act[gs][1] = 1
                    else:
                        p[self.to_act].fl_act[gs][1] = 1

                    p[self.to_act].acts[gs][0] += "1"
                    p[self.to_act].acts[gs][2] += 1
                    add_bets = (self.bets[gs] - p[self.to_act].bets[gs])*(1+gs/2)
                    self.pot += add_bets
                    #p[self.to_act]["cmoney"][gs] += add_bets
                #self.pot += (self.bets[gs] - p[self.to_act].bets[gs])
                    p[self.to_act].bets[gs] = self.bets[gs]
                    self.to_act = (self.to_act + 1)%self.np_now
                elif act == "2":
                    self.acts[gs] += "2"
                    if p[self.to_act].acts[gs][0] == "":
                        p[self.to_act].fl_act[gs][0] = 2
                        p[self.to_act].fl_act[gs][1] = 2
                    else:
                        p[self.to_act].fl_act[gs][1] = 2

                    p[self.to_act].acts[gs][0] += "2"
                    p[self.to_act].acts[gs][3] += 1
                    folded_player = p.pop(self.to_act)
                    p.append(folded_player)
                #if p[self.to_act].acts[gs][3] != 0:
                #    self.to_act = 0
                    self.np_now -=1
                    self.to_act = (self.to_act)%self.np_now
            #print  p[self.to_act].bets[gs], self.bets[gs], p[self.to_act].acts[gs][0], "jee"
                if p[self.to_act].bets[gs] == self.bets[gs] and p[self.to_act].acts[gs][0] != "":
                #GAMESTATE CHANGE
                    self.gamestate += 1
                    gs = self.gamestate
                    if gs < 4:
                        self.to_act = 0
                        self.bets[gs] = 0.0



        def get_available_actions(self):
            if self.gamestate >= 4 or self.np_now < 2:
                return (False, False, False, "SHOWDOWN")
            if self.np_now < 2:
                return (False, False, False, "NO PLAYERS")
            ap = self.to_act
            p = self.plr[ap]
            gs = self.gamestate
            h_bets = self.bets[gs]
            p_bets = p.bets[gs]
            r = c = f = True
            if h_bets == h["cap"]:
                r = False
        #if p_bets == 0:
        #    f = False
            return (r,c,f, "OK")

        def get_unique_state_from_hand(self, h):
            gs = self.gamestate
            if gs >= 4 or self.np_now < 2:
                return (-1, -1, -1, -1)
            to_act = 0
            acted = 0
            for i in xrange(self.np_now):
                p = self.plr[i]
                if p.acts[gs][0] == "" or p.bets[gs] < h.bets[gs]:
                    to_act += 1
                else:
                    acted += 1
            #print "PLR", p
            return (h.bets[gs], acted, to_act, self.to_act)

    class player:
        def __init__(self):
            self.stack = 1000
            self.bets = [0]*4
            self.seat = 0
            self.fl_act = [[-1,-1]]*4
            
            


if __name__ == "__main__":
    g = game(int(sys.argv[1]))
    states = g.get_all_unique_states()
    
    s = states.items()
    s.sort()
    for x in s:
        print x[0]

    print len(states)
