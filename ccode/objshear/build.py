from fabricate import *
import sys, os
import optparse

import glob


parser = optparse.OptionParser()
# make an options list, also send to fabricate
optlist=[optparse.Option('--prefix','-p',default=sys.exec_prefix,help="where to install"),
         optparse.Option('--with-truez',default=False,help="use true z for sources"),
         optparse.Option('--noopt',action="store_true",help="turn off compiler optimizations"),
         optparse.Option('-d','--debug',action="store_true",help="turn on debugging (assert)"),
         optparse.Option('--test',action="store_true",help="compile tests")]
parser.add_options(optlist)

options,args = parser.parse_args()
prefix=os.path.expanduser( options.prefix )
with_truez=options.with_truez
debug=options.debug
noopt=options.noopt

CC='gcc'

LINKFLAGS=['-lm']

CFLAGS=['-std=c99','-Wall','-Werror']
if not noopt:
    CFLAGS += ['-O2']
if not debug:
    CFLAGS += ['-DNDEBUG']

if with_truez:
    CFLAGS += ['-DWITH_TRUEZ']

test_programs = [{'name':'test/test-healpix','sources':['healpix','stack','test/test-healpix']},
                 {'name':'test/test-healpix-brute',
                  'sources':['healpix','gcirc','stack','Vector','sort','histogram','test/test-healpix-brute']},
                 {'name':'test/test-i64stack','sources':['stack','test/test-i64stack']},
                 {'name':'test/test-interp','sources':['interp','Vector','test/test-interp']},
                 {'name':'test/test-config','sources':['config','test/test-config']},
                 {'name':'test/test-source',
                  'sources':['source','sort','healpix','histogram','stack','Vector','test/test-source']},
                 {'name':'test/test-lens','sources':['lens','cosmo','test/test-lens']},
                 {'name':'test/test-cosmo','sources':['cosmo','test/test-cosmo']},
                 {'name':'test/test-sort','sources':['sort','Vector','test/test-sort']},
                 {'name':'test/test-hist','sources':['histogram','Vector','test/test-hist']}]

programs = [{'name':'objshear',
             'sources':['config','lens','lensum','source','cosmo',
                        'healpix','gcirc','stack','Vector','sort','histogram',
                        'shear',
                        'objshear']}]

if options.test:
    programs += test_programs

install_targets = [(prog['name'],'bin') for prog in programs]
install_targets += [('objshear.table','ups')]


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
        run(CC,LINKFLAGS,'-o', prog['name'], objects)

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

