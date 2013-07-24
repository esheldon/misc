"""
build the tests for gmix_em
"""

from fabricate import *
import sys, os
import glob
import optparse

CC='gcc'

# -lrt is only needed for the timing stuff
LINKFLAGS=['-lm']

CFLAGS=['-std=gnu99','-Wall','-Werror','-O2','-mfpmath=sse']

test_sources = ['test','gauss2','gmix','image','gmix_em','mtx2','gmix_image']
test_cocenter_sources = ['test-cocenter','gauss2','gmix','image','gmix_em','mtx2','gmix_image']

programs = [{'name':'test', 'sources':test_sources},
            {'name':'test-cocenter', 'sources':test_cocenter_sources}]

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


main()
