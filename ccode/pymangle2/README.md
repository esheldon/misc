A simple code to read and work with Mangle masks.

Included are both a python library and a couple of
simple stand alone codes.

build and install python library
--------------------------------

    python setup.py install --prefix=/some/path

build and install the stand alone routines
------------------------------------------

    # just build
    python build.py

    # also install.  Note different order from above
    python build.py --prefix=/some/path install

    # clean up
    python build.py clean
