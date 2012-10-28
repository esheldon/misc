"""
build the tests for gmix_em
"""

from fabricate import *
import sys, os
import glob
import optparse

CC='gcc'

# -lrt is only needed for the timing stuff
LINKFLAGS=['-lcfitsio','-lm']

#CFLAGS=['-std=gnu99','-Wall','-Werror','-O2','-mfpmath=sse']
CFLAGS=['-std=gnu99','-Wall','-Werror','-O2']

sources=['gmix-galsim','gmix_image','gmix_image_fits',
         'gmix','gmix_mcmc','image','mca','gauss',
         'image_rand','randn',
         'admom','shape']
objlist_sources=['gmix-galsim-objlist']
test_read_sources = ['test/test-read','image']

pstats_sources=['gmix-galsim-pstats']

programs = [{'name':'gmix-galsim', 'sources':sources},
            {'name':'gmix-galsim-objlist', 'sources':objlist_sources},
            {'name':'test/test-read', 'sources':test_read_sources},
            {'name':'gmix-galsim-pstats', 'sources':pstats_sources}]

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
