"""
Features
--------
    - Read subsets of table rows and columns.
    - Read and write all image types.  Can read compressed images.
    - Correctly interpret TDIM information for array columns.
    - Read and write all image types.  Can read compressed images.


Advantages
----------

    - Can read arbitrary subsets of columns and rows without loading the whole
      file.
    - Uses TDIM information to return array columns in the correct shape
    - Can write unsigned types.  Note the FITS standard does not support
      unsigned 8 byte yet.
    - Correctly writes 1 byte integers table columns, both signed and unsigned.
    - Correctly write string table columns.
    - Correctly read all types and shapes of string columns.

TODO
----
    - read header in entirety, with types.  Maybe new class for header?
    - read single header keywords
    - write/update single header keywords

    - write TDIM using standard routines
    - append rows to tables
    - writing extension names, reading by extension names
    - read row ranges
    - implement bit, logical, and complex types

NOTES:
    A principle: 
        
        since numpy uses C order, FITS uses fortran order, we have to write the
        TDIM and image dimensions in reverse order, but write the data as is.
        Then we need to also reverse the dims in creating the numpy dtype, but
        read as is.

"""
import os
import numpy
import _fitsio_wrap

class FITS:
    """
    A class to read and write FITS images and tables.

    This class uses the cfitsio library for almost all relevant work.

    parameters
    ----------
    filename: string
        The filename to open.  
    mode: int/string
        The mode, either a string or integer.
        For reading only
            'r' or 0
        For reading and writing
            'rw' or 1
        You can also use fitsio.READONLY and fitsio.READWRITE.
    create: boolean, optional
        If True, then attemp to create the file before opening.
    clobber:        
        If create=True, then remove any existing file before
        creation.
    """
    def __init__(self, filename, mode, create=False, clobber=False):
        self.open(filename, mode, create=create, clobber=clobber)
    
    def open(self, filename, mode, create=False, clobber=False):
        self.filename = extract_filename(filename)
        self.mode=mode
        self.create=create
        self.clobber=clobber
        if mode not in _int_modemap:
            raise ValueError("mode should be one of 'r','rw',READONLY,READWRITE")
        self.charmode = _char_modemap[mode]
        self.intmode = _int_modemap[mode]

        self.int_create = 1 if create else 0


        if create and clobber:
            if os.path.exists(filename):
                print 'Removing existing file'
                os.remove(filename)

        self._FITS =  _fitsio_wrap.FITS(filename, self.intmode, self.int_create)

    def close(self):
        self._FITS.close()
        self._FITS=None
        self.filename=None
        self.mode=None
        self.create=None
        self.clobber=None
        self.charmode=None
        self.intmode=None
        self.int_create=None
        self.hdu_list=None


    def reopen(self):
        """
        CFITSIO is unpredictable about flushing it's buffers.  It is sometimes
        necessary to close and reopen after writing if you want to read the
        data.
        """
        self._FITS.close()
        del self._FITS
        self._FITS =  _fitsio_wrap.FITS(self.filename, self.intmode, 0)
        self.update_hdu_list()


    def write_image(self, img):
        """
        Create a new image extension and write the data.  

        Unlike tables, the only way to create image extensions and write the
        data is atomically using this function.

        parameters
        ----------
        img: ndarray
            An n-dimensional image.

        restrictions
        ------------
        The File must be opened READWRITE
        """
        print 'writing image type:',img.dtype.descr
        self._FITS.write_image(img)
        self.update_hdu_list()

    def write_table(self, data, units=None, extname=None):
        """
        Create a new table extension and write the data.

        The table definition is taken from the input rec array.  If you
        want to append new rows to the table, access the HDU directly
        and use the write() function, e.g.
            fits[extension].write(data, append_rows=True)

        parameters
        ----------
        data: recarray
            A numpy array with fields.  The table definition will be
            determined from this array.
        extname: string, optional
            An optional string for the extension name.
        """

        if data.dtype.fields == None:
            raise ValueError("data must have fields")
        names, formats, dims = descr2tabledef(data.dtype.descr)
        print names
        print formats
        print dims
        self.create_table(names,formats,
                          units=units, dims=dims, extname=extname)
        
        for colnum,name in enumerate(data.dtype.names):
            self[-1].write_column(colnum, data[name])

    def create_table(self, names, formats, units=None, dims=None, extname=None):
        """
        Create a new, empty table extension and reload the hdu list.

        You can write data into the new extension using
            fits[extension].write(array)
            fits[extension].write_column(array)

        typically you will instead just use 

            fits.write(recarray)

        which will create the new table extension for you with the appropriate
        fields.

        parameters
        ----------
        names: list of strings
            The list of field names
        formats: list of strings
            The TFORM format strings for each field.
        units: list of strings, optional
            An optional list of unit strings for each field.
        dims: list of strings, optional
            An optional list of dimension strings for each field.  Should
            match the repeat count for the formats fields.
        extname: string, optional
            An optional extension name.

        restrictions
        ------------
        The File must be opened READWRITE
        """

        if not isinstance(names,list) or not isinstance(formats,list):
            raise ValueError("names and formats should be lists")
        if len(names) != len(formats):
            raise ValueError("names and formats must be same length")
        if units is not None:
            if not isinstance(units,list):
                raise ValueError("units should be a list")
            if len(units) != len(names):
                raise ValueError("names and units must be same length")
        if dims is not None:
            if not isinstance(dims,list):
                raise ValueError("dims should be a list")
            if len(dims) != len(names):
                raise ValueError("names and dims must be same length")
        if extname is not None:
            if not isinstance(extname,str):
                raise ValueError("extension name must be a string")
        self._FITS.create_table(names, formats, tunit=units, tdim=dims, extname=extname)

        # fits seems to have some issues with flushing.
        self.reopen()


    def update_hdu_list(self):
        self.hdu_list = []
        for ext in xrange(1000):
            try:
                hdu = FITSHDU(self._FITS, ext)
                self.hdu_list.append(hdu)
            except RuntimeError:
                break


    def moveabs_ext(self, ext):
        self._FITS.moveabs_hdu(ext+1)

    def __getitem__(self, ext):
        if not hasattr(self, 'hdu_list'):
            self.update_hdu_list()

        if isinstance(ext,(int,long)):
            return self.hdu_list[ext]
        else:
            raise ValueError("don't yet support getting "
                             "extensions by name")


    def __repr__(self):
        spacing = ' '*2
        if not hasattr(self, 'hdu_list'):
            self.update_hdu_list()

        rep = []
        rep.append("%sfile: %s" % (spacing,self.filename))
        rep.append("%smode: %s" % (spacing,_modeprint_map[self.intmode]))
        for i,hdu in enumerate(self.hdu_list):
            t = hdu.info['hdutype']
            rep.append("%sHDU%d: %s" % (spacing,(i+1), _hdu_type_map[t]))

        rep = '\n'.join(rep)
        return rep

    #def __del__(self):
    #    self.close()
    def __enter__(self):
        return self
    def __exit__(self, exception_type, exception_value, traceback):
        self.close()



