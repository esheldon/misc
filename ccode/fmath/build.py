"""
Build the admom test program
"""
from fabricate import *
import sys, os
import optparse



parser = optparse.OptionParser()
# make an options list, also send to fabricate
optlist=[optparse.Option('--make-dtbl',action="store_true",help="turn off compiler optimizations")]
parser.add_options(optlist)
options,args = parser.parse_args()


CC='gcc'
LINKFLAGS=['-lm']
CFLAGS=['-std=gnu99','-Wall','-Werror','-O2']


if options.make_dtbl:
    make_dtbl_src = ['make-dtbl']
    programs=[{'name':'make-dtbl','sources':make_dtbl_src}]
else:
    test_acc_src =['test-accuracy']
    test_speed_src =['test-speed']

    programs=[{'name':'test-accuracy','sources':test_acc_src},
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

# send options so it won't crash on us
main(extra_options=optlist)
