import esutil
import stomp
import numpy
import sys
import os

# remember, this is the old style map
mapfile=os.path.expanduser('~/masks/stripe_dr5_north.map_bound')

sm=stomp.StompMap(mapfile, system="sdss")

print 'Area:',sm.area()
print 'System:',sm.system()


x=numpy.arange(9, dtype='f8').reshape( (3,3) )

out=sm.TestNumpyVector(x);
print 'out=',out

sys.exit(0)

n=10
print 'generating random clambda/ceta coords'
clambda,ceta=sm.genrand(n)
print '  clambda:',clambda
print '  ceta:',ceta


print
print '  now checking these points are contained in the map. Should all be 1'

contained = sm.contains(clambda,ceta,system="sdss")
print '  ',contained

crap="""
print
print '  now checking with radius'
radius = numpy.zeros(n,dtype='f8')
radius[:] = 0.02


out=numpy.zeros(n, dtype=[('clambda','f8'),('ceta','f8'),('radius','f8')])
out['clambda'] =clambda
out['ceta'] = ceta
out['radius'] = radius
esutil.sfile.write(out,'test.rec',delim=' ')
"""


t=esutil.sfile.read('test2.rec')

clambda = t['clambda']
ceta = t['ceta']
radius = t['radius']

clambda = -5.29916
ceta = 35.844

#contained = sm.contains(clambda,ceta,radius=radius,system="sdss")
contained = sm.contains(clambda,ceta,system="sdss")
print '  ',contained
w,=numpy.where( contained != 1)
print '  number wedge inside:',w.size


sys.exit(0)

plt=esutil.plotting.setuplot()
plt.plot(clambda,ceta,',')
plt.plot(clambda[w],ceta[w],'.',color='red')
plt.show()
sys.exit(0)

print 'generating random ra,dec coords'
ra,dec=sm.genrand(10)
print '  ra:',ra
print '  dec:',dec

print
print '  now checking these points are contained in the map. Should all be 1'

contained = sm.contains(ra,dec)
print '  ',contained


n=100
print 'generating',n,'clambda/ceta examples over whole shere'
clambda = numpy.random.random(n)*180. - 90.0
ceta = numpy.random.random(n)*360. - 180.0

#print '  ra (whole sphere): ',ra_all
#print '  dec (whole sphere): ',dec_all

print '  checking against map with radius'

radius = numpy.zeros(n,dtype='f8')
radius[:] = 1.0

import time
tm0=time.time()
contained_all = sm.contains(clambda, ceta, radius=radius, system="sdss")
print 'time:',(time.time()-tm0)/60.0

#print '  ',contained_all

