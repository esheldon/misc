import std.math;
import point;

class HPoint : Point {
    // we inherit x,y,z
    double phi;
    double theta;

    this(double ra, double dec) {
        set(ra,dec);
    }
    void set(double ra, double dec) {
        // note this gives the same x,y,z as for the superclass Point
        phi = ra*D2R;
        theta = PI_2 -dec*D2R;

        double sintheta = sin(theta);
        x = sintheta * cos(phi);
        y = sintheta * sin(phi);
        z = cos(theta);
    }
}
