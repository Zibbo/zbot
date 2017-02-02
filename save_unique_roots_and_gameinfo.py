import plrmodel
import lzp

p = plrmodel.plrmodel("biggame")

f = file("gameinfo", "wb")
lzp.save_gameinfo(p.zp.i, f.fileno())
f.close()

f = file("unique_roots", "wb")
lzp.save_unique_roots( p.root_state.unique_root_node, f.fileno(), 0)
f.close()

