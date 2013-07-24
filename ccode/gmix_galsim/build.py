"""
build the tests
"""

from fabricate import *
import sys, os
import glob
import optparse

parser = optparse.OptionParser()
optlist=[optparse.Option('-p','--prefix',
                         default=sys.exec_prefix,
                         help="where to install")]
parser.add_options(optlist)

options,args = parser.parse_args()
prefix=options.prefix

CC='gcc'

# -lrt is only needed for the timing stuff
LINKFLAGS=['-lcfitsio','-lm']

CFLAGS=['-std=gnu99','-Wall','-Werror','-O2']

sources=['gmix-galsim','gmix_image','gmix_image_fits',
         'gmix','gmix_mcmc','image','mca','gauss2',
         'image_rand','randn',
         'admom','shape','config']
objlist_sources=['gmix-galsim-objlist']
test_read_sources = ['test/test-read','image']

pstats_sources=['gmix-galsim-pstats']

install_progs = [{'name':'gmix-galsim', 'sources':sources},
                 {'name':'gmix-galsim-objlist', 'sources':objlist_sources},
                 {'name':'gmix-galsim-pstats', 'sources':pstats_sources}]
tests=[{'name':'test/test-read', 'sources':test_read_sources}]

programs = install_progs + tests

install_targets = [(prog['name'],'bin') for prog in install_progs]

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

def clean():
    autoclean()

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


main(extra_options=optlist)
