# make the test program
from fabricate import *
import sys, os

CC='gcc'

LINKFLAGS=['-lm']

CFLAGS=['-std=gnu99','-Wall','-Werror']

programs = [{'name':'test-config','sources':['test-config','config']}]

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
    raise RuntimeError("install of library not yet implemented")

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
main()

