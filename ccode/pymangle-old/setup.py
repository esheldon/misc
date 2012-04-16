import distutils
from distutils.core import setup, Extension, Command
import os
import numpy

data_files=[]

ext=Extension("mangle._mangle", ["mangle/_mangle.c",
                                 "mangle/point.c",
                                 "mangle/cap.c",
                                 "mangle/polygon.c",
                                 "mangle/stack.c",
                                 "mangle/pixel.c",
                                 "mangle/rand.c"])
setup(name="mangle", 
      packages=['mangle'],
      version="0.1",
      data_files=data_files,
      ext_modules=[ext],
      include_dirs=numpy.get_include())