class FITSHDU:
    def __init__(self, fits, ext):
        """
        A representation of a FITS HDU

        parameters
        ----------
        fits: FITS object
            An instance of a FITS object
        ext: integer
            The extension number.
        """
        self._FITS = fits
        self.ext = ext
        self._update_info()

    def write_column(self, column, data):
        """
        Write data to a column in this HDU
        """

        colnum = self._extract_colnum(column)
        #data = numpy.array(data, ndmin=1, order='F')
        # need it contiguous
        data = numpy.array(data, ndmin=1)
        #print 'writing',data.size,'to column',column
        self._FITS.write_column(colnum+1, data)



    def read(self, columns=None, rows=None):
        """
        read data from this HDU

        By default, all data are read.  For tables, Send columns= and rows= to
        select subsets of the data.  Table data are read into a recarray; use
        read_column() to get a single column as an ordinary array.

        parameters
        ----------
        columns: optional
            An optional set of columns to read from table HDUs.  Default is to
            read all.  Can be string or number.
        rows: optional
            An optional list of rows to read from table HDUS.  Default is to
            read all.
        """
        if self.info['hdutype'] == IMAGE_HDU:
            return self.read_image()

        if columns is not None and rows is not None:
            return self.read_columns(columns, rows)
        elif columns is not None:
            return self.read_columns(columns)
        elif rows is not None:
            return self.read_rows(rows)
        else:
            return self.read_all()


    def read_image(self):
        """
        Read the image.

        If the HDU is an IMAGE_HDU, read the corresponding image.  Compression
        and scaling are dealt with properly.

        parameters
        ----------
        None
        """
        dtype, shape = self._get_image_dtype_and_shape()
        array = numpy.zeros(shape, dtype=dtype)
        self._FITS.read_image(self.ext+1, array)
        return array

    def read_column(self, col, rows=None):
        """
        Read the specified column

        parameters
        ----------
        col: string/int,  required
            The column name or number.
        rows: optional
            An optional set of row numbers to read.

        """
        if self.info['hdutype'] == _hdu_type_map['IMAGE_HDU']:
            raise ValueError("Cannot yet read columns from an image HDU")

        colnum = self._extract_colnum(col)
        rows = self._extract_rows(rows)

        npy_type, shape = self._get_simple_dtype_and_shape(colnum, rows=rows)

        array = numpy.zeros(shape, dtype=npy_type)

        self._FITS.read_column(self.ext+1,colnum+1, array, rows)
        
        self._rescale_array(array, 
                            self.info['colinfo'][colnum]['tscale'], 
                            self.info['colinfo'][colnum]['tzero'])
        return array

    def read_all(self):
        # read entire thing
        dtype = self.get_rec_dtype()
        nrows = self.info['numrows']
        array = numpy.zeros(nrows, dtype=dtype)
        self._FITS.read_as_rec(self.ext+1, array)

        for colnum,name in enumerate(array.dtype.names):
            self._rescale_array(array[name], 
                                self.info['colinfo'][colnum]['tscale'], 
                                self.info['colinfo'][colnum]['tzero'])


        return array

    def read_rows(self, rows):
        if rows is None:
            # we actually want all rows!
            return self.read_all()

        rows = self._extract_rows(rows)
        dtype = self.get_rec_dtype()
        array = numpy.zeros(rows.size, dtype=dtype)
        self._FITS.read_rows_as_rec(self.ext+1, array, rows)
        return array


    def read_columns(self, columns, rows=None, slow=False):
        if self.info['hdutype'] == _hdu_type_map['IMAGE_HDU']:
            raise ValueError("Cannot yet read columns from an image HDU")

        # if columns is None, returns all.  Guaranteed to be unique and sorted
        colnums = self._extract_colnums(columns)
        # if rows is None still returns None, and is correctly interpreted
        # by the reader to mean all
        rows = self._extract_rows(rows)

        if colnums.size == self.ncol and rows is None:
            # we are reading everything
            return self.read()

        # this is the full dtype for all columns
        dtype = self.get_rec_dtype(colnums)

        if rows is None:
            nrows = self.info['numrows']
        else:
            nrows = rows.size
        array = numpy.zeros(nrows, dtype=dtype)

        if slow:
            for i in xrange(colnums.size):
                colnum = int(colnums[i])
                name = array.dtype.names[i]
                self._FITS.read_column(self.ext+1,colnum+1, array[name], rows)
                self._rescale_array(array[name], 
                                    self.info['colinfo'][colnum]['tscale'], 
                                    self.info['colinfo'][colnum]['tzero'])
        else:       
            colnumsp = colnums[:].copy()
            colnumsp[:] += 1
            self._FITS.read_columns_as_rec(self.ext+1, colnumsp, array, rows)
            
            for i in xrange(colnums.size):
                colnum = int(colnums[i])
                name = array.dtype.names[i]
                self._rescale_array(array[name], 
                                    self.info['colinfo'][colnum]['tscale'], 
                                    self.info['colinfo'][colnum]['tzero'])
        return array



    def _extract_rows(self, rows):
        if rows is not None:
            rows = numpy.array(rows, ndmin=1, copy=False, dtype='i8')
            # returns unique, sorted
            rows = numpy.unique(rows)

            maxrow = self.info['numrows']-1
            if rows[0] < 0 or rows[-1] > maxrow:
                raise ValueError("rows must be in [%d,%d]" % (0,maxrow))
        return rows

    def _rescale_array(self, array, scale, zero):
        if scale != 1.0:
            print 'rescaling array'
            array *= scale
        if zero != 0.0:
            print 're-zeroing array'
            array += zero

    def get_rec_dtype(self, colnums=None):
        if colnums is None:
            colnums = self._extract_colnums()

        dtype = []
        for colnum in colnums:
            dt = self.get_rec_column_dtype(colnum) 
            dtype.append(dt)
        return dtype

    def get_rec_column_dtype(self, colnum):
        """
        Need to incorporate TDIM information
        """
        npy_type = self._get_tbl_numpy_dtype(colnum)
        name = self.info['colinfo'][colnum]['ttype']
        tdim = self.info['colinfo'][colnum]['tdim']

        shape = tdim2shape(tdim, is_string=(npy_type[0] == 'S'))

        """
        # need to deal with string array columns
        if npy_type[0] == 'S':
            repeat=1
        else:
            repeat = self.info['colinfo'][colnum]['trepeat']
        """
        if shape is not None:
            return (name,npy_type,shape)
        else:
            return (name,npy_type)

    def _get_image_dtype_and_shape(self):

        if self.info['hdutype'] != _hdu_type_map['IMAGE_HDU']:
            raise ValueError("HDU is not an IMAGE_HDU")

        npy_dtype = self._get_image_numpy_dtype()

        if self.info['imgdim'] != 0:
            shape = self.info['imgnaxis']
        elif self.info['zndim'] != 0:
            shape = self.info['znaxis']
        else:
            raise ValueError("no image present in HDU")

        return npy_dtype, shape

    def _get_simple_dtype_and_shape(self, colnum, rows=None):
        """
        When reading a single column, we want the basic data
        type and the shape of the array.

        for scalar columns, shape is just nrows, otherwise
        it is (nrows, dim1, dim2)

        """

        # basic datatype
        npy_type = self._get_tbl_numpy_dtype(colnum)
        info = self.info['colinfo'][colnum]

        if rows is None:
            nrows = self.info['numrows']
        else:
            nrows = rows.size

        shape = None
        tdim = info['tdim']

        shape = tdim2shape(tdim, is_string=(npy_type[0] == 'S'))
        if shape is not None:
            if not isinstance(shape,tuple):
                # vector
                shape = (nrows,shape)
            else:
                # multi-dimensional
                shape = tuple( [nrows] + list(shape) )
        else:
            # scalar
            shape = nrows
        return npy_type, shape

    def _get_image_numpy_dtype(self):
        try:
            ftype = self.info['img_equiv_type']
            npy_type = _image_bitpix2npy[ftype]
        except KeyError:
            raise KeyError("unsupported fits data type: %d" % ftype)

        return npy_type

    def _get_tbl_numpy_dtype(self, colnum, include_endianness=True):
        try:
            ftype = self.info['colinfo'][colnum]['tdatatype']
            npy_type = _table_fits2npy[ftype]
        except KeyError:
            raise KeyError("unsupported fits data type: %d" % ftype)

        if include_endianness:
            if npy_type not in ['u1','i1','S']:
                npy_type = '>'+npy_type
        if npy_type == 'S':
            width = self.info['colinfo'][colnum]['twidth']
            npy_type = 'S%d' % width
        return npy_type

    def _extract_colnums(self, columns=None):
        if columns is None:
            return numpy.arange(self.ncol, dtype='i8')
        
        colnums = numpy.zeros(len(columns), dtype='i8')
        for i in xrange(colnums.size):
            colnums[i] = self._extract_colnum(columns[i])

        # returns unique sorted
        colnums = numpy.unique(colnums)
        return colnums

    def _extract_colnum(self, col):
        if isinstance(col,(int,long)):
            colnum = col

            if (colnum < 0) or (colnum > (self.ncol-1)):
                raise ValueError("column number should be in [0,%d]" % (0,self.ncol-1))
        else:
            try:
                colnum = self.colnames.index(col)
            except ValueError:
                raise ValueError("column name '%s' not found" % col)
        return int(colnum)

    def _update_info(self):
        # do this here first so we can catch the error
        try:
            self._FITS.moveabs_hdu(self.ext+1)
        except IOError:
            raise RuntimeError("no such hdu")

        self.info = self._FITS.get_hdu_info(self.ext+1)
        # convert to c order
        self.info['imgnaxis'] = list( reversed(self.info['imgnaxis']) )
        self.colnames = [i['ttype'] for i in self.info['colinfo']]
        self.ncol = len(self.colnames)

    def __repr__(self):
        spacing = ' '*2
        text = []
        text.append("%sHDU: %d" % (spacing,self.info['hdunum']))
        text.append("%stype: %s" % (spacing,_hdu_type_map[self.info['hdutype']]))
        
        if self.info['hdutype'] == _hdu_type_map['IMAGE_HDU']:
            text.append("%simage info:" % spacing)
            cspacing = ' '*4

            dimstr = [str(d) for d in self.info['imgnaxis']]
            dimstr = ",".join(dimstr)

            dt = _image_bitpix2npy[self.info['img_equiv_type']]
            text.append("%sdata type: %s" % (cspacing,dt))
            text.append("%sdims: [%s]" % (cspacing,dimstr))

        else:
            text.append('%scolumn info:' % spacing)

            cspacing = ' '*4
            nspace = 4
            nname = 15
            ntype = 6
            format = cspacing + "%-" + str(nname) + "s %" + str(ntype) + "s  %s"
            pformat = cspacing + "%-" + str(nname) + "s\n %" + str(nspace+nname+ntype) + "s  %s"

            for colnum,c in enumerate(self.info['colinfo']):
                if len(c['ttype']) > 15:
                    f = pformat
                else:
                    f = format

                dt = self._get_tbl_numpy_dtype(colnum, include_endianness=False)

                tdim = c['tdim']
                dimstr=''
                if dt[0] == 'S':
                    if len(tdim) > 1:
                        dimstr = [str(d) for d in tdim[1:]]
                else:
                    if len(tdim) > 1 or tdim[0] > 1:
                        dimstr = [str(d) for d in tdim]
                if dimstr != '':
                    dimstr = ','.join(dimstr)
                    dimstr = 'array[%s]' % dimstr

                s = f % (c['ttype'],dt,dimstr)
                text.append(s)

        text = '\n'.join(text)
        return text


