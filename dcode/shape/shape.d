module shape;

import std.stdio;
import std.math;

class ShapeRangeError: Exception {
   this (string msg) {
       super(msg) ;
   }
}


enum ShapeSystem {
    SHAPE_SYSTEM_ETA,
    SHAPE_SYSTEM_G,
    SHAPE_SYSTEM_E
};

struct Shape {
    double g1;
    double g2;

    double e1;
    double e2;

    double eta1;
    double eta2;

    void setbad()
    {
        this.e1=-9999;
        this.e2=-9999;
        this.g1=-9999;
        this.g2=-9999;
        this.eta1=-9999;
        this.eta2=-9999;
    }

    long set_g(double g1, double g2) {
        this.g1=g1;
        this.g2=g2;
        auto g=sqrt(g1*g1 + g2*g2);
        if (g==0) {
            this.e1=0;
            this.e2=0;
            this.eta1=0;
            this.eta2=0;
            return 1;
        } 

        if (g >= 1) {
            //throw new ShapeRangeError(format("error: g must be < 1, found %.16g.",g));
            this.setbad();
            return 0;
        }

        auto eta = 2*atanh(g);
        auto e = tanh(eta);

        if (e >= 1) {
            //throw new ShapeRangeError(format("error: e must be < 1, found %.16g.",e));
            this.setbad();
            return 0;
        }

        auto cos2theta = g1/g;
        auto sin2theta = g2/g;

        this.e1=e*cos2theta;
        this.e2=e*sin2theta;
        this.eta1=eta*cos2theta;
        this.eta2=eta*sin2theta;

    }
};


