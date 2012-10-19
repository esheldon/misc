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

test_gauss_sources = ['test-gauss','gauss','gmix','image','gmix_mcmc','gmix_image','mca','admom','randn']
test_coellip_sources = ['test-coellip','gauss','gmix','image','gmix_mcmc','gmix_image','mca','admom','randn']

programs = [{'name':'test-gauss', 'sources':test_gauss_sources},
            {'name':'test-coellip', 'sources':test_coellip_sources}]
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
