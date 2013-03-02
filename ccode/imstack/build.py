"""
Build the admom test program
"""
from fabricate import *
import sys, os
import optparse

import glob

parser = optparse.OptionParser()
# make an options list, also send to fabricate
optlist=[optparse.Option('--prefix','-p',default=sys.exec_prefix,help="where to install"),
         optparse.Option('--dbg',action="store_true",help="turn on debugging (assert)")]
         
parser.add_options(optlist)

options,args = parser.parse_args()
prefix=os.path.expanduser( options.prefix )

CC='gcc'

LINKFLAGS=['-lcfitsio','-lm']

CFLAGS=['-std=gnu99','-Wall','-Werror','-O2']
if not options.dbg:
    CFLAGS += ['-DNDEBUG']

sources=['imstack', 'image', 'image_fits']
programs=[{'name':'imstack','sources':sources}]
install_targets = [(prog['name'],'bin') for prog in programs]

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
        run(CC,'-o',prog['name'],objects,LINKFLAGS)

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

# send options so it won't crash on us
main(extra_options=optlist)

