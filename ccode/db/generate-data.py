import os
import numpy
import esutil

print 'generating floats'
n=10000000
rfloat = numpy.zeros(n, dtype='f4')
r = numpy.random.random(n)
rfloat[:] = r[:]
del r

fname='random-float-10000000.bin'
print 'writing floats to file: ',fname
rfloat.tofile(fname)

del rfloat

print '\ngenerating int32'
ri = esutil.numpy_util.randind(1000, n, dtype='i4')

fname='random-int32-10000000.bin'
print 'writing int32 to file: ',fname
ri.tofile(fname)


spath='/global/data/DES/wlbnl/wlse0003/collated/wlse0003.cols/exposurename.col'
if os.path.exists(spath):
    print 'Getting exposurenames (S20)'
    data=esutil.sfile.read(spath)

    output='S20-10000000.bin'
    ename=data['exposurename']
    print 'Writing to S20 file: ',output
    ename.tofile(output)
