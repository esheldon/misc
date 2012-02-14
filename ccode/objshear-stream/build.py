from fabricate import *
import sys, os
import optparse

import glob


parser = optparse.OptionParser()
# make an options list, also send to fabricate
optlist=[optparse.Option('--prefix','-p',default=sys.exec_prefix,help="where to install"),
         optparse.Option('--with-truez',action="store_true",default=False,help="use true z for sources"),
         optparse.Option('--sdssmask',action="store_true",default=False,help="check quadrants from sdss mask"),
         optparse.Option('--hdfs',action="store_true",default=False,help="Files are in hdfs"),
         optparse.Option('--no-cache-output',action="store_true",default=False,help="keep outputs in memory"),
         optparse.Option('--noopt',action="store_true",help="turn off compiler optimizations"),
         optparse.Option('-d','--debug',action="store_true",help="turn on debugging (assert)"),
         optparse.Option('--stream',action="store_true",help="take sources from stdin as text"),
         optparse.Option('--test',action="store_true",help="compile tests")]
parser.add_options(optlist)

options,args = parser.parse_args()
prefix=os.path.expanduser( options.prefix )

CC='gcc'

LINKFLAGS=['-lm']

CFLAGS=['-std=c99','-Wall','-Werror']
if not options.noopt:
    CFLAGS += ['-O2']
if not options.debug:
    CFLAGS += ['-DNDEBUG']

if options.with_truez:
    CFLAGS += ['-DWITH_TRUEZ']

if options.sdssmask:
    CFLAGS += ['-DSDSSMASK']

if options.no_cache_output:
    CFLAGS += ['-DNO_CACHE_OUTPUT']

test_programs = [{'name':'test/test-healpix','sources':['healpix','stack','test/test-healpix']},
                 {'name':'test/test-healpix-brute',
                  'sources':['healpix','gcirc','stack','Vector','sort','histogram','test/test-healpix-brute']},
                 {'name':'test/test-i64stack','sources':['stack','test/test-i64stack']},
                 {'name':'test/test-interp','sources':['interp','Vector','test/test-interp']},
                 {'name':'test/test-sdss-survey','sources':['test/test-sdss-survey','sdss-survey']},
                 {'name':'test/test-sdss-quad','sources':['test/test-sdss-quad','sdss-survey']},
                 {'name':'test/test-sdss-quad-check','sources':['test/test-sdss-quad-check','sdss-survey']},
                 {'name':'test/test-config','sources':['config','test/test-config']},
                 {'name':'test/test-source',
                  'sources':['source','sort','healpix','histogram','stack','Vector','test/test-source']},
                 {'name':'test/test-lens','sources':['lens','cosmo','test/test-lens']},
                 {'name':'test/test-lens-loadhpix','sources':['lens','cosmo','stack','healpix','test/test-lens-loadhpix']},
                 {'name':'test/test-cosmo','sources':['cosmo','test/test-cosmo']},
                 {'name':'test/test-sort','sources':['sort','Vector','test/test-sort']},
                 {'name':'test/test-hist','sources':['histogram','Vector','test/test-hist']}]

sobjshear_sources = ['config', 'stack', 'Vector','source','lens','cosmo','healpix',
                     'shear','lensum','histogram','tree','interp','urls',
                     'sobjshear']
redshear_sources = ['healpix','cosmo','tree','stack','lens','lensum','config','urls','Vector',
                    'redshear']


if options.sdssmask:
    sobjshear_sources += ['sdss-survey']
    redshear_sources += ['sdss-survey']

if options.hdfs:
    sobjshear_sources += ['hdfs_lines']
    redshear_sources += ['hdfs_lines']
    LINKFLAGS += ['-L${HADOOP_HOME}/libhdfs','-lhdfs']
    CFLAGS += ['-DHDFS','-I${HADOOP_HOME}/src/c++/libhdfs']

programs = [{'name':'sobjshear', 'sources':sobjshear_sources},
            {'name':'redshear', 'sources':redshear_sources}]

if options.test:
    programs += test_programs

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

