/* vim: set ft=vala : */

using Math;

const double D2R = PI/180.0;
const double R2D = 180.0/PI;

class Point : GLib.Object {
    public double x;
    public double y;
    public double z;

    public Point() {}
    public Point.from_xyz(double x, double y, double z) {
        this.x=x;
        this.y=y;
        this.z=z;
    }
    public Point.from_radec(double ra, double dec) {
        set_radec(ra,dec);
    }
    public void set_xyz(double x, double y, double z) {
        this.x=x;
        this.y=y;
        this.z=z;
    }
    public void set_radec(double ra, double dec) {
        double cdec;

        ra *= D2R;
        dec *= D2R;

        cdec = cos(dec);
        x = cdec*cos(ra);
        y = cdec*sin(ra);
        z = sin(dec);
    }

    public double dot(Point p) {
        return this.x*p.x + this.y*p.y + this.z*p.z;
    }

    /* Property */
    public double phi {
        get { return atan2(y,x); }
    }
}

