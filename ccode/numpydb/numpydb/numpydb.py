"""
Module:
    numpydb
Purpose:

    This module contains a class NumpyDB that Facilitates writing and reading
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
            Open(dbfile, mode='r'):  Just returns NumpyDB(dbfile, mode=mode)
            create():  Create a new database.

    For more info, see the documentation for each individual class/method.


"""
import cnumpydb
import numpy
import os

# create if it doesn't exist, allow writing
DB_CREATE   = 1
# Error if it already exists. How to use this?
DB_EXCL     = 4096
# read only
DB_RDONLY   = 16
# delete any data in the table if exists
DB_TRUNCATE = 128

mode_dict = {}

# read only
mode_dict['r'] = DB_RDONLY

# these allow updating and creation
mode_dict['r+'] = DB_CREATE
mode_dict['a'] = DB_CREATE
mode_dict['a+'] = DB_CREATE

# if it exists, empty it, otherwise open it
mode_dict['w'] = DB_CREATE | DB_TRUNCATE
mode_dict['w+'] = DB_CREATE | DB_TRUNCATE

interval_dict={}
interval_dict['[]'] = cnumpydb.NUMPYDB_GE_LE
interval_dict['[)'] = cnumpydb.NUMPYDB_GE_LT
interval_dict['(]'] = cnumpydb.NUMPYDB_GT_LE
interval_dict['()'] = cnumpydb.NUMPYDB_GT_LT

interval_dict['='] = cnumpydb.NUMPYDB_EQ

interval_dict['>'] = cnumpydb.NUMPYDB_GT
interval_dict['>='] = cnumpydb.NUMPYDB_GE
interval_dict['<'] = cnumpydb.NUMPYDB_LT
interval_dict['<='] = cnumpydb.NUMPYDB_LE

select_dict={}
select_dict['values'] = cnumpydb.NUMPYDB_GETDATA
select_dict['keys'] = cnumpydb.NUMPYDB_GETKEYS
select_dict['both'] = cnumpydb.NUMPYDB_GETBOTH
select_dict['count'] = cnumpydb.NUMPYDB_GETCOUNT


def Open(dbfile, mode='r', verbose=False, verbosity=0):
    """
    Package:
        numpydb
    Name:
        Open
    Calling Sequence:
        >>> import numpydb
        >>> db=numpydb.Open(dbfile, mode='r', verbose=False)

    Purpose:
        Convienience function to open a database file.  Just
        returns NumpyDB(dbfile,mode=mode).  See the docs for
        the numpydb.NumpyDB class for more details.

    """
    db = NumpyDB()

    if verbose:
        verbosity=1
    db.set_verbosity(verbosity)
    db.open(dbfile, mode=mode)
    return db


def create(dbfile, key_dtype, data_dtype, verbosity=0):
    """
    Name:
        create
    Purpose:
        A helper function for creating a new database.  If the
        file already exists, and exception is raised.

    Inputs:
        dbfile: String representing the database file name to create.
            If the file exists, and exceptioni is raised.
        key_dtype: A string data type for the keys.  E.g.
            'i4', 'f8', 'S20', etc.
        data_dtype: A string data type for the values, or data in
            the database.  

    Errors:
        Opon failure to create the database, or if the file
        already exists, an exception is raised.

    Example:
        >>> import numpydb
        >>> key_dtype = 'S20'
        >>> data_dtype = 'i4'
        >>> numpydb.create(dbfile, key_dtype, data_dtpe)

        # now open for updating and reading
        >>> db = numpy.NumpyDB(dbfile, mode='r+')
    """

    db = NumpyDB()
    db.set_verbosity(verbosity)
    db.create(dbfile, key_dtype, data_dtype)
    del db

