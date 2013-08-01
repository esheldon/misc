"""
build the tests for
"""

from fabricate import *
import sys, os
import optparse
import glob

parser = optparse.OptionParser()
# make an options list, also send to fabricate
optlist=[optparse.Option('--prefix','-p',default=sys.exec_prefix,help="where to install")]
         
parser.add_options(optlist)

options,args = parser.parse_args()
prefix=os.path.expanduser( options.prefix )

CC='gcc'

# -lrt is only needed for the timing stuff
LINKFLAGS=['-lm','-static']

CFLAGS=['-std=gnu99','-Wall','-Werror','-O2']


sources = ['gsim-ring-mcmc',
           'config',
           'gmix_em',
           'mtx2',
           'gmix_mcmc_config',
           'gmix_mcmc',
           'shape',
           'prob',
           'dist',
           'image',
           'image_rand',
           'gauss2',
           'gmix',
           'gmix_image',
           'gmix_image_rand',
           'mca',
           'randn',
           'jacobian',
           'result',
           'obs',
           'gsim_ring',
           'gsim_ring_config',
           'object',
           'fileio']


programs = [{'name':'gsim-ring-mcmc', 'sources':sources}]
prog_installs = [(prog['name'],'bin') for prog in programs]

configs = glob.glob('./config/*.cfg')
config_installs = [ (conf,'share') for conf in configs]

install_targets = prog_installs + config_installs

def build():
    compile()
    link()

def compile():
    for prog in programs:
        for source in prog['sources']:
            run(CC, '-c', '-o',source+'.o', CFLAGS, source+'.c')

def link():
    for prog in programs:
        objects = [s+'.o' for s in prog['sources']]
        run(CC,'-o', prog['name'], objects,LINKFLAGS)

def install():
    import shutil

    # make sure everything is built first
    build()

    for target in install_targets:
        (name,subdir) = target
        subdir = os.path.join(prefix, subdir)
        if not os.path.exists(subdir):
            os.makedirs(subdir)

        dest=os.path.join(subdir, os.path.basename(name))
        sys.stdout.write("install: %s\n" % dest)
        shutil.copy(name, dest)


def clean():
    autoclean()


main(extra_options=optlist)
