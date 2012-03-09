import std.math;

class Point {
    real x;
    real y;
    real z;
    real ra;
    real dec;

    this(real ra, real dec) {
        real cdec;

        this.ra = ra;
        this.dec = dec;

        ra *= PI/180.;
        dec *= PI/180.;

        cdec = cos(dec);
        this.x = cdec*cos(ra);
        this.y = cdec*sin(ra);
        this.z = sin(dec);
    }
    this(real x, real y, real z) {
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
    real dot(Point p) {
        return this.x*p.x + this.y*p.y + this.z*p.z;
    }

}


