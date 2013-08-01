module gmix;

import gmix;
import shape;

enum GMixModel {
    GMIX_FULL,
    GMIX_COELLIP,
    GMIX_TURB,
    GMIX_EXP,
    GMIX_DEV,
    GMIX_BD
}

struct GMixPars {
    GMixModel model;

    ShapeSystem shape_system;

    double[] data;

    // not used by GMIX_FULL
    Shape shape;
};


struct GMix {
    Gauss2D[] data;

    this(long n) {
        this.data.length = n;
    }
}

