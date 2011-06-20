from fabricate import *
import sys, os
import optparse

import glob


parser = optparse.OptionParser()
# make an options list, also send to fabricate
optlist=[optparse.Option('--prefix','-p',default=sys.exec_prefix)]
parser.add_options(optlist)

options,args = parser.parse_args()
prefix=os.path.expanduser( options.prefix )

CC='gcc'


CFLAGS=['-std=c99','-O2']
LINKFLAGS=['-lm']

hpix_sources=['healpix','stack']

#programs = [{'name':'test-healpix','sources':hpix_sources+['test-healpix']},
#            {'name':'test-i64stack','sources':['stack','test-i64stack']},
#            {'name':'test-hist','sources':['histogram','Vector','test-hist']}]
programs = [{'name':'test/test-healpix','sources':hpix_sources+['test/test-healpix']},
            {'name':'test/test-i64stack','sources':['stack','test/test-i64stack']},
            {'name':'test/test-hist','sources':['histogram','Vector','test/test-hist']}]



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

