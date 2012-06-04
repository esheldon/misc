import _mangle
__doc__=_mangle.__doc__

# we only over-ride the init class to deal with verbose
class Mangle(_mangle.Mangle):
    __doc__=_mangle.Mangle.__doc__
    def __init__(self, filename, verbose=False):
        if verbose:
            verb=1
        else:
            verb=0
        super(Mangle,self).__init__(filename,verb)
