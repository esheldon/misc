"""
build the tests for
"""

from fabricate import *
import sys, os
import glob
import optparse

CC='gcc'

# -lrt is only needed for the timing stuff
LINKFLAGS=['-lm']

#CFLAGS=['-std=gnu99','-Wall','-Werror','-O2','-mfpmath=sse']
CFLAGS=['-std=gnu99','-Wall','-Werror','-O2']

sources=['gmix_mcmc',
         'gauss2',
         'jacobian',
         'gmix',
         'shape',
         'image','image_rand',
         'gmix_image','gmix_image_rand',
         'mca',
         'gmix_sim1',
         'randn']


test_gauss_sources = ['test/test-gauss','admom'] + sources
test_turb_sources = ['test/test-turb'] + sources
test_dev_sources = ['test/test-dev'] + sources
#test_dev_Tonly_sources = ['test/test-dev-Tonly'] + sources
test_coellip_sources = ['test/test-coellip','admom'] + sources

programs = [{'name':'test/test-gauss', 'sources':test_gauss_sources},
            {'name':'test/test-turb', 'sources':test_turb_sources},
            {'name':'test/test-dev', 'sources':test_dev_sources},
            #{'name':'test/test-dev-Tonly', 'sources':test_dev_Tonly_sources},
            {'name':'test/test-coellip', 'sources':test_coellip_sources}]
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
