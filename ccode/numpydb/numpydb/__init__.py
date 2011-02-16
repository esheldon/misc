"""
Package:
    numpydb
Purpose:

    This package contains a class NumpyDB that Facilitates writing and reading
    of numpy arrays to and from Berkeley db databases.  

    Berkeley db is a key-value store.  A use for this module is to make the
    "keys" of this database the values from a large set of data, and then
    make the "values" indices into the original data.  Because the database
    stores the keys in a b-tree, it is very fast to search and return the
    indices needed to extract subsets from the larger dataset.  

    This module contains the following functions and classes:

        Classes:
            NumpyDB: The class that implements database access.

        Helper functions:
            create():  Create a new database.

    For more info, see the documentation for each individual class/method.


"""

import numpydb
from numpydb import *
import unit_tests
