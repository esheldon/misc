import os
from distutils.core import setup,Extension

# if you run 
#   python setup.y build_ext --inplace
# it will put it in cwd for easier testing

CFLAGS='-std=c99'
if 'CFLAGS' in os.environ:
    os.environ['CFLAGS'] += CFLAGS
else:
    os.environ['CFLAGS'] = CFLAGS
import numpy
include_dirs=numpy.get_include()
# can be a list
ext_modules = [Extension('_swigtest', sources=['swigtest_wrap.c','swigtest.c'])]
py_modules = ['swigtest']


# data_files copies the ups/esutil.table into prefix/ups
setup(name='swigtest',
      description='Test extension module C++ class built with swig',
      ext_modules=ext_modules,
      py_modules=py_modules,
      include_dirs=include_dirs)
