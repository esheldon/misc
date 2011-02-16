from distutils.core import setup,Extension

# this one is built with swig, you'll have
# to run swig separately with
#  swig -python -c++ -o tester_wrap.cpp tester.i

# if you run 
#   python setup.y build_ext --inplace
# it will put the extension module in the current working
# direcory for easier immediate testing

import numpy
include_dirs=numpy.get_include()
# can be a list
sources = ['tester.cc','tester_wrap.cc']
depends = ['../NumpyVector.h','../NumpyRecords.h','tester.h']

extension = Extension('_tester', sources=sources, depends=depends)
ext_modules = [extension]
py_modules = ['tester','test']


# data_files copies the ups/esutil.table into prefix/ups
setup(name='tester',
      description='tester class for NumpyVector',
      ext_modules=ext_modules,
      py_modules=py_modules,
      include_dirs=include_dirs)