def extract_filename(filename):
    if filename[0] == "!":
        filename=filename[1:]
    filename = os.path.expandvars(filename)
    filename = os.path.expanduser(filename)
    return filename

def tdim2shape(tdim, is_string=False):
    shape=None
    if len(tdim) > 1 or tdim[0] > 1:
        if is_string:
            shape = list( reversed(tdim[1:]) )
        else:
            shape = list( reversed(tdim) )

        if len(shape) == 1:
            shape = shape[0]
        else:
            shape = tuple(shape)

    return shape

def descr2tabledef(descr):
    """
    Create a FITS table def from the input numpy descriptor.

    parameters
    ----------
    descr: list
        A numpy recarray type descriptor  array.dtype.descr

    returns
    -------
    names, formats, dims: tuple of lists
        These are the ttyp, tform and tdim header entries
        for each field.  dim entries may be None
    """
    names=[]
    formats=[]
    dims=[]

    for d in descr:
        # these have the form '<f4' or '|S25', etc.  Extract the pure type
        npy_dtype = d[1][1:]
        if npy_dtype[0] == 'S':
            name, form, dim = npy_string2fits(d)
        else:
            name, form, dim = npy_num2fits(d)

        names.append(name)
        formats.append(form)
        dims.append(dim)

    return names, formats, dims

