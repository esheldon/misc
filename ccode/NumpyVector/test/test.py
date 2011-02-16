import tester
import numpy

t=tester.tester()

t.dotest_creation()

print 'testing array'
iarr = numpy.arange(9,dtype='f4')

t.dotest_fromobj(iarr)
print 'After call: ',iarr

print '\ntesting scalar'
iarr = numpy.array(3,dtype='f4')
t.dotest_fromobj(iarr)
print 'After call: ',iarr

print '\ntesting array scalar i2'
iarr = numpy.array(223,dtype='i2')
t.dotest_fromobj(iarr)
print 'After call: ',iarr

print '\ntesting python scalar float'
iarr = 8.6
t.dotest_fromobj(iarr)
print 'After call: ',iarr


print '\ntesting getting output'
tmp = t.dotest_output()
print 'result:',tmp


print '\ntesting records'
dtype=[('i1field','i1'),
       ('u1field','u1'),
       ('i2field','i2'),
       ('u2field','u2'),
       ('i4field','i4'),
       ('u4field','u4'),
       ('i8field','i8'),
       ('u8field','u8'),
       ('f4field','f4'),
       ('f8field','f8'),
       ('str','S5')]

data = numpy.zeros(3, dtype=dtype)
data['u1field'] = (1,2,3)
data['i1field'] = (-1,-2,-3)

data['u2field'] = (10,20,30)
data['i2field'] = (-10,-20,-30)

data['u4field'] = (100,200,300)
data['i4field'] = (-100,-200,-300)

data['u8field'] = (1000,2000,3000)
data['i8field'] = (-1000,-2000,-3000)

data['f4field'] = (111.11,222.22, 333.33)
data['f8field'] = (21.334,numpy.pi,-943.23)

data['str'] = ('hello','there','world')

tmp = t.testrec(data)
