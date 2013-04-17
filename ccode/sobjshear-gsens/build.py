from fabricate import *
import sys, os
import optparse

import glob


parser = optparse.OptionParser()
# make an options list, also send to fabricate
optlist=[optparse.Option('--prefix','-p',
                         default=sys.exec_prefix,help="where to install"),
         optparse.Option('--hdfs',action="store_true",
                         default=False,help="Files are in hdfs"),
         optparse.Option('--noopt',action="store_true",
                         help="turn off compiler optimizations"),
         optparse.Option('--dbg',action="store_true",
                         help="turn on debugging (assert)")]
parser.add_options(optlist)

options,args = parser.parse_args()
prefix=os.path.expanduser( options.prefix )

CC='gcc'

LINKFLAGS=['-lm']

CFLAGS=['-std=gnu99','-Wall','-Werror']
#CFLAGS=['-std=gnu99','-Wall','-Werror']
if not options.noopt:
    CFLAGS += ['-O2']
if not options.dbg:
    CFLAGS += ['-DNDEBUG']



sobjshear_sources = ['sconfig', 'config', 'stack', 'Vector','source',
                     'lens','cosmo','healpix',
                     'shear','lensum','histogram','tree','interp','urls',
                     'sobjshear','sdss-survey']
redshear_sources = ['healpix','cosmo','tree','stack','lens','lensum',
                    'sconfig','config',
                    'urls','Vector',
                    'util',
                    'redshear','sdss-survey']



if options.hdfs:
    sobjshear_sources += ['hdfs_lines']
    redshear_sources += ['hdfs_lines']
    LINKFLAGS += ['-L${HADOOP_HOME}/libhdfs','-lhdfs']
    CFLAGS += ['-DHDFS','-I${HADOOP_HOME}/src/c++/libhdfs']

programs = [{'name':'sobjshear', 'sources':sobjshear_sources},
            {'name':'redshear', 'sources':redshear_sources}]


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
        run(CC,'-o', prog['name'], objects, LINKFLAGS)

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

