import sys
import time
import select
import pdb

import zpoker
import plrmodel


gamedir = sys.argv[3]
if gamedir[-1] != "/":
    gamedir+="/"
p = plrmodel.plrmodel(gamedir)

print "loading bb model", sys.argv[4]
p.load_model(gamedir+sys.argv[4],0)
print "loading sb model", sys.argv[5]
p.load_model(gamedir+sys.argv[5],1)


seat = int(sys.argv[1])
mode = int(sys.argv[2])
p.zp.load_jbs()
print "reseting hand ev"
p.reset_hand_ev(seat)

t = plrmodel.traverser(p.zp)

t.c_data.plr[seat].hand_odds_from = 2
t.c_data.plr[seat].update_hand_odds_every = 10
t.c_data.plr[seat].ev_decay = 0.99999
t.set_start_unique_state(p.root_state)
t.iter_count = [0]*p.n_plr
t.iter_count[seat] = 1
last_time = time.time()

if mode == 1:
    print "setting odds from avg"
    p.set_hand_odds_from_avg()

x = 0
avg_change = 0
last_avg = [[],[]]

while True:
    x+=1
    if select.select([sys.stdin,],[],[],0.0)[0]:
        print "getting input"
        inp = raw_input()
        if inp == "break":
            break
        elif inp == "evsim":
            n_iter = int(raw_input("iterations?"))
            p.exec_ev_calc_one_hand(n_iter)
            s = p.browser_get_situ("")
            p.print_situ(s)
            s = p.browser_get_situ("r")
            p.print_situ(s)
        elif inp == "debug":
            pdb.set_trace()
    t.traverse_complete()
    new_time = time.time()
    print x, avg_change, new_time-last_time
    last_time = new_time
    avg_change = 0
    for pl in xrange(p.n_plr):
        print t.c_data.plr[pl].odds_change,
        print t.c_data.plr[pl].positive_regs,
        print t.c_data.plr[pl].hands_updated,
        if t.c_data.plr[pl].hands_updated > 0:
            print t.c_data.plr[pl].odds_change/t.c_data.plr[pl].hands_updated,
            print t.c_data.plr[pl].positive_regs/t.c_data.plr[pl].hands_updated
        else:
            print
        t.c_data.plr[pl].odds_change = 0
        t.c_data.plr[pl].positive_regs = 0
        t.c_data.plr[pl].hands_updated = 0
    avg_ev = t.get_avg_hand_ev()

    for i,ev in enumerate(avg_ev):
        #print i,x, last_avg[i]
        last_avg[i].append(ev)
        if len(last_avg[i]) > 50:
            last_avg[i].pop(0)
        print i, sum(last_avg[i])/50.0
        
    #t.print_avg_hand_ev()
    t.reset_avg_hand_ev()

