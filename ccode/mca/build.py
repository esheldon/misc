"""
Build the admom test program
"""
from fabricate import *
import sys, os

CC='gcc'

LINKFLAGS=['-lm']

CFLAGS=['-std=gnu99','-Wall','-Werror','-O2']

programs=[]

trand_src=['test/test-mca-rand','mca','randn']
trand_prog = {'name':'test/test-mca-rand','sources':trand_src}

tconst_src=['test/test-mca-const','mca','randn']
tconst_prog = {'name':'test/test-mca-const','sources':tconst_src}

programs=[trand_prog, tconst_prog]

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
        #run(CC,LINKFLAGS,'-o', prog['name'], objects)
        run(CC,'-o',prog['name'],objects,LINKFLAGS)

def clean():
    autoclean()

main()

