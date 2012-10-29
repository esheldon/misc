from fabricate import *
import sys, os

import glob


CC='gcc'

LINKFLAGS=['-lm']

CFLAGS=['-std=gnu99','-Wall','-Werror']

test_sources = ['test']
ref_sources = ['reftest']

programs = [{'name':'test', 'sources':test_sources},
            {'name':'reftest', 'sources':ref_sources}]

def build():
    compile()
    link()

def compile():
    for prog in programs:
        for source in prog['sources']:
            run(CC,'-c','-o',source+'.o', CFLAGS, source+'.c')

def link():
    for prog in programs:
        objects = [s+'.o' for s in prog['sources']]
        run(CC,'-o', prog['name'], objects,LINKFLAGS)

def clean():
    autoclean()

# send options so it won't crash on us
main()

