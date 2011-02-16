#ifndef _COSMOLOGY_H
#define _COSMOLOGY_H

#include <cmath>
#include <vector>

using std::vector;

// class to calculate distances. You can use both a full integration and
// the Pen 1997 approximate formula. Both methods currently assume flat
//
// It turns out, for npts=5 the integration is faster than the approximate
// formula and much more accurate.
//
// For integration, either 3 or 5 points can be used, both of which are
// very high precision, 5 is essentially exact.

struct Cosmology {

    /*
     * Assumes:
     *     flat lambda cdm
     *  This is based off of esutil.cosmology.Cosmo for the special
     *  case of flat
     */

    int npts;
    vector<float> xxi;
    vector<float> wwi;

    float H0;
    float omega_m, omega_l;

    // The hubble distance c/H0
    float DH;

    // see esutil/cosmology.py Cosmo class
    //   four_pi_G_over_c_squared(dunits='Mpc')
    float four_pi_G_over_c_squared;

    // for integral calculations
    float f1,f2,z,ezinv;

    // for approx calculations
    float s, s2, s3, s4;

    Cosmology(float H0, float omega_m, int npts=5) {

        // in km/s/Mpc
        this->H0 = H0;
        this->omega_m = omega_m;
        this->omega_l = 1.0 - omega_m;


        // Hubble distance c/H0 in Mpc
        this->DH = 2.99792458e5/H0;

        this->four_pi_G_over_c_squared = 6.0150504541630152e-07;

        this->npts = npts;
        this->set_weights(npts);


        // for approx calculations
        this->s3 = (1.-omega_m)/omega_m;
        this->s = pow(this->s3, 1.0/3.0);
        this->s2 = this->s*this->s;
        this->s4 = this->s2*this->s2;


    }

    void set_weights(int npts) {
        this->npts=npts;
        this->gauleg(-1.0, 1.0, npts, this->xxi, this->wwi);
    }

    // Comoving distance.
    float dc(float zmin, float zmax) {
        float d = this->DH*this->ez_inverse_integral(zmin, zmax);
        return d;
    }


    // Angular diameter distance in Mpc
    //
    // For flat, the transverse comoving distance is the same
    // as the comoving distance, so this is essentially Dc 
    // from esutil.cosmology.Cosmo times DH/(1+zmax)

    float da(float zmin, float zmax) {
        float d = this->DH*this->ez_inverse_integral(zmin, zmax);

        d /= (1+zmax);
        return d;
    }

    // omega_k = 0
    float da(float dcmin, float dcmax, float zmax) {
        return (dcmax-dcmin)/(1+zmax);
    }


    float ez_inverse_integral(float zmin, float zmax) {
        // note npts,xxi,wwi,f1,f2,z,ezinv are instance variables
        f1 = (zmax-zmin)/2.;
        f2 = (zmax+zmin)/2.;

        float retval=0;
        for (int i=0; i<npts; i++) {
            z = xxi[i]*f1 + f2;
            ezinv = ez_inverse(z);

            retval += f1*ezinv*wwi[i];
        }

        return retval;

    }
    float ez_inverse(float z) {
        /* full formula
         * omega_m*(1.0+z)**3 + omega_k*(1.0+z)**2 + omega_l
         */
        float oneplusz = (1.0+z);
        float retval = this->omega_m*oneplusz*oneplusz*oneplusz + this->omega_l;
        retval = 1.0/retval;
        return sqrt(retval);
    }


    /*
     *
     * Inverse critical density in pc^2/Msun
     *
     */

    float scinv(float zlens, float zsource) {
        if (zsource <= zlens) {
            return 0.0;
        }

        // comoving distances can be subtracted
        float dl = this->dc(0.0, zlens);
        float ds = this->dc(0.0, zsource);
        float scinv = dl/(1.0+zlens)*(ds-dl)/ds;

        scinv *= this->four_pi_G_over_c_squared;
        return scinv;
    }

    // this one is super fast because we have precomputed
    // everything
    // dl and ds are *COMOVING* distances
    float scinv(float zlens, float dl, float ds) {
        if (dl >= ds) {
            return 0.0;
        }

        float scinv = dl/(1.0+zlens)*(ds-dl)/ds;
        scinv *= this->four_pi_G_over_c_squared;
        return scinv;
    }


