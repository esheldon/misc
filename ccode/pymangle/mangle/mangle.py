import _mangle

class Mangle(_mangle.Mangle):
    """
    Mangle routines
    """
    def __init__(self, filename, verbose=False):
        if verbose:
            verb=1
        else:
            verb=0
        super(Mangle,self).__init__(filename,verb)
