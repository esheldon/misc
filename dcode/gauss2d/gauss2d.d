module gauss2;

import std.stdio;
import std.math;

class Gauss2DDetError: Exception {
   this (string msg) {
       super(msg) ;
   }
}

struct Gauss2D {
    double p;
    double row;
    double col;
    double irr;
    double irc;
    double icc;

    // derived quantities
    double det;

    double drr; // irr/det
    double drc;
    double dcc;

    double norm; // 1/( 2*pi*sqrt(det) )

    double pnorm; // p*norm

    this(double p,
         double row,
         double col,
         double irr,
         double irc,
         double icc)
    {
        set(p,row,col,irr,irc,icc);
    }

    void set(double p,
             double row,
             double col,
             double irr,
             double irc,
             double icc)
    {
        this.det = irr*icc - irc*irc;

        if (this.det <= 0) {
            throw new Gauss2DDetError("negative det");
        }

        this.p   = p;
        this.row = row;
        this.col = col;
        this.irr = irr;
        this.irc = irc;
        this.icc = icc;

        auto idet=1.0/this.det;

        this.drr = this.irr*idet;
        this.drc = this.irc*idet;
        this.dcc = this.icc*idet;
        this.norm = 1./(2.0*PI*sqrt(this.det));

        this.pnorm = p*this.norm;

    }

    void print(File* stream) {
        stream.writef("  p:   %.16g\n", this.p);
        stream.writef("  row: %.16g\n", this.row);
        stream.writef("  col: %.16g\n", this.col);
        stream.writef("  irr: %.16g\n", this.irr);
        stream.writef("  irc: %.16g\n", this.irc);
        stream.writef("  icc: %.16g\n", this.icc);
    }

}

unittest 
{
    Gauss2D g = Gauss2D(1.0, 15.0, 16.2, 4.0, 0.1, 4.2);

    g.print(&stdout);
}
