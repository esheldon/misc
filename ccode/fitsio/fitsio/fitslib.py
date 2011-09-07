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

    def __getitem__(self, ext):
        if not hasattr(self, '_hdu_info'):
            self._update_hdu_info()

        n_ext = len(self._hdu_info)
        if isinstance(ext,(int,long)):
            if (ext < 0) or (ext > (n_ext-1)):
                raise ValueError("extension number %s out of "
                                 "bounds [%d,%d]" % (ext,0,n_ext-1))
        else:
            raise ValueError("don't yet support getting "
                             "extensions by name")
        return ext

    def _update_hdu_info(self):
        self._hdu_info = self._FITS.get_hdu_info()

    def __repr__(self):
        if not hasattr(self, '_hdu_info'):
            self._update_hdu_info()
        repr="    file: %s\n" % self.filename
        repr+="    mode: %s\n" % _modeprint_map[self.intmode]
        for i,t in enumerate(self._hdu_info):
            repr+="    HDU%d: %s\n" % ((i+1), _hdu_type_map[t])

        return repr

_modeprint_map = {'r':'READONLY','rw':'READWRITE', 0:'READONLY',1:'READWRITE'}
_char_modemap = {'r':'r','rw':'rw', 0:'r',1:'rw'}
_int_modemap = {'r':0,'rw':1, 0:0,1:1}
_hdu_type_map = {0:'IMAGE_HDU',
                 1:'ASCII_TBL',
                 2:'BINARY_TBL'}
