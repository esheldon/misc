import os
from glob import glob
import numpy as np
from numpy import array,zeros,ones,where,arange,linspace,logspace, \
    sqrt, exp, cos, sin, tanh, arctanh, log, log10, median, \
    diag
tpng=os.path.expandvars('$WEBDIR/tmp/plots/tmp.png')
tsvg=os.path.expandvars('$WEBDIR/tmp/plots/tmp.svg')
teps=os.path.expandvars('$WEBDIR/tmp/plots/tmp.eps')
try:
    import esutil as eu
    from esutil.numpy_util import ahelp, aprint, where1, between
    from esutil.misc import colprint, ptime
    from esutil.stat import get_stats, print_stats
except ImportError:
    pass


try:
    import biggles
    from biggles import plot, plot_hist
except ImportError:
    pass

try:
    from plotting import plot_hist2d
except ImportError:
    pass

try:
    import fitsio
except ImportError:
    pass

try:
    import ngmix
except ImportError:
    pass


try:
    import nsim
except ImportError:
    pass


# python 3
try:
    from importlib import reload
except ImportError:
    pass