    float scinv(float dl, float ds, float zlens, float zsource) {
        if (zsource <= zlens) {
            return 0.0;
        }

        // (dmax_comoving - dmin_comoving)/(1+zmax)
        float dls = ( ds*(1+zsource) - dl*(1+zlens) )/(1+zsource);
        float scinv = dls*dl/ds;

        scinv *= this->four_pi_G_over_c_squared;
        return scinv;
    }





    /*
     *
     * Approximate formulae from Pen 1997
     *
     */

    float dc_approx(float zmin, float zmax) {
        float dc = this->DH*( calc_aeta(zmin) - calc_aeta(zmax) );
        return(dc);
    }
    // angular diameter distance in Mpc between zmin and zmax.
    float da_approx(float zmin, float zmax) {

        float fac = this->DH/(1+zmax);
        float da = fac*( calc_aeta(zmin) - calc_aeta(zmax) );

        return(da);
    }

    float calc_aeta(float z) {
        float a, a2, a3, a4, time;

        static const float 
            c0=1.0, c1= -.1540, c2=.4304, c3= .19097, c4=.066941, exp= -0.125,
            onethird=0.3333333;

        /*
        s3 = (1.-this->omega_m)/this->omega_m;
        s = pow(s3, onethird);
        s2 = s*s;
        s4 = s2*s2;
        */

        a = 1./(1.0+z);
        a2 = a*a;
        a3 = a2*a;
        a4 = a3*a;

        // note s,s2,s3,s4 are instance variables
        time = c0/a4 + c1*s/a3 + c2*s2/a2 + c3*s3/a + c4*s4;
        time = 2.0*sqrt(s3+1.0)*pow(time, exp);
        return(time);
    }




    // Same as above but using fast approximate formulae
    // but note for npts=3 these aren't actually faster.
    // It is by pre-computing aeta that we get much faster.
    // because dls is trivial.
    float scinv_approx(float zlens, float zsource) {
        if (zsource <= zlens) {
            return 0.0;
        }
        float dl = this->da_approx(0.0, zlens);
        float ds = this->da_approx(0.0, zsource);
        float dls = this->da_approx(zlens, zsource);

        float scinv = dls*dl/ds;

        scinv *= this->four_pi_G_over_c_squared;
        return scinv;
    }

    // pre-computed dlens and dsource.  Should be faster.
    float scinv_approx(float dl, float ds, float zlens, float zsource) {
        if (zsource <= zlens) {
            return 0.0;
        }
        float dls = this->da_approx(zlens, zsource);

        float scinv = dls*dl/ds;

        scinv *= this->four_pi_G_over_c_squared;
        return scinv;
    }

    // using the aeta-relative as inputs
    // This is the fast one!
    float scinv_approx(float DL, float aeta_rel_lens, float aeta_rel_source) {
        if (aeta_rel_source <= aeta_rel_lens) {
            return 0.0;
        }
        float retval = (aeta_rel_source - aeta_rel_lens)/aeta_rel_source;
        retval *= this->four_pi_G_over_c_squared*DL;

        return retval;
    }


    // from numerical recipes
    void gauleg(double x1, double x2, int npts, 
                vector<float>& x, 
                vector<float>& w) {

        x.resize(npts);
        w.resize(npts);

        int i, j, m;
        double xm, xl, z1, z, p1, p2, p3, pp=0, EPS, abszdiff;

        EPS = 4.e-11;

        m = (npts + 1)/2;

        xm = (x1 + x2)/2.0;
        xl = (x2 - x1)/2.0;
        z1 = 0.0;

        for (i=1; i<= m; ++i) 
        {

            z=cos( M_PI*(i-0.25)/(npts+.5) );

            abszdiff = fabs(z-z1);

            while (abszdiff > EPS) 
            {
                p1 = 1.0;
                p2 = 0.0;
                for (j=1; j <= npts;++j)
                {
                    p3 = p2;
                    p2 = p1;
                    p1 = ( (2.0*j - 1.0)*z*p2 - (j-1.0)*p3 )/j;
                }
                pp = npts*(z*p1 - p2)/(z*z -1.);
                z1=z;
                z=z1 - p1/pp;

                abszdiff = fabs(z-z1);

            }

            x[i-1] = xm - xl*z;
            x[npts+1-i-1] = xm + xl*z;
            w[i-1] = 2.0*xl/( (1.-z*z)*pp*pp );
            w[npts+1-i-1] = w[i-1];


        }

    }


};

#endif
