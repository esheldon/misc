from distutils.core import setup,Extension
from os.path import join as path_join
import os,sys

# this one is built with swig, you'll have
# to run swig separately with
#  swig -python -c++ -o tarray_wrap.cpp tarray.i

# if you run
#   python setup.y build_ext --inplace
# it will put it in cwd for easier testing

import numpy
include_dirs=numpy.get_include()
# can be a list


packages=['numpydb']
ext_modules=[]


subdir='numpydb'

sources=['cnumpydb_wrap.cc', 'cnumpydb.cc']
depends = ['NumpyVoidVector.h','cnumpydb.h']

sources = [path_join(subdir,s) for s in sources]
depends = [path_join(subdir,d) for d in depends]


# link the berkeley db Library
libraries=['db']
ext_modules = [Extension('numpydb._cnumpydb', 
                         sources=sources,
                         libraries=libraries,
                         depends=depends)]



# create the ups table
pyvers='%s.%s' % sys.version_info[0:2]
d1='lib/python%s/site-packages' % pyvers
d2='lib64/python%s/site-packages' % pyvers

if not os.path.exists('ups'):
    os.mkdir('ups')
tablefile=open('ups/numpydb.table','w')
tab="""
setupOptional("python")
setupOptional("db")
envPrepend(PYTHONPATH,${PRODUCT_DIR}/%s)
envPrepend(PYTHONPATH,${PRODUCT_DIR}/%s)
""" % (d1,d2)
tablefile.write(tab)
tablefile.close()

data_files=[]
data_files.append( ('ups',[path_join('ups','numpydb.table')] ) )

# data_files copies the ups/esutil.table into prefix/ups
setup(name='numpydb',
      description='SWIGified C++ wrapper for Berkeley db-numpy interface',
      packages=packages,
      ext_modules=ext_modules,
      data_files=data_files,
      include_dirs=include_dirs)



