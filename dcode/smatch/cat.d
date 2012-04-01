module cat;
import std.stdio;
import std.c.stdio;
import std.math;
import point;
import stack;
import healpix;
class CatPoint : Point {
    // we inherit x,y,z from Point
    long index;
    double cos_radius;

    this(double ra, double dec, long index, double cos_radius) {
        this.index=index;
        this.cos_radius = cos_radius;
        super(ra,dec);
    }
}


class Cat {

    // a dictionary keyed by longs (htmid) and with values a stack of Points
    Stack!(CatPoint)[long] pdict;
    Healpix hpix;

    this(string filename, long nside, double rad_arcsec) {
        File* file;
        double ra,dec,rad_radians,cos_radius;
        file = new File(filename,"r");

        int rad_in_file=_radius_check(rad_arcsec,&rad_radians,&cos_radius);

        Stack!(long) pixlist;
        hpix = new Healpix(nside);
        writefln("area: %.10g linscale(arcsec): %.10g", 
                 hpix.area,sqrt(hpix.area)*R2D*3600.);
        writefln("rad(arcsec): %s rad(radians): %s",rad_arcsec,rad_radians);
        long index=0;
        while (2 == fscanf(file.getFP(),"%lf %lf\n", &ra, &dec)) {

            if (rad_in_file) {
                fscanf(file.getFP(),"%lf", &rad_arcsec);
                rad_radians = rad_arcsec/3600.*D2R;
                cos_radius = cos(rad_radians);
            }

            auto p = new CatPoint(ra,dec,index,cos_radius);
            hpix.disc_intersect(p, rad_radians, &pixlist);

            for (size_t i=0; i<pixlist.length; i++) {
                long pix=pixlist[i];

                auto cps = (pix in pdict);
                if (cps) {
                    cps.push(p);
                } else {
                    pdict[pix] = Stack!CatPoint();
                    pdict[pix].push(p);
                }
            }
            index++;
        }
    }

    void print_counts() {
        foreach (pix, cps; pdict) {
            writefln("%s %s", pix, cps.length);
        }
    }
    int _radius_check(double radius_arcsec, 
                      double* rad_radians,
                      double* cos_radius)
    {
        int radius_in_file=0;
        if (radius_arcsec <= 0) {
            radius_in_file=1;
        } else {
            radius_in_file=0;
            *rad_radians = radius_arcsec/3600.*D2R;
            *cos_radius = cos(*rad_radians);
        }
        return radius_in_file;
    }

}

unittest
{
    long nside=4096;
    double rad_arcsec=2; 
    string f="/home/esheldon/tmp/rand-radec-10000.dat";
    auto c = new Cat(f,nside,rad_arcsec);

    //c.print_counts();
}