class NumpyDB(cnumpydb.NumpyDB):
    """
    Class:
        NumpyDB
    Purpose:
        Facilitate writing and reading of numpy arrays to Berkeley db
        databases.

    Construction:
        db = NumpyDB()
        db = NumpyDB(dbfile, mode='r')
        
        Inputs:
            dbfile:  
                A python string representing the database file.  If the file
                does not exist, an error is raised.  To create a new database,
                instantiate with no arguments and use the create() method, or
                use the create() helper function.

            mode: 
                The mode for opening the file.   Default is read only, 'r'
                    'r': Open read only
                    'r+': Open for reading and updating.
                    'w': Truncate the database and open for reading and 
                         updating.
                         

        Examples of Construction and Creation
            # creating a database with keys strings of length 20
            # and data values that are 32-bit integers (4-byte).
            # this is effectively an index on the strings.

            # use the create helper function for creation.
            >>> import numpydb
            >>> key_dtype = 'S20'
            >>> data_dtype = 'i4'
            >>> numpydb.create(dbfile, key_dtype, data_dtype)

            # constructing the NumpyBD and opening database for
            # reading and updating
            >>> db=numpydb.NumpyDB(dbfile, mode='r+')

            # construction without opening a file
            >>> db=numpydb.NumpyDB()

            # now open the database for updating and reading.
            >>> db.open(dbfile, 'r+')

            # show a representation of the database
            >>> db
            Filename: 'test-S20.db'
                key dtype: 'S20'
                data dtype: 'i4'

        Putting Records:
            # keys and values can be scalar, sequence, or array.  But
            # be convertible to the same numpy data type as the keys
            # and values in the database

            >>> db.put(keys, values)

        Query Examples:
            >>> import numpydb
            >>> db = numpydb.NumpyDB('somefile.db')

            # Extract exact key matches from the database.  Can 
            # be scalar, sequence, or array
            >>> values = db.match(values)
            
            # Extract from a range of keys, using default inclusive interval
            # [low,high].   Note between and range are synonyms
            >>> values = db.between(low, high)
            >>> values = db.range(low, high)
            
            # Extract from different types of intervals.  Again, note range()
            # is synonymous with between
            >>> values = db.between(low, high,'[]')
            >>> values = db.between(low, high,'[)')
            >>> values = db.between(low, high,'(]')
            >>> values = db.between(low, high,'()')
            >>> values = db.range(low, high,'()')


            # one sided range queries
            >>> values = db.range1(low, '>')
            >>> values = db.range1(low, '>=')
            >>> values = db.range1(high,'<')
            >>> values = db.range1(high,'<=')
            >>> values = db.range1(key2match,'=')


            # Can also return keys,values, e.g.
            >>> keys,values = db.match(values, select='both')
            >>> keys,values = db.between(low, high, select='both')

            # Just counting the matches
            >>> count = db.match(values,select='count')
            >>> count = db.between(low, high,select='count')

            # just getting the keys
            >>> keys = db.match(values, select='keys')
            >>> keys = db.between(low,high,select='keys')

    Methods:
        See the docs for each method for more details.

            create(dbfile, key_dtype, data_dtype)
                Create the database.  This must be called before opening the
                database for operations.  The dtypes must be strings, e.g.
                'f4', 'i8', 'S20'

            open(dbfile, mode='r')
                Open the database for read and write.  You must have called
                create() first.

            put(keys, data)
                Add keys and values to the database.

            match(values, select='values')
                Extract data from the database with keys that match the
                requested values.  Can be a scalar/sequence/array.

            between(low, high, interval='[]', select='values')
            AKA
            range(low, high, interval='[]', select='values')
                Extract data with keys in the range low,high. Intervals
                can be open or closed on either side.

                Currently only numerical keys are supported for range
                searching.

            range1(keyval, interval, select='values')
                Extract data for keys in a one-sided range query. Interval
                can be '<','<=','>','>=','='

            print_nrecords(n):
                Print out the top n records.

            file_name()
                Get the file name.
            key_dtype()
                Get the data type of the keys.
            data_dtype()
                Get the data type of the data entries.

            set_verbosity(iteger):
                Set to > 0 to print information during processing.

            __repr__()
                Show some stats about the database.  Pinting the db object or
                just typing its name at the prompt will show this info.

    """
    def __init__(self, dbfile=None, mode='r'):
        self.between=self.range
        self._dbfile=self.expand_filename(dbfile)
        self._mode=mode
        cnumpydb.NumpyDB.__init__(self)
        if self._dbfile is not None:
            self.open(dbfile,mode)

    def create(self, dbfile, key_dtype, data_dtype):
        """
        Class:
            NumpyDB
        Name:
            create
        Purpose:
            Create a new database and initialize the metadata table.  Note the
            numpydb.create() helper function, which is a wrapper for this
            method, may be more convenient.

        Calling Sequence:
            >>> import numpydb
            >>> db=numpydb.NumpyDB()
            >>> db.create(dbfile, key_dtype, data_dtype)

        Inputs:
            dbfile: 
                A python string holding the database file name.  The file must
                already exist.  To create a database, use the .create() method.
            key_dtype: A numpy data type string for the key values.  This type
                will be strictly enforced when data are inserted.
            data_dtype: A numpy data type string.

        Example Data Types:
            Example data type strings using single characters for types,
            followed by a byte count.  You can also include the leading
            byte orer, e.g. '<i4', but note only native endianness is
            currently supported.  Also, structured data are not supported,
            e.g. arrays with fields.

                Signed integer types:
                    'i1','i2','i4','i8',   
                Unsigned integer types:
                    'u1','u2','u4','u8'
                Floating point types:
                    'f4','f8'
                Strings:
                    'S20', 'S125', etc.

        """

        dbfile = self.expand_filename(dbfile)
        cnumpydb.NumpyDB.create(self, dbfile, key_dtype, data_dtype)

    def open(self, dbfile, mode='r'):
        """
        Class:
            NumpyDB
        Name:
            open
        Purpose:
            Open a database.  Note if the NumpyDB is constructed with the file
            argument, this method is called automatically.

        Calling Sequence:
            >>> import numpydb
            >>> db=numpydb.NumpyDB()
            >>> db.open(dbfile, mode='r')

        Inputs:

            dbfile: 
                A python string holding the database file name.  The file must
                already exist.  To create a database, use the .create() method.
            mode: Mode for opening the database.  Default is 'r'.
                    'r': Open read only
                    'r+': Open for reading and updating.
                    'w': Truncate the database and open for reading and 
                         updating.

        """
        if mode not in mode_dict:
            raise ValueError("Don't know how to open with mode: '%s'" % mode)
        
        self._dbfile = self.expand_filename(dbfile)
        self._mode = mode
        self._open_flags = mode_dict[mode]

        cnumpydb.NumpyDB.open(self, dbfile, self._open_flags)

    def close(self):
        """
        Class:
            NumpyDB
        Name:
            open
        Purpose:
            Close the database.
        Calling Sequence:
            >>> db.close()
        """
        cnumpydb.NumpyDB.close(self)

    def put(self, keys, values):
        """
        Class:
            NumpyDB
        Method:
            put
        Purpose:
            Add new records to the database.  The entered keys and values must
            be convertible to the correct types.

        Calling Sequence:
            >>> import numpydb
            >>> db=numpydb.NumpyDB('somefile.db')

            # Extract the data values for the range of keys
            >>> data.put(keys, values)

        Inputs:
            keys:
                New keys to add to the database.  Must be convertible to the
                key data type.

            values:
                New values to add to the database.  Must be convertible to the
                values data type.  Must be the same length as the keys.
        """
        cnumpydb.NumpyDB.put(self, keys, values)


    def range(self, low, high, interval='[]', select='values'):
        """
        Class:
            NumpyDB
        Method:
            between()
            synonymous with range()
        Purpose:
            Extract entries from the database with keys in the range low,high.
            Default is to use an inclusive interval, but this is configurable
            with the interval keyword.  By default the values are returned, but
            one can also extract keys, both or just the count.

            range() and between() are synonyms.

        Calling Sequence:
            # two-sided range queries
            result=between(low,high,interval='[]',select='values')
            result=range(low,high,interval='[]',select='values')

            # one-sided range queries, specifier can be '>','>=','<','<='
            result=range(val, specifier)

            # note this is better done with the match() method.
            result=range(val, '=')

        Examples:
            >>> import numpydb
            >>> db=numpydb.NumpyDB('somefile.db')

            # Extract the data values for the range of keys
            # default interval is closed on both sides: '[]'
            >>> data = db.between(low,high)

            # Extract from different types of intervals
            >>> values = db.between(low, high,'[]')
            >>> values = db.between(low, high,'[)')
            >>> values = db.between(low, high,'(]')
            >>> values = db.between(low, high,'()')

            # you can also do one-sided range queries but this is better
            # done using the range1() function
            >>> values = db.range(low, None, '>')
            >>> values = db.range(low, None, '>=')
            >>> values = db.range(None, high,'<')
            >>> values = db.range(None, high,'<=')

            # you can also test for equality, but this is better done using 
            # the match() method
            >>> values = db.range(value,None,'=')

            # Extract the key values for the range of keys
            >>> keys = db.between(low,high,select='keys')

            # Extract both keys and values for the range of keys
            >>> keys,values = db.between(low,high,select='both')

            # just return the count
            >>> count = db.between(low,high,select='count')

        Inputs:
            low: 
                the lower end of the range.  Must be convertible to the key data
                type.  Can be None if you are doing < or <= queries.

            high: 
                the upper end of the range.  Must be convertible to the key data
                type. Can be None if doing >, >=, or == queries, although note
                for equality it is better to use the match() method.

        Optional Inputs:
            interval:
                '[]': Closed on both sides
                '[)': Closed on the lower side, open on the high side.
                '(]': Open on the lower side, closed on the high side
                '()': Open on both sides.
                '>': One sided open
                '>=': One sided closed.
                '<': One sided open
                '<=': One sided closed.
                '=': Equality.

            select: Which data to return.  Can be
                'values': Return the values of the key-value pairs (Default)
                'keys': Return the keys of the key-value pairs.
                'both': Return a tuple (keys,values)
                'count': Return the count of all matches.

            Default behaviour is to return the values of the key-value
            pairs in the database.

        """

        if select not in select_dict:
            raise ValueError("Bad selection indicator: '%s'" % select)
        return_type = select_dict[select]

        if interval not in interval_dict:
            raise ValueError("Bad interval indicator: '%s'" % interval)
        query_type=interval_dict[interval]

        #return cnumpydb.NumpyDB.between(self, low, high, return_type)
        return cnumpydb.NumpyDB.range_generic(self, 
                                              low, high, 
                                              query_type, return_type)

    def range1(self, keyval, interval, select='values'):
        """
        Class:
            NumpyDB
        Method:
            range1()
        Purpose:
            Extract entries from the database with keys in a one-sided range,
            such as '>', '>=', '<', '<=', or even equality '='.

        Calling Sequence:
            result=range1(key,interval,select='values')

        Examples:
            >>> import numpydb
            >>> db=numpydb.NumpyDB('somefile.db')

            # Extract the data values for the one-sided range of keys

            # you can also do one-sided range queries.  These are more easily
            # done using the range1() function, which wraps this one
            >>> values = db.range1(low, '>')
            >>> values = db.range1(low, '>=')
            >>> values = db.range1(high,'<')
            >>> values = db.range1(high,'<=')

            # you can also test for equality, but this is better done using 
            # the match() method
            >>> values = db.range1(value,'=')

            # Extract the key values for the range of keys
            >>> keys = db.range1(val,interval,select='keys')

            # Extract both keys and values for the range of keys
            >>> keys,values = db.range1(val,interval,select='both')

            # just return the count
            >>> count = db.range1(value,select='count')

        Inputs:
            key: 
                A bound to use in the one sided range query.
            interval:
                '>': One sided open
                '>=': One sided closed.
                '<': One sided open
                '<=': One sided closed.
                '=': Equality.

        Keyword Inputs:
            select: Which data to return.  Can be
                'values': Return the values of the key-value pairs (Default)
                'keys': Return the keys of the key-value pairs.
                'both': Return a tuple (keys,values)
                'count': Return the count of all matches.

            Default behaviour is to return the values of the key-value
            pairs in the database.

        """

        if select not in select_dict:
            raise ValueError("Bad selection indicator: '%s'" % select)
        return_type = select_dict[select]

        if interval not in ['>','>=','<','<=','=']:
            raise ValueError("Bad interval indicator: '%s'" % interval)
        query_type=interval_dict[interval]

        if interval in ['>','>=','=']:
            low=keyval
            high=None
        else:
            high=keyval
            low=None


        return cnumpydb.NumpyDB.range_generic(self, 
                                              low, high, 
                                              query_type, return_type)

    def match(self, keys2match, select='values'):
        """
        Class:
            NumpyDB
        Method:
            match
        Purpose:
            Extract entries from the database with keys that match the input
            value or values.  The values can be a scalar, sequence, or array,
            and must be convertible to the key data type.

        Calling Sequence:
            result=match(keys2match, select='values')

            >>> import numpydb
            >>> db=numpydb.NumpyDB('somefile.db')

            # Extract the data values that match the value
            >>> values = db.match(key)

            # Extract the data values that match any of the sequence of values
            >>> values = db.match([key1,key2,key3])


            # Extract the key values; these will simply be equall to the
            # requested values
            >>> keys = db.match(keys2match, select='keys')

            # Extract both keys and values for the range of keys
            >>> keys,data = db.match(values,select='both')

            # just return the count
            >>> count = db.match(values,select='count')

        Inputs:
            keys2match:
                A scalar, sequence, or array of keys to match.  All entries
                that have keys matching any of the entered values are returned.
                Must be convertible to the key data type. 

                Note, these values should be *unique*, otherwise you'll get
                duplicates returned.

        Optional Inputs:
            select: Which data to return.  Can be
                'values': Return the values of the key-value pairs (Default)
                'keys': Return the keys of the key-value pairs.
                'both': Return a tuple (keys,values)
                'count': Return the count of all matches.

            Default behaviour is to return the values of the key-value paris in
            the database.

        """

        if select not in select_dict:
            raise ValueError("Bad selection indicator: '%s'" % select)
        return_type = select_dict[select]

        # this functionality is in range_generic as well but 
        # we gain 5% speed using this version
        return cnumpydb.NumpyDB.match(self, keys2match, return_type)



    def print_nrecords(self, num):
        """
        Class:
            NumpyDB
        Method:
            print_nrecords
        Purpose:
            Print the top n entries in the database.

        Calling Sequence:
            >>> import numpydb
            >>> db=numpydb.NumpyDB('somefile.db')

            >>> db.print_nrecords(10)

        """ 
        num2send = long(num)
        cnumpydb.NumpyDB.print_nrecords(self, num2send)

    def set_verbosity(self, verbosity):
        """
        Class:
            NumpyDB
        Method:
            set_verbosity
        Purpose:
            Set the verbosity level.  Higher verbosity means more
            informational messages.

        Calling Sequence:
            >>> import numpydb
            >>> db=numpydb.NumpyDB('somefile.db')
            >>> db.print_nrecords(10)

        """ 

        verbosity = long(verbosity)
        cnumpydb.NumpyDB.set_verbosity(self, verbosity)

    def file_name(self):
        """
        Class:
            NumpyDB
        Method:
            file_name
        Purpose:
            Get the file name for the current database.
        Calling Sequence:
            >>> import numpydb
            >>> db=numpydb.NumpyDB('somefile.db')
            >>> fname = db.file_name()
        """

        return cnumpydb.NumpyDB.file_name(self)

    def key_dtype(self):
        """
        Class:
            NumpyDB
        Method:
            key_dtype
        Purpose:
            Get the numpy data type of the keys in the current database.

        Calling Sequence:
            >>> import numpydb
            >>> db=numpydb.NumpyDB('somefile.db')
            >>> dtype = db.file_name()
        """

        return cnumpydb.NumpyDB.key_dtype(self)

    def data_dtype(self):
        """
        Class:
            NumpyDB
        Method:
            data_dtype
        Purpose:
            Get the numpy data type of the data values in the current database.

        Calling Sequence:
            >>> import numpydb
            >>> db=numpydb.NumpyDB('somefile.db')
            >>> dtype = db.file_name()
        """

        return cnumpydb.NumpyDB.data_dtype(self)



    def __repr__(self):
        fname=self.file_name()
        key_dtype=self.key_dtype()
        data_dtype = self.data_dtype()
        mode = self._mode
        s = []
        if fname != "":
            s += ["    filename:   '%s'" % fname]
        if key_dtype != "":
            s += ["    key dtype:  '%s'" % key_dtype]
        if data_dtype != "":
            s += ["    data dtype: '%s'" % data_dtype]
        if mode != "" and mode != None:
            s += ["    mode:       '%s'" % mode]

        if len(s) > 0:
            s = ['Database Info:'] + s
        s = '\n'.join(s)
        return s

    def expand_filename(self, name):
        if name is None:
            return name
        newname = os.path.expanduser(name)
        newname = os.path.expandvars(newname)
        return newname



