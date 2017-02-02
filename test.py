import zpoker
import plrmodel
import lzp

p = plrmodel.plrmodel("megagame")
p.gen_plrmodel_tree_for_all_us()

p.zp.load_diffs_for_all_types()
for us in p.iter_all_unique_states():
    if us.gamestate < 4 and us.id >= 2:
        print "expanding", us.state
        if us.gamestate == 0:
            us.expand_type_max(0,us.gamestate)
        us.expand_type_max(1,6)
        us.expand_type_max(1,5)        
        us.expand_type_max(1,4)
        us.expand_type_max(1,3)
        us.expand_type_max(1,2)
        #us.expand_type_max(1,1)