def npy_num2fits(d):
    """
    d is the full element from the descr

    For vector,array columns the form is the total counts
    followed by the code.

    For array columns with dimension greater than 1, the dim is set to
        (dim1, dim2, ...)
    So it is treated like an extra dimension
        
    """

    dim = None

    name = d[0]

    npy_dtype = d[1][1:]
    if npy_dtype[0] == 'S':
        raise ValueError("got S type: use npy_string2fits")

    if npy_dtype not in _table_npy2fits_form:
        raise ValueError("unsupported type '%s'" % npy_dtype)
    form = _table_npy2fits_form[npy_dtype]

    # now the dimensions
    if len(d) > 2:
        if isinstance(d[2], tuple):
            # this is an array column.  the form
            # should be total elements followed by A
            #count = 1
            #count = [count*el for el in d[2]]
            count=reduce(lambda x, y: x*y, d[2])
            form = '%d%s' % (count,form)

            # will have to do tests to see if this is the right order
            dim = list(reversed(d[2]))
            #dim = d[2]
            dim = [str(e) for e in dim]
            dim = '(' + ','.join(dim)+')'
        else:
            # this is a vector (1d array) column
            count = d[2]
            form = '%d%s' % (count,form)

    return name, form, dim


def npy_string2fits(d):
    """
    d is the full element from the descr

    form for strings is the total number of bytes followed by A.  Thus
    for vector or array columns it is the size of the string times the
    total number of elements in the array.

    Then the dim is set to
        (sizeofeachstring, dim1, dim2, ...)
    So it is treated like an extra dimension
        
    """

    dim = None

    name = d[0]

    npy_dtype = d[1][1:]
    if npy_dtype[0] != 'S':
        raise ValueError("expected S type")

    # get the size of each string
    string_size_str = npy_dtype[1:]
    string_size = int(string_size_str)

    # now the dimensions
    if len(d) == 2:
        form = string_size_str+'A'
    else:
        if isinstance(d[2], tuple):
            # this is an array column.  the form
            # should be total elements followed by A
            #count = 1
            #count = [count*el for el in d[2]]
            count=reduce(lambda x, y: x*y, d[2])
            count = string_size*count
            form = '%dA' % count

            # will have to do tests to see if this is the right order
            dim = list(reversed(d[2]))
            #dim = d[2]
            dim = [string_size_str] + [str(e) for e in dim]
            dim = '(' + ','.join(dim)+')'
        else:
            # this is a vector (1d array) column
            count = string_size*d[2]
            form = '%dA' % count

            # will have to do tests to see if this is the right order
            dim = [string_size_str, str(d[2])]
            dim = '(' + ','.join(dim)+')'

    return name, form, dim

 


