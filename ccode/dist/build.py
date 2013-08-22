"""
build the tests for
"""

from fabricate import *
import sys, os
import glob

CC='gcc'

# -lrt is only needed for the timing stuff
LINKFLAGS=['-lm']

CFLAGS=['-std=gnu99','-Wall','-Werror','-O2']

test_gmix3_eta_sources = ['test-gmix3-eta',
                          'shape',
                          'randn',
                          'dist']
test_g_ba_sources = ['test-g-ba',
                     'shape',
                     'randn',
                     'dist']

test_lognorm_sources = ['test-lognorm',
                        'shape',
                        'randn',
                        'dist']
test_gauss_sources = ['test-gauss',
                      'shape',
                      'randn',
                      'dist']

test_pqr_sources = ['mtx2',
                    'shape',
                    'dist',
                    'randn',
                    'test-pqr']

programs = [{'name':'test-gmix3-eta', 'sources':test_gmix3_eta_sources},
            {'name':'test-g-ba', 'sources':test_g_ba_sources},
            {'name':'test-lognorm', 'sources':test_lognorm_sources},
            {'name':'test-gauss', 'sources':test_gauss_sources},
            {'name':'test-pqr', 'sources':test_pqr_sources},
           ]

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
