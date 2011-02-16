import numpy
import stomp
import os

datname='test_output/des_footprint.dat'
print 'reading file:',datname
d = stomp.smap_read(datname)

#fname='test_output/test.eps'
fname='test_output/test.pdf'
stomp.display(d['pixnum'], resolution=d['resolution'], fname=fname)
