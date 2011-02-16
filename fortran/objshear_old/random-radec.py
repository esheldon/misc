import numpy
import recfile

import esutil as eu

#n=10000000
#n=100000
n=100
#n=10
ra,dec = eu.coords.randsphere(n)

output=numpy.zeros(n, dtype=[('ra','f8'),('dec','f8')])
output['ra'] = ra
output['dec'] = dec


fobj=open('rand-radec.bin','w')

narr = numpy.array([n],dtype='i4')

narr.tofile(fobj)


output.tofile(fobj)


# also ascii to look at
robj = recfile.Open('rand-radec.dat', mode='w', delim=' ')
robj.write(output)
robj.close()
