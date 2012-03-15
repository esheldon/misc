import IPython.ipapi
ip = IPython.ipapi.get()

def main():
    # We can set options through ip.options
    o = ip.options

    # Set up the prompt.  Choose your favorite color
    if o.colors == 'LightBG':
        o.prompt_in1= r'\C_Blue>>> '
    else:
        o.prompt_in1= r'\C_Green>>> '
    o.prompt_out= r''

    # don't put annoying extra newline between prompts
    o.separate_in = ''
    o.confirm_exit = 0


    #
    # now import some useful modules
    #

    # for quick timing of commands
    ip.ex("import time")

    ip.ex("import os")
    ip.ex("import sys")

    # import numerical python and 
    # the often used where function
    try:
        ip.ex("import numpy")
        ip.ex("from numpy import array")
        ip.ex("from numpy import zeros")
        ip.ex("from numpy import ones")
        ip.ex("from numpy import where")
        ip.ex("from numpy import arange")
        ip.ex("from numpy import linspace")
        ip.ex("from numpy import sqrt")
        ip.ex("from numpy import exp")
        ip.ex("from numpy import cos")
        ip.ex("from numpy import sin")
        ip.ex("from numpy import log")
        ip.ex("from numpy import log10")
    except:
        print "Could not import numpy"

    try:
        ip.ex("import fitsio")
    except:
        print "Could not import fitsio"


    try:
        ip.ex("import zphot")
    except:
        print "Could not import zphot"

    try:
        ip.ex("import cosmology")
    except:
        print "Could not import cosmology"

    try:
        ip.ex("import admom")
    except:
        print "Could not import admom"
    try:
        ip.ex("import images")
    except:
        print "Could not import images"
    try:
        ip.ex("import fimage")
    except:
        print "Could not import fimage"

    # For plotting
    try:
        ip.ex("import biggles")
        ip.ex("from biggles import FramedPlot")
        ip.ex("from biggles import PlotKey")
        ip.ex("from biggles import Curve")
        ip.ex("from biggles import Points")
    except:
        print "Could not import biggles"


    # esutil has lots of tools for working
    # with data
    try:
        ip.ex("import esutil as eu")
        ip.ex("from esutil.numpy_util import ahelp")
        ip.ex("from esutil.numpy_util import aprint")
        ip.ex("from esutil.numpy_util import where1")
        ip.ex("from esutil.misc import colprint")
        ip.ex("from esutil.misc import ptime")
        ip.ex("from esutil import sfile")
        ip.ex("from esutil import coords")
    except:
        print "Could not import esutil"


    try:
        ip.ex("import sdsspy")
        ip.ex("from sdsspy import yanny")
    except:
        print "Could not import sdsspy"

    try:
        ip.ex("import es_sdsspy")
    except:
        print "Could not import es_sdsspy"


    try:
        ip.ex("from esutil import oracle_util as ou")
        try:
            ip.ex("oc = ou.Connection()")
        except:
            pass
    except:
        pass
        #print "Could not import oracle_util"


    try:
        ip.ex("import des")
        ip.ex("from des import util as du")
    except:
        print "Could not import des"

    try:
        ip.ex("import lensing")
    except:
        print "Could not import lensing"
 

    try:
        ip.ex("import deswl")
    except:
        print "Could not import deswl"
        
    try:
        ip.ex("import columns")
        ip.ex("from columns import Columns")
    except:
        print "Could not import columns"

    try:
        ip.ex("import pgnumpy")
        ip.ex("bpg=pgnumpy.connect('dbname=boss')")
        ip.ex("dpg=pgnumpy.connect('dbname=des')")
    except:
        print "Could not import pgnumpy"

main()
