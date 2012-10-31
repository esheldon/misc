"""
Build the admom test program
"""
from fabricate import *
import sys, os

import glob


CC='gcc'

LINKFLAGS=['-lm']

CFLAGS=['-std=gnu99','-Wall','-Werror','-O2']

programs=[]

make_dtbl_src = ['make-dtbl']
test_acc_src =['test-accuracy']
test_speed_src =['test-speed']

programs=[{'name':'make-dtbl','sources':make_dtbl_src},
          {'name':'test-accuracy','sources':test_acc_src},
          {'name':'test-speed','sources':test_speed_src}]


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
        run(CC,'-o',prog['name'],objects,LINKFLAGS)

def clean():
    autoclean()

main()

