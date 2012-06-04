module healpix;
import std.stdio;
import std.math;
import hpoint;
import point; // for some constants
import stack;


enum real M_TWO_PI = 6.28318530717958647693; /* 2*pi */
enum real M_TWOTHIRD = 0.66666666666666666666;

class Healpix {
    long nside;
    long npix;
    long ncap; // number of pixels in the north polar cap
    double area;

    this(long nside_) {
        nside = nside_;
        npix = 12*nside*nside;
        area = 4.0*PI/npix;
        ncap = 2*nside*(nside-1); 
    }

    long pixelof(double ra, double dec) {
        auto p = new HPoint(ra,dec);
        return pixelof(p);
    }
    long pixelof(in HPoint p) {
        long ipix=0;
        double za = fabs(p.z);

        // in [0,4)
        double tt = fmod(p.phi, M_TWO_PI)/PI_2;

        if (za <= M_TWOTHIRD) {
            double temp1 = this.nside*(.5 + tt);
            double temp2 = this.nside*.75*p.z;

            long jp = cast(long)(temp1-temp2); // index of  ascending edge line
            long jm = cast(long)(temp1+temp2); // index of descending edge line
            // in {1,2n+1} (ring number counted from z=2/3)
            long ir = this.nside + 1 + jp - jm;  
            long kshift = 1 - (ir % 2);      // kshift=1 if ir even, 0 otherwise

            long nl4 = 4*this.nside;
            // in {0,4n-1}
            long ip = cast(long)( ( jp+jm - this.nside + kshift + 1 ) / 2); 

            ip = ip % nl4;

            ipix = this.ncap + nl4*(ir-1) + ip;

        } else {
            // North & South polar caps
            double tp = tt - cast(long)(tt); // MODULO(tt,1.0_dp)

            double tmp = this.nside * sqrt( 3.0*(1.0 - za) );
            long jp = cast(long)(tp*tmp); // increasing edge line index
            long jm = cast(long)((1.0 - tp) * tmp); // decreasing edge line index

            long ir = jp + jm + 1; // ring number counted from the closest pole
            long ip = cast(long)( tt * ir); // in {0,4*ir-1}

            if (ip >= 4*ir) {
                ip = ip - 4*ir;
            }
            if (p.z>0.) {
                ipix = 2*ir*(ir-1) + ip;
            } else {
                ipix = this.npix - 2*ir*(ir+1) + ip;
            }

        }

        return ipix;
    }

    /**
     * radius in radians
     */
    long disc_intersect(double ra, 
                        double dec, 
                        double radius, 
                        Stack!(long)* listpix) {
        auto p = new Point(ra,dec);
        return disc_intersect(p, radius, listpix);
    }
    long disc_intersect(Point p, double radius, Stack!(long)* listpix) {

        // this is from the f90 code
        // this number is acos(2/3)
        double fudge = 0.84106867056793033/this.nside; // 1.071* half pixel size

        // this is from the c++ code
        //double fudge = 1.362*PI/(4*nside);

        //double fudge = sqrt(area);

        radius += fudge;
        disc_contains(p, radius, listpix);
        return listpix.length;
    }


