/* vim: set ft=vala : */
using Gee;
using Math;

public class CatPoint : Point {
    // we inherit x,y,z from Point
    public long index;
    public double cos_radius;

    public CatPoint(double ra, double dec, long index, double cos_radius) {
        this.index=index;
        this.cos_radius = cos_radius;
        set_radec(ra,dec);
    }
}

public class Match : GLib.Object {
    public long index;
    public double cos_dist;

    public Match(long index, double cos_dist) {
        this.index=index;
        this.cos_dist=cos_dist;
    }
}
public int match_compare(Match* m1, Match* m2) {
    if (m1->cos_dist > m2->cos_dist) {
        return -1;
    } else if (m1->cos_dist < m2->cos_dist) {
        return 1;
    } else {
        return 0;
    }
}


public class Cat : GLib.Object {

    // a dictionary keyed by longs (htmid) and with values a stack of Points
    //ArrayList<long> pdict;
    HashMap<long, ArrayList<CatPoint> > pdict 
        = new HashMap<long, ArrayList<CatPoint> >();
    Healpix hpix = new Healpix();

    public Cat(string filename, long nside, double rad_arcsec) {
        double ra=0,dec=0,rad_radians=0,cos_radius=0;
        //pdict = new HashMap<long, ArrayList<CatPoint> >();
        //hpix = new Healpix(nside);
        hpix.set_nside(nside);
        var pixlist = new ArrayList<long>();
        var file = FileStream.open(filename,"r");

        bool rad_in_file=_radius_check(rad_arcsec,&rad_radians,&cos_radius);

        long index=0;
        ArrayList<CatPoint> tmp;
        while (2==file.scanf("%lf %lf", &ra, &dec)) {
            //stderr.printf("%.16g %.16g\n", ra, dec);
            if (rad_in_file) {
                file.scanf("%lf", &rad_arcsec);
                rad_radians = rad_arcsec/3600.0*D2R;
                cos_radius = cos(rad_radians);
            }

            var pt = new CatPoint(ra,dec,index,cos_radius);
            //var pth = new HPoint.from_radec(ra,dec);
            //var pixelof = hpix.pixelof(pth); 
            //bool found=false;

            hpix.disc_intersect(pt, rad_radians, pixlist);
            foreach (long pix in pixlist) {
                //if (pix == pixelof) found=true;
                //stderr.printf("pix: %ld\n", pix);
                tmp = pdict[pix];
                if (tmp == null) {
                    tmp = new ArrayList<CatPoint>();
                    tmp.add(pt);
                    pdict[pix] = tmp;
                } else {
                    tmp.add(pt);
                }
            }

            /*
            if (found=false) {
                stderr.printf("Didn't find my own pixel!\n");
            }
            if (!(pixelof in pdict)) {
                stderr.printf("Didn't find my own pixel 2!\n");
            }
            */
            index++;
        }
    }

    public void match(HPoint pt, ArrayList<Match> matches) {
        matches.clear();
        var hpixid = hpix.pixelof(pt);
        //stderr.printf("hpixid: %ld\n", hpixid);
        var entry = pdict[hpixid];
        if (entry != null) {
            //stderr.printf("yes1\n");
            foreach (var cat_point in entry) {
                double cos_angle = cat_point.dot(pt);
                if (cos_angle > cat_point.cos_radius) {
                    //stderr.printf("yes2\n");
                    matches.add(new Match(cat_point.index,cos_angle));
                }
            }
        }
    }


    public void print_counts() {
        foreach (var entry in pdict.entries) {
            // fucking idiots made size an int
            stderr.printf("%ld %d\n", entry.key, entry.value.size);
        }
    }

    private bool _radius_check(double radius_arcsec, 
                              double* rad_radians,
                              double* cos_radius)
    {
        bool radius_in_file=false;
        if (radius_arcsec <= 0) {
            radius_in_file=true;
        } else {
            *rad_radians = radius_arcsec/3600.0*D2R;
            *cos_radius = cos(*rad_radians);
        }
        return radius_in_file;
    }

}