READONLY=0
READWRITE=1
IMAGE_HDU=0
ASCII_TBL=1
BINARY_TBL=2

_modeprint_map = {'r':'READONLY','rw':'READWRITE', 0:'READONLY',1:'READWRITE'}
_char_modemap = {'r':'r','rw':'rw', 
                 READONLY:'r',READWRITE:'rw'}
_int_modemap = {'r':READONLY,'rw':READWRITE, READONLY:READONLY, READWRITE:READWRITE}
_hdu_type_map = {IMAGE_HDU:'IMAGE_HDU',
                 ASCII_TBL:'ASCII_TBL',
                 BINARY_TBL:'BINARY_TBL',
                 'IMAGE_HDU':IMAGE_HDU,
                 'ASCII_TBL':ASCII_TBL,
                 'BINARY_TBL':BINARY_TBL}

# no support yet for complex
_table_fits2npy = {11:'u1',
                   12: 'i1',
                   14: 'i1', # logical. Note pyfits uses this for i1, cfitsio casts to char*
                   16: 'S',
                   20: 'u2',
                   21: 'i2',
                   30: 'u4',
                   31: 'i4',
                   40: 'u4',
                   41: 'i4',
                   42: 'f4',
                   81: 'i8',
                   82: 'f8'}

# for TFORM
# note actually there are no unsigned, they get scaled
# and converted to signed.  When reading, can only do signed.
_table_npy2fits_form = {'u1':'B',
                        'i1':'S', # gets converted to unsigned
                        'S' :'A',
                        'u2':'U', # gets converted to signed
                        'i2':'I',
                        'u4':'V', # gets converted to signed
                        'i4':'J',
                        'i8':'K',
                        'f4':'E',
                        'f8':'D'}

