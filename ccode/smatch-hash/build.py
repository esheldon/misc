from fabricate import *
import sys, os
import optparse

import glob


parser = optparse.OptionParser()
# make an options list, also send to fabricate
optlist=[optparse.Option('-p','--prefix',default=sys.exec_prefix,
                         help="where to install"),
         optparse.Option('-d','--debug',action="store_true",
                         help="turn on debugging (assert)"),
         optparse.Option('--pixelof',action="store_true",
                         help="make the pixelof executable"),
         optparse.Option('--intersect',action="store_true",
                         help="make the intersect executable")]
parser.add_options(optlist)

options,args = parser.parse_args()
prefix=os.path.expanduser( options.prefix )

CC='gcc'

LINKFLAGS=['-lm']

CFLAGS=['-std=gnu99','-Wall','-Werror','-O2']
#CFLAGS=['-std=gnu99','-Wall','-Werror','-O2','-DHASH_FUNCTION=HASH_SFH']
# use this when testing hash functions
#CFLAGS=['-std=gnu99','-Wall','-O2','-DHASH_EMIT_KEYS=3']
# redirect ./smatch .. 3> keystats.bin
# then run the keystats program in the /tests subdir of the uthash distro

if not options.debug:
    CFLAGS += ['-DNDEBUG']



sources = ['alloc','healpix','matchstack','point_hash','ptrstack','stack',
           'cat','files','smatch']

programs = [{'name':'smatch', 'sources':sources}]

if options.pixelof:
    p_sources = ['alloc','healpix','stack', 'pixelof']
    programs.append({'name':'pixelof','sources':p_sources})
if options.intersect:
    p_sources = ['alloc','healpix','stack', 'intersect']
    programs.append({'name':'intersect','sources':p_sources})


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

# send options so it won't crash on us
main(extra_options=optlist)

