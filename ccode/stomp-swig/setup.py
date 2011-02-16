from distutils.core import setup,Extension

# this one is built with swig, you'll have
# to run swig separately with
#  swig -python -c++ -o tarray_wrap.cpp tarray.i

# if you run 
#   python setup.y build_ext --inplace
# it will put it in cwd for easier testing

import numpy
include_dirs=numpy.get_include()
# can be a list
stomp_sources = ['stomp_util.cc',
                 'stomp_wrap.cc',
                 'stomp.cc']
stomp_depends = ['NumpyVector.h','TypeInfo.h']
stomp_extension = Extension('_stomp', 
                            sources=stomp_sources, 
                            depends=stomp_depends)
ext_modules = [stomp_extension]
py_modules = ['stomp']


# data_files copies the ups/esutil.table into prefix/ups
setup(name='stomp',
      description='Stomp-based C++ class built with swig',
      ext_modules=ext_modules,
      py_modules=py_modules,
      include_dirs=include_dirs)
