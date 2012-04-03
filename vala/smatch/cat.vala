/* vim: set ft=vala : */
using Gee;
using Math;

class CatPoint : Point {
    // we inherit x,y,z from Point
    public long index;
    public double cos_radius;

    public CatPoint(double ra, double dec, long index, double cos_radius) {
        this.index=index;
        this.cos_radius = cos_radius;
        set_radec(ra,dec);
    }
}

class Cat : GLib.Object {

    // a dictionary keyed by longs (htmid) and with values a stack of Points
    ArrayList<long> pdict;
    Healpix hpix;

    public Cat(string filename, long nside, double rad_arcsec) {
        double ra=0,dec=0,rad_radians=0,cos_radius=0;
        int rad_in_file=_radius_check(rad_arcsec,&rad_radians,&cos_radius);

        var file = FileStream.open(filename,"r");
        while (2==file.scanf("%lf %lf", &ra, &dec)) {
            stderr.printf("%.16g %.16g\n", ra, dec);
        }
    }

    private int _radius_check(double radius_arcsec, 
                              double* rad_radians,
                              double* cos_radius)
    {
        int radius_in_file=0;
        if (radius_arcsec <= 0) {
            radius_in_file=1;
        } else {
            radius_in_file=0;
            *rad_radians = radius_arcsec/3600.0*D2R;
            *cos_radius = cos(*rad_radians);
        }
        return radius_in_file;
    }

}

