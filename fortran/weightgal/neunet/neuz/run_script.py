#!/usr/bin/env python
import os
import sys
import shutil

MAX_IT=5

itrange=range(MAX_IT)
prefix=sys.argv[1]
dfilelist=''
wfilelist=''
sfilelist=''
trainfile=prefix + '.train'
validfile=prefix + '.valid'
fitfile=prefix + '.nfit'
for it in itrange:
    blah=256
    print 'it = ' + str(it)
    while blah > 255:
        blah=os.system('./neuz.x run.param ' + trainfile + ' ' + validfile)
	print 'blah = ' + str(blah)
        if blah > 0 and blah < 256:
           print 'blah failed' 
           sys.exit(1)
    os.system('./neu_fit.x blah.wts ' + fitfile)
    dfile='%d.tbl' % it
    wfile='%d.wts' % it
    dfilelist=dfilelist + dfile + ' '
    wfilelist=wfilelist + wfile + ' '
    shutil.copy('blah.wts', wfile)
    shutil.copy('bzphot.tbl', dfile)
print dfilelist
print wfilelist
os.system('./combine_dat.x ' + dfilelist)
shutil.move('com_mean.xtbl', 'final_mean.xtbl')
