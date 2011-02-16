import numpy
import numpydb
from numpydb import cnumpydb
import os
from esutil.misc import colprint

import tempfile

def test_range():

    # here we test range queries, including
    # equality checks, even though these are more simply
    # done using the match() method.

    # temp file names
    fdbfile=tempfile.mktemp(prefix='f8rand-i4index-', suffix='.db')
    idbfile=tempfile.mktemp(prefix='i4data-i4index-', suffix='.db')

    # create the databases
    n=10
    f = numpy.arange(n,dtype='f8')
    i = numpy.arange(n,dtype='i4')

    numpydb.create(fdbfile, "f8", "i4")
    numpydb.create(idbfile, "i4", "i4")


    db = numpydb.Open(fdbfile, "r+")
    db.put(f, i)
    db.close()
    del db

    db = numpydb.Open(idbfile, "r+")
    db.put(i, i)
    db.close()
    del db

        
    # now open for reading and tests
    fdb = numpydb.Open(fdbfile)
    idb = numpydb.Open(idbfile)

    dbs = [fdb, idb]
    types = ['float','int']

    for i in xrange(2):
        db = dbs[i]
        type = types[i]

        print '\nTesting %s data' % type
        print '-'*70
        val=9999
        print '\nfirst printing all'
        print '-'*70
        keys,data = db.range(None, val, '<', select='both')

        colprint(keys,data, format='%15s')


        val=5
        print '\ntesting == %s on %s' % (val,type)
        print '-'*70
        keys,data = db.range(val, None, '=', select='both')

        colprint(keys,data, format='%15s')


        print '\ntesting >= %s on %s' % (val,type)
        print '-'*70
        keys,data = db.range(val, None, '>=', select='both') 

        colprint(keys,data, format='%15s')

        print '\ntesting > %s on %s' % (val,type)
        print '-'*70
        keys,data = db.range(val, None, '>', select='both') 

        colprint(keys,data, format='%15s')

        print '\ntesting <= %s on %s' % (val,type)
        print '-'*70
        keys,data = db.range(None, val, '<=', select='both') 

        colprint(keys,data, format='%15s')

        print '\ntesting < %s on %s' % (val,type)
        print '-'*70
        keys,data = db.range(None, val, '<', select='both') 

        colprint(keys,data, format='%15s')

        low=5
        high=8
        print '\ntesting %s <= key <= %s %s' % (low,high,type)
        print '-'*70
        keys,data = db.range(low,high, '[]', select='both')

        colprint(keys,data, format='%15s')

        print '\ntesting %s <= key < %s %s' % (low,high,type)
        print '-'*70
        keys,data = db.range(low,high, '[)', select='both')

        colprint(keys,data, format='%15s')

        print '\ntesting %s < key <= %s %s' % (low,high,type)
        print '-'*70
        keys,data = db.range(low,high, '(]', select='both')

        colprint(keys,data, format='%15s')

        print '\ntesting %s < key < %s %s' % (low,high,type)
        print '-'*70
        keys,data = db.range(low,high, '()', select='both')

        colprint(keys,data, format='%15s')


    os.remove(fdbfile)
    os.remove(idbfile)

def test_range1():

    # here we test range queries, including
    # equality checks, even though these are more simply
    # done using the match() method.

    # temp file names
    fdbfile=tempfile.mktemp(prefix='f8rand-i4index-', suffix='.db')
    idbfile=tempfile.mktemp(prefix='i4data-i4index-', suffix='.db')

    # create the databases
    n=10
    f = numpy.arange(n,dtype='f8')
    i = numpy.arange(n,dtype='i4')

    numpydb.create(fdbfile, "f8", "i4")
    numpydb.create(idbfile, "i4", "i4")


    db = numpydb.Open(fdbfile, "r+")
    db.put(f, i)
    db.close()
    del db

    db = numpydb.Open(idbfile, "r+")
    db.put(i, i)
    db.close()
    del db

        
    # now open for reading and tests
    fdb = numpydb.Open(fdbfile)
    idb = numpydb.Open(idbfile)

    dbs = [fdb, idb]
    types = ['float','int']

    for i in xrange(2):
        db = dbs[i]
        type = types[i]

        print '\nTesting %s data' % type
        print '-'*70
        val=9999
        print '\nfirst printing all'
        print '-'*70
        keys,data = db.range1(val, '<', select='both')

        colprint(keys,data, format='%15s')


        val=5
        print '\ntesting == %s on %s' % (val,type)
        print '-'*70
        keys,data = db.range1(val, '=', select='both')

        colprint(keys,data, format='%15s')


        print '\ntesting >= %s on %s' % (val,type)
        print '-'*70
        keys,data = db.range1(val, '>=', select='both') 

        colprint(keys,data, format='%15s')

        print '\ntesting > %s on %s' % (val,type)
        print '-'*70
        keys,data = db.range1(val, '>', select='both') 

        colprint(keys,data, format='%15s')

        print '\ntesting <= %s on %s' % (val,type)
        print '-'*70
        keys,data = db.range1(val, '<=', select='both') 

        colprint(keys,data, format='%15s')

        print '\ntesting < %s on %s' % (val,type)
        print '-'*70
        keys,data = db.range1(val, '<', select='both') 

        colprint(keys,data, format='%15s')


    os.remove(fdbfile)
    os.remove(idbfile)