    /**
     * radius in radians
     */
    void disc_contains(Point p, double radius, Stack!(long)* listpix) {

        long tmp1,tmp2;
        double cosang = cos(radius);

        // this does not alter the storage
        listpix.resize(0);

        double dth1 = 1. / (3.0*nside*nside);
        double dth2 = 2. / (3.0*nside);

        double phi0=0.0;
        if ((p.x != 0.) || (p.y != 0.)) {
            // in (-Pi, Pi]
            phi0 = p.phi;
            //phi0 = atan2(p.y,p.x);
        }
        double cosphi0 = cos(phi0);
        double a = p.x*p.x + p.y*p.y;

        //     --- coordinate z of highest and lowest points in the disc ---
        double rlat0  = asin(p.z);    // latitude in RAD of the center
        double rlat1  = rlat0 + radius;
        double rlat2  = rlat0 - radius;
        double zmax;
        if (rlat1 >=  PI_2) {
            zmax =  1.0;
        } else {
            zmax = sin(rlat1);
        }
        long irmin = ring_num(zmax);
        tmp1 = irmin-1;
        irmin = (1 > tmp1) ? 1 : tmp1;
        //irmin = max(1, irmin-1); // start from a higher point, to be safe

        double zmin;
        if (rlat2 <= -PI_2) {
            zmin = -1.;
        } else {
            zmin = sin(rlat2);
        }
        long irmax = ring_num(zmin);
        tmp1 = 4*nside-1;
        tmp2 = irmax+1;
        irmax = (tmp1 < tmp2) ? tmp1 : tmp2;
        //irmax = min(4*nside-1, irmax + 1); // go down to a lower point

        //double z, tmp=0;
        double tmp=0;
        for (long iz=irmin; iz<= irmax; iz++) {

            double z;
            if (iz <= nside-1) { // north polar cap
                z = 1.  - iz*iz*dth1;
            } else if (iz <= 3*nside) { // tropical band + equat.
                z = (2*nside-iz) * dth2;
            } else {
                tmp = 4*nside-iz;
                z = - 1. + tmp*tmp*dth1;
            }
            double b = cosang - z*p.z;
            double c = 1. - z*z;
            double dphi;
            if ((p.x==0.) && (p.y==0.)) {
                dphi=PI;
                if (b > 0.) {
                    goto SKIP2; // out of the disc, 2008-03-30
                }
                goto SKIP1;
            } 
            double cosdphi = b / sqrt(a*c);
            if (fabs(cosdphi) <= 1.) {
                dphi = acos(cosdphi); // in [0,Pi]
            } else {
                if (cosphi0 < cosdphi) {
                    goto SKIP2; // out of the disc
                }
                dphi = PI; // all the pixels at this elevation are in the disc
            }
SKIP1:
            in_ring(iz, phi0, dphi, listpix);

SKIP2:
            // we have to put something here
            continue;

        }

        /*
        writeln("t1: ", t1);
        writeln("t2: ", t2);
        writeln("t3: ", t3);
        writeln("t4: ", t4);
        writeln("--------------");
        */
    }

    long ring_num(double z) {

        // rounds double to nearest long long int
        long iring = lrint( nside*(2.-1.5*z) );

        // north cap
        if (z > M_TWOTHIRD) {
            iring = lrint( nside* sqrt(3.*(1.-z)) );
            if (iring == 0) {
                iring = 1;
            }
        } else if (z < -M_TWOTHIRD) {
            iring = lrint( nside* sqrt(3.*(1.+z)) );

            if (iring == 0) {
                iring = 1;
            }
            iring = 4*nside - iring;
        }

        return iring;
    }



    void in_ring(long iz, 
                 double phi0, 
                 double dphi, 
                 Stack!(long)* plist) {

        long nr, ir, ipix1;
        double shift=0.5;

        if (iz<this.nside) {
            // north pole
            ir = iz;
            nr = ir*4;
            ipix1 = 2*ir*(ir-1);        //    lowest pixel number in the ring
        } else if (iz>(3*this.nside)) {
            // south pole
            ir = 4*this.nside - iz;
            nr = ir*4;
            ipix1 = this.npix - 2*ir*(ir+1); // lowest pixel number in the ring
        } else {
            // equatorial region
            ir = iz - this.nside + 1;           //    within {1, 2*nside + 1}
            nr = this.nside*4;
            if ((ir&1)==0) shift = 0;
            ipix1 = this.ncap + (ir-1)*nr; // lowest pixel number in the ring
        }

        long ipix2 = ipix1 + nr - 1;  //    highest pixel number in the ring


        if (dphi > (PI-1e-7)) {
            for (long i=ipix1; i<=ipix2; ++i) {
                plist.push(i);
            }
        } else {

            // M_1_PI is 1/pi
            long ip_lo = cast(long)( floor(nr*.5*M_1_PI*(phi0-dphi) - shift) )+1;
            long ip_hi = cast(long)( floor(nr*.5*M_1_PI*(phi0+dphi) - shift) );
            long pixnum = ip_lo+ipix1;
            if (pixnum<ipix1) {
                pixnum += nr;
            }
            for (long i=ip_lo; i<=ip_hi; ++i, ++pixnum) {
                if (pixnum>ipix2) {
                    pixnum -= nr;
                }
                plist.push(pixnum);
            }
        }

    }


}






