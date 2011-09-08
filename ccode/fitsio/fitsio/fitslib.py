"""
Advantages:

    - Can read arbitrary subsets of columns and rows without loading the whole
    file.
    - Uses TDIM information to return array columns in the correct shape
    - Can write unsigned types.  Note the FITS standard does not support
    unsigned 8 byte yet.
    - Correctly writes 1 byte integers, both signed and unsigned.

"""
import numpy
import _fitsio_wrap

class FITS:
    def __init__(self, filename, mode):
        self.filename = filename
        if mode not in _int_modemap:
            raise ValueError("mode should be 'r','rw',0,1")
        self.charmode = _char_modemap[mode]
        self.intmode = _int_modemap[mode]

        self._FITS =  _fitsio_wrap.FITS(filename, self.intmode)

    def close(self):
        self._FITS.close()

    def update_hdu_list(self):
        self._hdu_list = []
        for ext in xrange(1000):
            try:
                hdu = FITSHDU(self._FITS, ext)
                self._hdu_list.append(hdu)
            except RuntimeError:
                break


    def moveabs_ext(self, ext):
        self._FITS.moveabs_hdu(ext+1)

    def __getitem__(self, ext):
        if not hasattr(self, '_hdu_list'):
            self.update_hdu_list()

        n_ext = len(self._hdu_list)
        if isinstance(ext,(int,long)):
            if (ext < 0) or (ext > (n_ext-1)):
                raise ValueError("extension number %s out of "
                                 "bounds [%d,%d]" % (ext,0,n_ext-1))
        else:
            raise ValueError("don't yet support getting "
                             "extensions by name")
        return self._hdu_list[ext]


    def __repr__(self):
        spacing = ' '*2
        if not hasattr(self, '_hdu_list'):
            self.update_hdu_list()

        rep = []
        rep.append("%sfile: %s" % (spacing,self.filename))
        rep.append("%smode: %s" % (spacing,_modeprint_map[self.intmode]))
        for i,hdu in enumerate(self._hdu_list):
            t = hdu.info['hdutype']
            rep.append("%sHDU%d: %s" % (spacing,(i+1), _hdu_type_map[t]))

        rep = '\n'.join(rep)
        return rep

class FITSHDU:
    def __init__(self, fits, ext):
        """
        A representation of a FITS HDU

        parameters
        ----------
        fits: FITS object
            An instance of a FITS objects
        ext:
            The extension number.
        """
        self._FITS = fits
        self.ext = ext
        self._update_info()

    def _update_info(self):
        # do this here first so we can catch the error
        try:
            self._FITS.moveabs_hdu(self.ext+1)
        except IOError:
            raise RuntimeError("no such hdu")

        self.info = self._FITS.get_hdu_info(self.ext+1)
        self.colnames = [i['ttype'] for i in self.info['colinfo']]
        self.ncol = len(self.colnames)

    def read_column(self, col):
        if self.info['hdutype'] == _hdu_type_map['IMAGE_HDU']:
            raise ValueError("Cannot yet read columns from an image HDU")

        colnum = self._extract_colnum(col)

        npy_type, shape = self._extract_simple_dtype_and_shape(colnum)

        array = numpy.zeros(shape, dtype=npy_type)

        self._FITS.read_column(self.ext+1,colnum+1, array)
        
        self._rescale_array(array, 
                            self.info['colinfo'][colnum]['tscale'], 
                            self.info['colinfo'][colnum]['tzero'])
        return array

    def _rescale_array(self, array, scale, zero):
        if scale != 1.0:
            print 'rescaling array'
            array *= scale
        if zero != 0.0:
            print 're-zeroing array'
            array += zero

    def _extract_simple_dtype_and_shape(self, colnum):
        try:
            ftype = self.info['colinfo'][colnum]['tdatatype']
            npy_type = _typemap[ftype]
        except KeyError:
            raise KeyError("unsupported fits data type: %d" % ftype)

        repeat = self.info['colinfo'][colnum]['trepeat']
        if repeat > 1:
            shape = (self.info['numrows'], repeat)
        else:
            shape = self.info['numrows']

        return npy_type, shape

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
        return colnum

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

            text.append("%sdata type: %d" % (cspacing,self.info['img_equiv_type']))
            text.append("%sdims: [%s]" % (cspacing,dimstr))

        else:
            text.append('%scolumn info:' % spacing)

            cspacing = ' '*4
            nspace = 4
            nname = 15
            ntype = 6
            format = cspacing + "%-" + str(nname) + "s %" + str(ntype) + "s  %s"
            pformat = cspacing + "%-" + str(nname) + "s\n %" + str(nspace+nname+ntype) + "s  %s"

            for c in self.info['colinfo']:
                if len(c['ttype']) > 15:
                    f = pformat
                else:
                    f = format
                if c['trepeat'] > 1:
                    rep = 'array[%d]' % c['trepeat']
                else:
                    rep=''
                dt = _typemap[c['tdatatype']]
                s = f % (c['ttype'],dt,rep)
                text.append(s)

        text = '\n'.join(text)
        return text


_modeprint_map = {'r':'READONLY','rw':'READWRITE', 0:'READONLY',1:'READWRITE'}
_char_modemap = {'r':'r','rw':'rw', 0:'r',1:'rw'}
_int_modemap = {'r':0,'rw':1, 0:0,1:1}
_hdu_type_map = {0:'IMAGE_HDU',
                 1:'ASCII_TBL',
                 2:'BINARY_TBL',
                 'IMAGE_HDU':0,
                 'ASCII_TBL':1,
                 'BINARY_TBL':2}

# no support yet for logical or complex
_typemap = {11:'u1', 'u1':11,
            12: 'i1', 'i1': 12,
            14: 'i1', 'i1': 14, # logical: correct?
            16: 'S', 'S': 16,
            20: 'u2', 'u2':20,
            21: 'i2', 'i2':21,
            30: 'u4', 'u4': 30,
            31: 'i4', 'i4': 31,
            40: 'u4', 'u4': 40, # these are "long" but on linux same as int...
            41: 'i4', 'i4': 41, # need to be more careful here
            42: 'f4', 'f4': 42,
            81: 'i8', 'i8': 81,
            82: 'f8', 'f8': 82}





            
