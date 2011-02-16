// this doesn't seem to matter
//module point;

import std.exception;
import std.math;


class HCube {

    int ndim;
    double[] low;
    double[] high;

    this(int ndim) {
        this.ndim = ndim;
        low.length = ndim;
        high.length = ndim;

        low[] = 0;
        high[] = 0;
    }
    this(double[] low, double[] high) {
        enforce(low.length == high.length,
                "low and high points have the same dimensions");

        this.ndim = low.length;
        this.low.length = low.length;
        this.high.length = low.length;

        this.low[] = low[];
        this.high[] = high[];
    }

    // aggregate quadratic distance outside the bounds.
    // it is done dimension by dimension.
    double dist(double[] p) {
        enforce(p.length == high.length,
                "input point has the wrong dimensions");

        double sum=0.0;
        for (size_t dim=0; dim<low.length; dim++) {
            if (p[dim] < low[dim]) {
                sum +=  (low[dim]-p[dim])^^2;
            }
            if (p[dim] > high[dim]) {
                sum += (p[dim]-high[dim])^^2;
            }
        }
        return sqrt(sum);
    }

    bool contains(double[] p) {
        enforce(p.length == high.length,
                "input point has the wrong dimensions");
        if (dist(p) > 0) {
            return false;
        } else {
            return true;
        }
    }
}

struct SourceCatalog {

    // the z vals corresponding to the Source.scinv vals
    // the length of this must be determined from the
    // input file or a config file
    double[] zlens_vals;

    Source[] sources;

    // need a "interpolate scinv" method
    float interp_scinv(float zlens) {

    }

    void read(string filename) {

    }

}

struct Source {
    double clambda;
    double ceta;

    float e1;
    float e2;
    float err;

    int htm_index;

    // expectation value of the inverse critical density
    // as a function of lens redshift
    float[] scinv;
}

struct Lens {
    double ra;
    double dec;
    float z;
}

// this is kind of pointless
class Point {
    private int _ndim;
    private double[] _x;

    // constructor from the number of dimensions
    this(int n_dim) {
        _ndim = n_dim;
        _x.length = n_dim;
        _x[] = 0;
    }

    // constructor from input data.  Dimensions will be determined
    // from that data.  Note the z value is optional defaulting to 0.
    this(double[] xin) {
        _x.length = xin.length;
        _x[] = xin[];
    }

    int ndim() {
        return _ndim;
    }

    double opIndex(int i) {
        return _x[i];
    }
    double opIndexAssign(double value, int i) {
        _x[i] = value;
        return value;
    }

    void copyFrom(Point p) {
        // we don't want points to change their dimensionality *ever*
        enforce(p.ndim() == _ndim);
        for (int i=0; i<_ndim; i++) {
            _x[i] = p[i];
        }
    }

}

struct sPoint(const int NDIM) {
    double[NDIM] x;
    double z=0;
    const int ndim=NDIM;

    // constructor from input data.  Dimensions will be determined
    // from that data.  Note the z value is optional defaulting to 0.
    /*
    this(double[] xin, double zin=0) {
        enforce(xin.length == ndim);
        x[] = xin[];
        z = zin;
    }
    */

    /*
    sPoint opAssign(sPoint p) {
        enforce(p.x.length == x.length);
        x[] = p.x[];
        z = p.z;
        return this;
    }
    */
}


/*
class Point(int NDIM) {
    double[NDIM] x;
    int ndim=NDIM;
}
*/

class Bar(T)
{
    T member;
}

