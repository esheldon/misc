import std.math;
import Typedef;

class Point {
    ftype x;
    ftype y;
    ftype z;
    ftype ra;
    ftype dec;

    this(ftype ra, ftype dec) {
        ftype cdec;

        this.ra = ra;
        this.dec = dec;

        ra *= PI/180.;
        dec *= PI/180.;

        cdec = cos(dec);
        this.x = cdec*cos(ra);
        this.y = cdec*sin(ra);
        this.z = sin(dec);
    }
    this(ftype x, ftype y, ftype z) {
        this.x=x;
        this.y=y;
        this.z=z;

        this.ra = 180./PI*atan2(this.y, this.x);
        this.dec = 180./PI*asin(this.z);

        while (this.ra < 0.) {
            this.ra += 360.;
        }
        while (this.ra > 360.) {
            this.ra -= 360.;
        }
    }
    ftype dot(in Point p) {
        return this.x*p.x + this.y*p.y + this.z*p.z;
    }

}


