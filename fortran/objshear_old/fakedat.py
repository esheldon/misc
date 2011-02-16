import numpy
import recfile

nsource=4
nzvals=10

dtype = [('ra','f8'),('dec','f8'),
         ('e1','f4'),('e2','f4'),('err','f4'),
         ('htmid10','i4'),
         ('scinv','f4',nzvals)]
         
data=numpy.zeros(nsource,dtype=dtype)

data['ra'] = [331.234326, 186.97321, 1.97321, 2.0]
data['dec'] = [186.97321, -14.23412, 3.14159e-13, 1.0]

data['e1'] = numpy.random.random(nsource)
data['e2'] = numpy.random.random(nsource)
data['err'] = numpy.random.random(nsource)

for i in xrange(nzvals):
    data['scinv'][:,i] = i


nsource_array = numpy.array([nsource], dtype='i4')

fobj=open('testbin.bin','w')

nsource_array.tofile(fobj)

data.tofile(fobj)

fobj.close()

robj = recfile.Open('testasc.dat', mode='w', delim=' ')
robj.write(data)
robj.close()
