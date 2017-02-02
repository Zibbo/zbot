from ctypes import *
import numpy as np

import datatypes as dt


lib = cdll.LoadLibrary("libplrmodel.dylib")
lib.precalc_conversions()

# FUNCTION DEFENITIONS

lib.generate_empty_plrmodel_tree.restype = POINTER(dt.plrmodel_node)

lib.divide_all_same_type.argtypes = [POINTER(dt.plrmodel_node), c_int, c_int]

lib.walk_all_situs.argtypes = [POINTER(dt.plrmodel_node), c_void_p]
lib.walk_all_situs.restype = POINTER(dt.situ_list)

lib.simulate.argtypes = [POINTER(dt.gameinfo), POINTER(dt.unique_root), POINTER(c_short), POINTER(c_short), POINTER(c_uint), POINTER(dt.simu_data), c_int]
lib.simulate.restype = c_double

lib.get_first_matching_situ.argtypes = [POINTER(dt.plrmodel_node), POINTER(dt.situ_type)]
lib.get_first_matching_situ.restype = POINTER(dt.plrmodel_node)

lib.free_situ_tmp_data.argtypes = [POINTER(dt.plrmodel_node)]

lib.solve_preflop.argtypes = [POINTER(dt.gameinfo), np.ctypeslib.ndpointer(dt.hand_hv_flop), POINTER(dt.unique_root), POINTER(POINTER(c_double)), POINTER(dt.situ_type), c_double, c_int]

lib.reset_hand_odds.argtypes = [POINTER(dt.situ)]
lib.randomize_hand_odds.argtypes = [POINTER(dt.situ)]
lib.set_hand_odds_from_avg.argtypes = [POINTER(dt.situ)]
lib.set_hand_odds_from_regs.argtypes = [POINTER(dt.situ)]
lib.reset_hand_odds_avg.argtypes = [POINTER(dt.situ)]
lib.reset_hand_regs.argtypes = [POINTER(dt.situ)]
lib.reset_hand_ev.argtypes = [POINTER(dt.situ)]
lib.reset_hand_visits.argtypes = [POINTER(dt.situ)]
lib.reset_hand_path_level.argtypes = [POINTER(dt.situ)]
lib.reset_hand_data.argtypes = [POINTER(dt.situ)]

lib.traverse_tree.argtypes = [POINTER(dt.traverse_data), POINTER(dt.traverse_plr_data), POINTER(dt.unique_root), POINTER(dt.situ_type), c_double]
lib.traverse_tree_complete.argtypes = [POINTER(dt.traverse_data), POINTER(dt.traverse_plr_data), POINTER(dt.unique_root), POINTER(dt.situ_type), c_double]

lib.update_regret_and_odds.argtypes = [POINTER(dt.plrmodel_node), POINTER(dt.traverse_data)]

lib.load_plrmodel_tree.argtypes = [c_int, POINTER(dt.unique_root)]
lib.load_plrmodel_tree.restype = POINTER(dt.plrmodel_node)

lib.save_plrmodel_tree.argtypes = [c_int, POINTER(dt.plrmodel_node)]
lib.free_plrmodel_tree.argtypes = [POINTER(dt.plrmodel_node)]

