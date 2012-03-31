module healpix;
import math;

class Healpix {
    int64 nside;
    int64 npix;
    int64 ncap; // number of pixels in the north polar cap
    double area;

    this(int64 nside0) {
        nside = nside0;
        npix = 12*nside*nside;
        area = 4.0*PI/npix;
        ncap = 2*nside*(nside-1); 
    }

}
