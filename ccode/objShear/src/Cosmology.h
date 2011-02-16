#ifndef _COSMOLOGY_H
#define _COSMOLOGY_H

#include <cmath>
#include <vector>


// class to calculate distances. You can use both a full integration and
// the Pen 1997 approximate formula. Both methods currently assume flat
//
// For integration, either 3 or 5 points can be used, both of which are
// very high precision

struct Cosmology {

    /*
     * Assumes:
     *     flat lambda cdm
     *  This is based off of esutil.cosmology.Cosmo for the special
     *  case of flat
     */

    int npts;
    std::vector<float> xxi;
    std::vector<float> wwi;

    float C;
    float H0;
    float omega_m, omega_l;

    // The hubble distance c/H0
    float DH;

    // see esutil/cosmology.py Cosmo class
    //   four_pi_over_c_squared(dunits='Mpc')
    float four_pi_over_c_squared;

    // for calculations
    float f1,f2,z,ezinv;

    Cosmology(float H0, float omega_m, int npts=3) {

        // in km/s/Mpc
        this->H0 = H0;
        this->omega_m = omega_m;
        this->omega_l = 1.0 - omega_m;

        // Hubble distance c/H0 in Mpc
        this->C = 2.99792458e5;
        this->DH = 2.99792458e5/H0;

        this->four_pi_over_c_squared = 6.0150504541630152e-07;

        this->npts = npts;
        this->set_weights(npts);

    }

    void set_weights(int npts) {
        this->npts=npts;
        this->xxi.resize(npts);
        this->wwi.resize(npts);

        // the gauleg outputs
        if (npts == 5) {
            this->xxi[0] = -0.906179845939;
            this->xxi[1] = -0.538469310106;
            this->xxi[2] = 0.0;
            this->xxi[3] = 0.538469310106;
            this->xxi[4] = 0.906179845939;

            this->wwi[0] = 0.236926885056;
            this->wwi[1] = 0.478628670486;
            this->wwi[2] = 0.568888888889;
            this->wwi[3] = 0.478628670486;
            this->wwi[4] = 0.236926885056;

        } else if (npts == 3) {

            this->xxi[0] = -0.77459667;
            this->xxi[1] = 0.; 
            this->xxi[2] = 0.77459667;

            this->wwi[0] = 0.55555556;
            this->wwi[1] = 0.88888889;
            this->wwi[2] = 0.55555556;
        }

    }

    // Angular diameter distance in Mpc
    //
    // For flat, the transverse comoving distance is the same
    // as the comoving distance, so this is essentially Dc 
    // from esutil.cosmology.Cosmo times DH/(1+zmax)

    float Da(float zmin, float zmax) {
        float da = this->DH*this->Ez_inverse_integral(zmin, zmax);

        da /= (1+zmax);
        return da;
    }


    // Inverse critical density in pc^2/Msun
    float sigmacritinv(float zlens, float zsource) {
        if (zsource <= zlens) {
            return 0.0;
        }
        float dl = Da(0.0, zlens);
        float ds = Da(0.0, zsource);
        float dls = Da(zlens, zsource);

        float scinv = dls*dl/ds;

        scinv *= this->four_pi_over_c_squared;
        return scinv;
    }

    // pre-computed dlens and dsource.  Should be faster.
    float sigmacritinv(float dl, float ds, float zlens, float zsource) {
        if (zsource <= zlens) {
            return 0.0;
        }
        float dls = Da(zlens, zsource);

        float scinv = dls*dl/ds;

        scinv *= this->four_pi_over_c_squared;
        return scinv;
    }



    float Ez_inverse_integral(float zmin, float zmax) {
        // note npts,xxi,wwi,f1,f2,z,ezinv are instance variables
        f1 = (zmax-zmin)/2.;
        f2 = (zmax+zmin)/2.;

        float retval=0;
        for (int i=0; i<npts; i++) {
            z = xxi[i]*f1 + f2;
            ezinv = Ez_inverse(z);

            retval += f1*ezinv*wwi[i];
        }

        return retval;

    }
    float Ez_inverse(float z) {
        /* full formula
         * omega_m*(1.0+z)**3 + omega_k*(1.0+z)**2 + omega_l
         */
        float oneplusz = (1.0+z);
        float retval = this->omega_m*oneplusz*oneplusz*oneplusz + this->omega_l;
        retval = 1.0/retval;
        return sqrt(retval);
    }

    /*
     * Approximate formulae from Pen 1997
     */

    // angular diameter distance in Mpc between zmin and zmax.
    float DaFast(float zmin, float zmax) {

        float fac = this->DH/(1+zmax);
        float da = fac*( calc_aeta(zmin, omega_m) - calc_aeta(zmax, omega_m) );

        return(da);
    }

    float calc_aeta(float z, float omega_m) {
        float 
            s, s2, s3, s4, 
            a, a2, a3, a4,
            time;

        static const float 
            c0=1.0, c1= -.1540, c2=.4304, c3= .19097, c4=.066941, exp= -0.125,
            onethird=0.3333333;

        s3 = (1.-omega_m)/omega_m;
        s = pow(s3, onethird);
        s2 = s*s;
        s4 = s2*s2;
        a = 1./(1.0+z);

        a2 = a*a;
        a3 = a2*a;
        a4 = a3*a;

        time = c0/a4 + c1*s/a3 + c2*s2/a2 + c3*s3/a + c4*s4;
        time = 2.0*sqrt(s3+1.0)*pow(time, exp);
        return(time);
    }


};

#endif
