/* vim: set ft=vala : */

using Math;
using Gee;

// 2^28 : largest nside available
const long NS_MAX = 268435456;

const double M_TWO_PI   = 6.28318530717958647693; /* 2*pi */
const double M_TWOTHIRD = 0.66666666666666666666;
const double M_1_PI     = 0.31830988618379067154; /* 1/pi */


public class HPoint : Point {
    public new double phi;
    public double theta;

    public HPoint.from_radec(double ra, double dec) {
        // note this gives the same x,y,z as for the superclass Point
        phi = ra*D2R;
        theta = PI_2 -dec*D2R;

        double sintheta = sin(theta);
        x = sintheta * cos(phi);
        y = sintheta * sin(phi);
        z = cos(theta);
    }

    public new void set_radec(double ra, double dec) {
        phi = ra*D2R;
        theta = PI_2 -dec*D2R;
        double sintheta = sin(theta);
        x = sintheta * cos(phi);
        y = sintheta * sin(phi);
        z = cos(theta);

    }
}

public class Healpix : GLib.Object {
    public long nside;
    public long npix;
    public long ncap; // number of pixels in the north polar cap
    public double area;


    public Healpix(long nside=4096) 
        requires (nside > 0 && nside <= NS_MAX) // yay contracts
    {
        set_nside(nside);
    }
    public void set_nside(long nside) {
        this.nside = nside;
        npix = 12*nside*nside;
        area = 4.0*PI/npix;
        ncap = 2*nside*(nside-1); 
    }
    public long pixelof_eq(double ra, double dec) {
        var p = new HPoint.from_radec(ra,dec);
        return pixelof(p);
    }

    public long pixelof(HPoint pt) {
        long ipix=0;
        double za = Math.fabs(pt.z);

        // in [0,4)
        double tt = Math.fmod(pt.phi, M_TWO_PI)/PI_2;
        if (za <= M_TWOTHIRD) {
            double temp1 = this.nside*(0.5 + tt);
            double temp2 = this.nside*0.75*pt.z;

            long jp = (long)(temp1-temp2); // index of  ascending edge line
            long jm = (long)(temp1+temp2); // index of descending edge line
            // in {1,2n+1} (ring number counted from z=2/3)
            long ir = this.nside + 1 + jp - jm;  
            long kshift = 1 - (ir % 2);      // kshift=1 if ir even, 0 otherwise

            long nl4 = 4*this.nside;
            // in {0,4n-1}
            long ip = (long)( ( jp+jm - this.nside + kshift + 1 ) / 2); 

            ip = ip % nl4;

            ipix = this.ncap + nl4*(ir-1) + ip;

        } else {
            // North & South polar caps
            double tp = tt - (long)(tt); // MODULO(tt,1.0_dp)

            double tmp = this.nside * sqrt( 3.0*(1.0 - za) );
            long jp = (long)(tp*tmp); // increasing edge line index
            long jm = (long)((1.0 - tp) * tmp); // decreasing edge line index

            long ir = jp + jm + 1; // ring number counted from the closest pole
            long ip = (long)( tt * ir); // in {0,4*ir-1}

            if (ip >= 4*ir) {
                ip = ip - 4*ir;
            }
            if (pt.z>0.0) {
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
    public long disc_intersect_eq(double ra, 
                                  double dec, 
                                  double radius, 
                                  ArrayList<long> listpix) {
        var pt = new Point.from_radec(ra,dec);
        return disc_intersect(pt, radius, listpix);
    }
    public long disc_intersect(Point pt, 
                               double radius, 
                               ArrayList<long> listpix) {

        // this is from the f90 code
        // this number is acos(2/3)
        double fudge = 0.84106867056793033/this.nside; // 1.071* half pixel size

        // this is from the c++ code
        //double fudge = 1.362*PI/(4*nside);

        //double fudge = sqrt(area);

        radius += fudge;
        disc_contains(pt, radius, listpix);
        return listpix.size;
    }


    public void disc_contains(Point p, double radius, ArrayList<long> listpix) {

        long tmp1,tmp2;
        double cosang = cos(radius);

        // this does not alter the storage
        listpix.clear();

        double dth1 = 1.0 / (3.0*nside*nside);
        double dth2 = 2.0 / (3.0*nside);

        double phi0=0.0;
        if ((p.x != 0.0) || (p.y != 0.0)) {
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
            zmin = -1.0;
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
        int skip=0;
        for (long iz=irmin; iz<= irmax; iz++) {

            double z;
            if (iz <= nside-1) { // north polar cap
                z = 1.0  - iz*iz*dth1;
            } else if (iz <= 3*nside) { // tropical band + equat.
                z = (2*nside-iz) * dth2;
            } else {
                tmp = 4*nside-iz;
                z = - 1.0 + tmp*tmp*dth1;
            }
            double b = cosang - z*p.z;
            double c = 1.0 - z*z;
            double dphi=0;
            skip=0;
            if ((p.x==0.0) && (p.y==0.0)) {
                dphi=PI;
                if (b > 0.0) {
                    skip=2;
                    //goto SKIP2; // out of the disc, 2008-03-30
                } else {
                    skip=1;
                }
                //goto SKIP1;
            } 
            if (skip==0) {
                double cosdphi = b / sqrt(a*c);
                if (fabs(cosdphi) <= 1.0) {
                    dphi = acos(cosdphi); // in [0,Pi]
                } else {
                    if (cosphi0 < cosdphi) {
                        skip=2;
                        //goto SKIP2; // out of the disc
                    } else {
                        dphi = PI; // all the pixels at this elevation are in the disc
                    }
                }
            }
//SKIP1:
            if (skip <= 1) {
                in_ring(iz, phi0, dphi, listpix);
            }

//SKIP2:
            // we have to put something here
            //continue;

        }

    }


    public long ring_num(double z) {

        // rounds double to nearest long long int
        long iring = lrint( nside*(2.0-1.5*z) );

        // north cap
        if (z > M_TWOTHIRD) {
            iring = lrint( nside* sqrt(3.0*(1.0-z)) );
            if (iring == 0) {
                iring = 1;
            }
        } else if (z < -M_TWOTHIRD) {
            iring = lrint( nside* sqrt(3.0*(1.0+z)) );

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
                 ArrayList<long> plist) {

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
                plist.add(i);
            }
        } else {

            // M_1_PI is 1/pi
            long ip_lo = (long)( floor(nr*0.5*M_1_PI*(phi0-dphi) - shift) )+1;
            long ip_hi = (long)( floor(nr*0.5*M_1_PI*(phi0+dphi) - shift) );
            long pixnum = ip_lo+ipix1;
            if (pixnum<ipix1) {
                pixnum += nr;
            }
            for (long i=ip_lo; i<=ip_hi; ++i, ++pixnum) {
                if (pixnum>ipix2) {
                    pixnum -= nr;
                }
                plist.add(pixnum);
            }
        }

    }


}
