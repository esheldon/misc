from __future__ import print_function
from fabricate import *
import sys, os
import optparse

import glob


parser = optparse.OptionParser()
# make an options list, also send to fabricate
optlist=[optparse.Option('--prefix','-p',default=sys.exec_prefix),
         optparse.Option('--f90',default='gfortran'),
         optparse.Option('--openmp',default='gfortran')]
parser.add_options(optlist)

options,args = parser.parse_args()
prefix=os.path.expanduser( options.prefix )
f90=options.f90
openmp=options.openmp

if f90 == 'ifort':
    if openmp:
        f90flags=['-implicitnone','-fast','-openmp','-fpp']
        linkflags=['-fast','-openmp','-fpp']
    else:
        f90flags=['-implicitnone','-fast']
        linkflags=['-fast']
else:
    if openmp:
        f90flags=['-fimplicit-none','-O2','-fopenmp']
        linkflags=['-fopenmp']
    else:
        f90flags=['-fimplicit-none','-O2']
        linkflags=[]



sources = ['errlib',
           'fileutil',
           'intlib',
           'interplib',
           'healpix',
           'gcirclib',
           'sortlib',
           'cosmolib',   # requires intlib
           'configlib',  # fileutil
           'lenslib',    # fileutil, cosmolib
           'srclib',     # fileutil, cosmolib, healpix
           'histogram',  # errlib
           'shearlib',   # srclib,lenslib,configlib,cosmolib,healpix,fileutil
           'objshear']


programs = [{'name':'objshear','sources':sources}]


install_targets = [(prog['name'],'bin') for prog in programs]
install_targets += [('objshear.table','ups')]


def build():
    compile()
    link()

def compile():
    for prog in programs:
        for source in prog['sources']:
            run(f90, '-c', f90flags, source+'.f90')

def link():
    for prog in programs:
        objects = [s+'.o' for s in prog['sources']]
        run(f90,linkflags,'-o', prog['name'], objects)

def clean():
    autoclean()


def install():
    import shutil

    # make sure everything is built first
    build()

    for target in install_targets:
        (name,subdir) = target
        subdir = os.path.join(prefix, subdir)
        if not os.path.exists(subdir):
            os.makedirs(subdir)

        dest=os.path.join(subdir, os.path.basename(name))
        print("install:",dest)
        shutil.copy(name, dest)

# send options so it won't crash on us
main(extra_options=optlist)