unittest 
{
    long nside = 4096;
    auto hpix = new Healpix(nside);

    writeln("hpix: doing pixelof unit test");
    assert(hpix.pixelof(0.00000000,-90.00000000)== 201326588);
    assert(hpix.pixelof(0.00000000,-85.00000000)== 200942028);
    assert(hpix.pixelof(0.00000000,-65.00000000)== 191887080);
    assert(hpix.pixelof(0.00000000,-45.00000000)== 171827712);
    assert(hpix.pixelof(0.00000000,-25.00000000)== 143204352);
    assert(hpix.pixelof(0.00000000,-5.00000000)== 109420544);
    assert(hpix.pixelof(0.00000000,0.00000000)== 100638720);
    assert(hpix.pixelof(0.00000000,5.00000000)== 91889664);
    assert(hpix.pixelof(0.00000000,25.00000000)== 58105856);
    assert(hpix.pixelof(0.00000000,45.00000000)== 29483520);
    assert(hpix.pixelof(0.00000000,65.00000000)== 9430824);
    assert(hpix.pixelof(0.00000000,85.00000000)== 382812);
    assert(hpix.pixelof(0.00000000,90.00000000)== 0);
    assert(hpix.pixelof(40.00000000,-90.00000000)== 201326588);
    assert(hpix.pixelof(40.00000000,-85.00000000)== 200942222);
    assert(hpix.pixelof(40.00000000,-65.00000000)== 191888045);
    assert(hpix.pixelof(40.00000000,-45.00000000)== 171829418);
    assert(hpix.pixelof(40.00000000,-25.00000000)== 143189788);
    assert(hpix.pixelof(40.00000000,-5.00000000)== 109438748);
    assert(hpix.pixelof(40.00000000,0.00000000)== 100656924);
    assert(hpix.pixelof(40.00000000,5.00000000)== 91875100);
    assert(hpix.pixelof(40.00000000,25.00000000)== 58124060);
    assert(hpix.pixelof(40.00000000,45.00000000)== 29485226);
    assert(hpix.pixelof(40.00000000,65.00000000)== 9431789);
    assert(hpix.pixelof(40.00000000,85.00000000)== 383006);
    assert(hpix.pixelof(40.00000000,90.00000000)== 0);
    assert(hpix.pixelof(80.00000000,-90.00000000)== 201326588);
    assert(hpix.pixelof(80.00000000,-85.00000000)== 200942417);
    assert(hpix.pixelof(80.00000000,-65.00000000)== 191889010);
    assert(hpix.pixelof(80.00000000,-45.00000000)== 171846484);
    assert(hpix.pixelof(80.00000000,-25.00000000)== 143207993);
    assert(hpix.pixelof(80.00000000,-5.00000000)== 109424185);
    assert(hpix.pixelof(80.00000000,0.00000000)== 100658744);
    assert(hpix.pixelof(80.00000000,5.00000000)== 91893305);
    assert(hpix.pixelof(80.00000000,25.00000000)== 58109497);
    assert(hpix.pixelof(80.00000000,45.00000000)== 29471576);
    assert(hpix.pixelof(80.00000000,65.00000000)== 9432754);
    assert(hpix.pixelof(80.00000000,85.00000000)== 383201);
    assert(hpix.pixelof(80.00000000,90.00000000)== 0);
    assert(hpix.pixelof(120.00000000,-90.00000000)== 201326589);
    assert(hpix.pixelof(120.00000000,-85.00000000)== 200944362);
    assert(hpix.pixelof(120.00000000,-65.00000000)== 191898662);
    assert(hpix.pixelof(120.00000000,-45.00000000)== 171848190);
    assert(hpix.pixelof(120.00000000,-25.00000000)== 143193429);
    assert(hpix.pixelof(120.00000000,-5.00000000)== 109442389);
    assert(hpix.pixelof(120.00000000,0.00000000)== 100660565);
    assert(hpix.pixelof(120.00000000,5.00000000)== 91878741);
    assert(hpix.pixelof(120.00000000,25.00000000)== 58127701);
    assert(hpix.pixelof(120.00000000,45.00000000)== 29473282);
    assert(hpix.pixelof(120.00000000,65.00000000)== 9425034);
    assert(hpix.pixelof(120.00000000,85.00000000)== 381646);
    assert(hpix.pixelof(120.00000000,90.00000000)== 1);
    assert(hpix.pixelof(160.00000000,-90.00000000)== 201326589);
    assert(hpix.pixelof(160.00000000,-85.00000000)== 200942806);
    assert(hpix.pixelof(160.00000000,-65.00000000)== 191899627);
    assert(hpix.pixelof(160.00000000,-45.00000000)== 171834538);
    assert(hpix.pixelof(160.00000000,-25.00000000)== 143211634);
    assert(hpix.pixelof(160.00000000,-5.00000000)== 109427826);
    assert(hpix.pixelof(160.00000000,0.00000000)== 100662385);
    assert(hpix.pixelof(160.00000000,5.00000000)== 91896946);
    assert(hpix.pixelof(160.00000000,25.00000000)== 58113138);
    assert(hpix.pixelof(160.00000000,45.00000000)== 29490346);
    assert(hpix.pixelof(160.00000000,65.00000000)== 9425999);
    assert(hpix.pixelof(160.00000000,85.00000000)== 383590);
    assert(hpix.pixelof(160.00000000,90.00000000)== 1);
    assert(hpix.pixelof(200.00000000,-90.00000000)== 201326590);
    assert(hpix.pixelof(200.00000000,-85.00000000)== 200943001);
    assert(hpix.pixelof(200.00000000,-65.00000000)== 191900592);
    assert(hpix.pixelof(200.00000000,-45.00000000)== 171836245);
    assert(hpix.pixelof(200.00000000,-25.00000000)== 143213454);
    assert(hpix.pixelof(200.00000000,-5.00000000)== 109429646);
    assert(hpix.pixelof(200.00000000,0.00000000)== 100664206);
    assert(hpix.pixelof(200.00000000,5.00000000)== 91898766);
    assert(hpix.pixelof(200.00000000,25.00000000)== 58114958);
    assert(hpix.pixelof(200.00000000,45.00000000)== 29492053);
    assert(hpix.pixelof(200.00000000,65.00000000)== 9426964);
    assert(hpix.pixelof(200.00000000,85.00000000)== 383785);
    assert(hpix.pixelof(200.00000000,90.00000000)== 2);
    assert(hpix.pixelof(240.00000000,-90.00000000)== 201326590);
    assert(hpix.pixelof(240.00000000,-85.00000000)== 200944945);
    assert(hpix.pixelof(240.00000000,-65.00000000)== 191901557);
    assert(hpix.pixelof(240.00000000,-45.00000000)== 171853309);
    assert(hpix.pixelof(240.00000000,-25.00000000)== 143198890);
    assert(hpix.pixelof(240.00000000,-5.00000000)== 109447850);
    assert(hpix.pixelof(240.00000000,0.00000000)== 100666026);
    assert(hpix.pixelof(240.00000000,5.00000000)== 91884202);
    assert(hpix.pixelof(240.00000000,25.00000000)== 58133162);
    assert(hpix.pixelof(240.00000000,45.00000000)== 29478401);
    assert(hpix.pixelof(240.00000000,65.00000000)== 9427929);
    assert(hpix.pixelof(240.00000000,85.00000000)== 382229);
    assert(hpix.pixelof(240.00000000,90.00000000)== 2);
    assert(hpix.pixelof(280.00000000,-90.00000000)== 201326591);
    assert(hpix.pixelof(280.00000000,-85.00000000)== 200943390);
    assert(hpix.pixelof(280.00000000,-65.00000000)== 191893837);
    assert(hpix.pixelof(280.00000000,-45.00000000)== 171855015);
    assert(hpix.pixelof(280.00000000,-25.00000000)== 143217095);
    assert(hpix.pixelof(280.00000000,-5.00000000)== 109433287);
    assert(hpix.pixelof(280.00000000,0.00000000)== 100667847);
    assert(hpix.pixelof(280.00000000,5.00000000)== 91902407);
    assert(hpix.pixelof(280.00000000,25.00000000)== 58118599);
    assert(hpix.pixelof(280.00000000,45.00000000)== 29480107);
    assert(hpix.pixelof(280.00000000,65.00000000)== 9437581);
    assert(hpix.pixelof(280.00000000,85.00000000)== 384174);
    assert(hpix.pixelof(280.00000000,90.00000000)== 3);
    assert(hpix.pixelof(320.00000000,-90.00000000)== 201326591);
    assert(hpix.pixelof(320.00000000,-85.00000000)== 200943585);
    assert(hpix.pixelof(320.00000000,-65.00000000)== 191894802);
    assert(hpix.pixelof(320.00000000,-45.00000000)== 171841365);
    assert(hpix.pixelof(320.00000000,-25.00000000)== 143202531);
    assert(hpix.pixelof(320.00000000,-5.00000000)== 109451491);
    assert(hpix.pixelof(320.00000000,0.00000000)== 100669667);
    assert(hpix.pixelof(320.00000000,5.00000000)== 91887843);
    assert(hpix.pixelof(320.00000000,25.00000000)== 58136803);
    assert(hpix.pixelof(320.00000000,45.00000000)== 29497173);
    assert(hpix.pixelof(320.00000000,65.00000000)== 9438546);
    assert(hpix.pixelof(320.00000000,85.00000000)== 384369);
    assert(hpix.pixelof(320.00000000,90.00000000)== 3);
    assert(hpix.pixelof(360.00000000,-90.00000000)== 201326588);
    assert(hpix.pixelof(360.00000000,-85.00000000)== 200942028);
    assert(hpix.pixelof(360.00000000,-65.00000000)== 191887080);
    assert(hpix.pixelof(360.00000000,-45.00000000)== 171827712);
    assert(hpix.pixelof(360.00000000,-25.00000000)== 143204352);
    assert(hpix.pixelof(360.00000000,-5.00000000)== 109420544);

    // precision issues make this not exactly same as the C code
    //assert(hpix.pixelof(360.00000000,0.00000000)== 100638720);

    assert(hpix.pixelof(360.00000000,5.00000000)== 91889664);
    assert(hpix.pixelof(360.00000000,25.00000000)== 58105856);
    assert(hpix.pixelof(360.00000000,45.00000000)== 29483520);
    assert(hpix.pixelof(360.00000000,65.00000000)== 9430824);
    assert(hpix.pixelof(360.00000000,85.00000000)== 382812);
    assert(hpix.pixelof(360.00000000,90.00000000)== 0);

}
unittest
{
    long nside = 4096;
    auto hpix = new Healpix(nside);

    writeln("hpix: doing ring num unit test");
    assert(hpix.ring_num(-0.75) == 12837);
    assert(hpix.ring_num(-0.2) == 9421);
    assert(hpix.ring_num(0.2) == 6963);
    assert(hpix.ring_num(0.75) == 3547);

}
unittest
{
    long nside = 4096;
    auto hpix = new Healpix(nside);

    Stack!(long) pixlist;

    double rad_arcmin=40.0/60.;
    double rad = rad_arcmin/60.*D2R;

    writeln("hpix: doing intersect unit test");
    assert(hpix.disc_intersect(0.000000,-90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(0.000000,-85.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(0.000000,-65.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(0.000000,-45.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(0.000000,-25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(0.000000,-5.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(0.000000,0.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(0.000000,5.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(0.000000,25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(0.000000,45.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(0.000000,65.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(0.000000,85.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(0.000000,90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(40.000000,-90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(40.000000,-85.000000,rad,&pixlist)==9);
    assert(hpix.disc_intersect(40.000000,-65.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(40.000000,-45.000000,rad,&pixlist)==10);
    assert(hpix.disc_intersect(40.000000,-25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(40.000000,-5.000000,rad,&pixlist)==7);
    assert(hpix.disc_intersect(40.000000,0.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(40.000000,5.000000,rad,&pixlist)==7);
    assert(hpix.disc_intersect(40.000000,25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(40.000000,45.000000,rad,&pixlist)==10);
    assert(hpix.disc_intersect(40.000000,65.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(40.000000,85.000000,rad,&pixlist)==9);
    assert(hpix.disc_intersect(40.000000,90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(80.000000,-90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(80.000000,-85.000000,rad,&pixlist)==9);
    assert(hpix.disc_intersect(80.000000,-65.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(80.000000,-45.000000,rad,&pixlist)==9);
    assert(hpix.disc_intersect(80.000000,-25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(80.000000,-5.000000,rad,&pixlist)==7);
    assert(hpix.disc_intersect(80.000000,0.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(80.000000,5.000000,rad,&pixlist)==7);
    assert(hpix.disc_intersect(80.000000,25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(80.000000,45.000000,rad,&pixlist)==9);
    assert(hpix.disc_intersect(80.000000,65.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(80.000000,85.000000,rad,&pixlist)==9);
    assert(hpix.disc_intersect(80.000000,90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(120.000000,-90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(120.000000,-85.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(120.000000,-65.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(120.000000,-45.000000,rad,&pixlist)==7);
    assert(hpix.disc_intersect(120.000000,-25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(120.000000,-5.000000,rad,&pixlist)==7);
    assert(hpix.disc_intersect(120.000000,0.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(120.000000,5.000000,rad,&pixlist)==7);
    assert(hpix.disc_intersect(120.000000,25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(120.000000,45.000000,rad,&pixlist)==7);
    assert(hpix.disc_intersect(120.000000,65.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(120.000000,85.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(120.000000,90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(160.000000,-90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(160.000000,-85.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(160.000000,-65.000000,rad,&pixlist)==10);
    assert(hpix.disc_intersect(160.000000,-45.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(160.000000,-25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(160.000000,-5.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(160.000000,0.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(160.000000,5.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(160.000000,25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(160.000000,45.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(160.000000,65.000000,rad,&pixlist)==10);
    assert(hpix.disc_intersect(160.000000,85.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(160.000000,90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(200.000000,-90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(200.000000,-85.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(200.000000,-65.000000,rad,&pixlist)==10);
    assert(hpix.disc_intersect(200.000000,-45.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(200.000000,-25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(200.000000,-5.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(200.000000,0.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(200.000000,5.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(200.000000,25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(200.000000,45.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(200.000000,65.000000,rad,&pixlist)==10);
    assert(hpix.disc_intersect(200.000000,85.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(200.000000,90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(240.000000,-90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(240.000000,-85.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(240.000000,-65.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(240.000000,-45.000000,rad,&pixlist)==7);
    assert(hpix.disc_intersect(240.000000,-25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(240.000000,-5.000000,rad,&pixlist)==7);
    assert(hpix.disc_intersect(240.000000,0.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(240.000000,5.000000,rad,&pixlist)==7);
    assert(hpix.disc_intersect(240.000000,25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(240.000000,45.000000,rad,&pixlist)==7);
    assert(hpix.disc_intersect(240.000000,65.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(240.000000,85.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(240.000000,90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(280.000000,-90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(280.000000,-85.000000,rad,&pixlist)==9);
    assert(hpix.disc_intersect(280.000000,-65.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(280.000000,-45.000000,rad,&pixlist)==9);
    assert(hpix.disc_intersect(280.000000,-25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(280.000000,-5.000000,rad,&pixlist)==7);
    assert(hpix.disc_intersect(280.000000,0.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(280.000000,5.000000,rad,&pixlist)==7);
    assert(hpix.disc_intersect(280.000000,25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(280.000000,45.000000,rad,&pixlist)==9);
    assert(hpix.disc_intersect(280.000000,65.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(280.000000,85.000000,rad,&pixlist)==9);
    assert(hpix.disc_intersect(280.000000,90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(320.000000,-90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(320.000000,-85.000000,rad,&pixlist)==9);
    assert(hpix.disc_intersect(320.000000,-65.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(320.000000,-45.000000,rad,&pixlist)==10);
    assert(hpix.disc_intersect(320.000000,-25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(320.000000,-5.000000,rad,&pixlist)==7);
    assert(hpix.disc_intersect(320.000000,0.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(320.000000,5.000000,rad,&pixlist)==7);
    assert(hpix.disc_intersect(320.000000,25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(320.000000,45.000000,rad,&pixlist)==10);
    assert(hpix.disc_intersect(320.000000,65.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(320.000000,85.000000,rad,&pixlist)==9);
    assert(hpix.disc_intersect(320.000000,90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(360.000000,-90.000000,rad,&pixlist)==12);
    assert(hpix.disc_intersect(360.000000,-85.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(360.000000,-65.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(360.000000,-45.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(360.000000,-25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(360.000000,-5.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(360.000000,0.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(360.000000,5.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(360.000000,25.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(360.000000,45.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(360.000000,65.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(360.000000,85.000000,rad,&pixlist)==8);
    assert(hpix.disc_intersect(360.000000,90.000000,rad,&pixlist)==12);

}
