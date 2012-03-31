module point;
import std.math;

enum real D2R = PI/180.;
enum real R2D = 180.0/PI;

class Point {
    double x;
    double y;
    double z;

    this() {}
    this(double x, double y, double z) { 
        set(x,y,z);
    }
    this(double ra, double dec) {
        set(ra,dec);
    }
    void set(double x, double y, double z) { 
        this.x=x;
        this.y=y;
        this.z=z;
    }
    void set(double ra, double dec) {
        double cdec;

        ra *= D2R;
        dec *= D2R;

        cdec = cos(dec);
        x = cdec*cos(ra);
        y = cdec*sin(ra);
        z = sin(dec);
    }

    double dot(in Point p) {
        return this.x*p.x + this.y*p.y + this.z*p.z;
    }

    @property double phi() {
        double t = atan2(y,x) - PI;
        return t;
    }
}

