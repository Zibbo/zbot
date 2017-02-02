import time
import select
import sys
import plrmodel
import pdb
import worker

gamedir = sys.argv[1]
model_name = sys.argv[2]

p = plrmodel.plrmodel(gamedir)
p.model_name = model_name
p.gen_plrmodel_tree_all_us()

w = worker.worker(p.root_state,1)

sys.exit()
#p.zp.load_jbs_for_simu()
#p.zp.load_jbs()

#p.zp.load_hand_slots(0)
#print p.zp.get_random_jbs_for_simu()
#sys.exit()
for state in p.states_dict.values():
    if state.gamestate == 4:
        continue
    print "dividing node", state.id, state.gamestate
    state.divide_all_same_type(11, -1)
    state.divide_all_same_type(12, -1)
    state.divide_all_same_type(12, -1)

    #if state.gamestate == 1:
    #    state.divide_all_same_type(20,1)
    #    state.divide_all_same_type(20,1)
    #    state.divide_all_same_type(20,1)
    #    state.divide_all_same_type(20,1)
    #    state.divide_all_same_type(20,1)
    #    state.divide_all_same_type(20,1)
    #    state.divide_all_same_type(20,1)

    state.divide_all_hands()
    state.divide_all_hands()
    state.divide_all_hands()
    state.divide_all_hands()
    state.divide_all_hands()
    state.divide_all_hands()
    if state.gamestate != 0:
        state.divide_all_hands()
p.reset_hand_odds()
p.reset_all_hand_data()
p.set_preblind_raise_all()
cont = 1
x = 0
avg_change = 0
last_time = time.time()


t = plrmodel.traverser(p.zp)
t.set_start_unique_state(p.root_state)
t.c_data.plr[0].update_hand_odds_every = 10
t.c_data.plr[1].update_hand_odds_every = 10
t.c_data.plr[0].regs_decay = 0.999
t.iter_count = [1,1]

while cont:
    try:
        if select.select([sys.stdin,],[],[],0.0)[0]:
            print "getting input"
            inp = raw_input()
            print "input:", inp
            if inp == "browse":
                p.browser()
            elif inp == "break":
                cont = 0
            elif inp == "debug":
                pdb.set_trace()
            else:
                print "unidentified input", inp
        x += 1
        #board_data, hand_data, handval_data = p.zp.get_random_jbs(-1)
        #print "starting simulate"

        #avg_change += p.exec_simulate(board_data, hand_data, handval_data)
        #print "done"
        t.traverse()

        if not x%1000:
            new_time = time.time()
            print x, avg_change, new_time-last_time
            
            avg_change = 0
            visits_per_second = 0
            for pl in xrange(p.n_plr):
                print t.c_data.plr[pl].odds_change,
                print t.c_data.plr[pl].positive_regs,
                print t.c_data.plr[pl].hands_updated,
                if t.c_data.plr[pl].hands_updated > 0:
                    print t.c_data.plr[pl].odds_change/t.c_data.plr[pl].hands_updated,
                    print t.c_data.plr[pl].positive_regs/t.c_data.plr[pl].hands_updated
                    
                else:
                    print
                visits_per_second += t.c_data.plr[pl].hands_updated/(new_time-last_time)
                t.c_data.plr[pl].odds_change = 0
                t.c_data.plr[pl].positive_regs = 0
                t.c_data.plr[pl].hands_updated = 0
                
            last_time = new_time
            print "visits_per_second", visits_per_second
        if not x%100000:
            p.model_name = model_name + "_" +str(x)
            p.save_model(overwrite=False)
            
    except KeyboardInterrupt:
        cont = 0

