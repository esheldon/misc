"""
build the tests for rand
"""

from fabricate import *
import sys, os
import glob
import optparse

CC='gcc'

# -lrt is only needed for the timing stuff
LINKFLAGS=['-lm']

CFLAGS=['-std=gnu99','-Wall','-Werror','-O2']

poisson_sources = ['gen-poisson','randn']
randn_sources = ['gen-randn','randn']

programs = [{'name':'gen-poisson', 'sources':poisson_sources},
            {'name':'gen-randn', 'sources':randn_sources}]

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


main()