# remember, you should be using the equivalent image type for this
_image_bitpix2npy = {8: 'u1',
                     10: 'i1',
                     16: 'i2',
                     20: 'u2',
                     32: 'i4',
                     40: 'u4',
                     64: 'i8',
                     -32: 'f4',
                     -64: 'f8'}

def test_create():
    fname='test-write.fits'
    mode='rw'

    fits1 = FITS(fname,mode,create=True,clobber=True)
    try:
        fits2 = FITS(fname,mode,create=True)
    except IOError:
        print 'Caught expected exception on existing file'

    if os.path.exists(fname):
        os.remove(fname)

def test_write_table():
    fname='test-write-table.fits'
    dtype=[('f','f4'),
           ('fvec','f4',2),
           ('darr','f8',(2,3)),#] 
           ('s','S8'),
           ('svec','S12',3),
           ('sarr','S5',(3,4))]
    #dtype=[('s','S8'),
    #       ('svec','S12',3)]

    nrows=4
    data=numpy.zeros(4, dtype=dtype)

    if 'f' in data.dtype.names:
        data['f'] = 1 + numpy.arange(nrows, dtype='f4')
    if 'fvec' in data.dtype.names:
        data['fvec'] = 1 + numpy.arange(nrows*2,dtype='f4').reshape(nrows,2)
    if 'darr' in data.dtype.names:
        data['darr'] = 1 + numpy.arange(nrows*2*3,dtype='f8').reshape(nrows,2,3)
    if 's' in data.dtype.names:
        data['s'] = ['hello','world','and','bye']
    if 'svec' in data.dtype.names:
        data['svec'][:,0] = 'hello'
        data['svec'][:,1] = 'there'
        data['svec'][:,2] = 'world'
        print 'svec shape:',data['svec'].shape
    if 'sarr' in data.dtype.names:
        s = 1 + numpy.arange(nrows*3*4)
        s = [str(el) for el in s]
        data['sarr'] = numpy.array(s).reshape(nrows,3,4)

    print data
    print 'writing to:',fname
    with FITS(fname,'rw',create=True,clobber=True) as fits:
        fits.write_table(data)

    with FITS(fname,'r') as fits:

        if 'f' in data.dtype.names:
            print 'f:',fits[-1].read_column('f')

        if 'fvec' in data.dtype.names:
            print 'fvec:',fits[-1].read_column('fvec')
        if 'darr' in data.dtype.names:
            print 'darr:',fits[-1].read_column('darr')
        if 's' in data.dtype.names:
            print 's:',fits[-1].read_column('s')
        if 'svec' in data.dtype.names:
            print 'svec:',fits[-1].read_column('svec')
        if 'sarr' in data.dtype.names:
            print 'sarr:',fits[-1].read_column('sarr')
        return fits[-1].read()

