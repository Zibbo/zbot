import ConfigParser
import sys

gamedir = sys.argv[1] + "/"


config = ConfigParser.RawConfigParser()

# When adding sections or items, add them in the reverse order of
# how you want them to be displayed in the actual file.
# In addition, please note that using RawConfigParser's and the raw
# mode of ConfigParser's respective set functions, you can assign
# non-string values to keys internally, but will receive an error
# when attempting to write to a file or when you get it in non-raw
# mode. SafeConfigParser does not allow such assignments to take place.
config.add_section('n_types')
config.set('n_types', 'preflop', '169')
config.set('n_types', 'flop', '8192')
config.set('n_types', 'turn', '8192')
config.set('n_types', 'river', '1024')
config.add_section('n_rtypes')
config.set('n_rtypes', 'preflop', '32')
config.set('n_rtypes', 'flop', '64')
config.set('n_rtypes', 'turn', '128')
config.set('n_rtypes', 'river', '3')
config.add_section('gauss_width')
config.set('gauss_width', 'preflop', '0.1')
config.set('gauss_width', 'flop', '0.1')
config.set('gauss_width', 'turn', '0.1')
config.set('gauss_width', 'river', '0.1')

#config.set('Section1', 'bool', 'true')
#config.set('Section1', 'float', '3.1415')
#config.set('Section1', 'baz', 'fun')
#config.set('Section1', 'bar', 'Python')
#config.set('Section1', 'foo', '%(bar)s is %(baz)s!')

# Writing our configuration file to 'example.cfg'
with open(gamedir + 'config.cfg', 'wb') as configfile:
    config.write(configfile)