def test_write_new_table_old(type=BINARY_TBL):
    fname='test-write-table.fits'
    with FITS(fname,'rw',create=True,clobber=True) as fits:
        fits._FITS.test_write_new_table()
        fits.reopen()
        print fits
        print fits[1]

        print 'Reading all rows'
        data = fits[1].read()
        print data
        data = fits[1].read_columns(['Planet','Diameter'])
        print data
        data = fits[1].read_column('Planet')
        print data

        rows = [1,3]
        print '\nReading rows',rows
        data = fits[1].read(rows=rows)
        print data
        data = fits[1].read_columns(['Planet','Diameter'], rows=rows)
        print data
        data = fits[1].read_column('Planet', rows=rows)
        print data


    return

def test_write_image(dtype):
    fname='test-write.fits'
    mode='rw'

    nx = 5
    ny = 3

    img = numpy.arange(nx*ny, dtype=dtype).reshape(nx,ny)
    print 'writing image:'
    print img
    fits = FITS(fname,mode,create=True,clobber=True)

    fits.write_image(img)

    print fits
    print fits[0]

    imgread = fits[0].read()
    print 'read image:'
    print imgread

    maxdiff = numpy.abs( (img-imgread) ).max()
    print 'maxdiff:',maxdiff
    if maxdiff > 0:
        raise ValueError("Found differences")
