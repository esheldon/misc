#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include "healpix.h"
#include "defs.h"

int64 hpix_npix(int64 nside) {
    return 12*nside*nside;
}

double hpix_pixarea(int64 nside) {
    int64 npix = hpix_npix(nside);
    return 4.0*M_PI/npix;
}

int64 hpix_eq2pix(int64 nside, double ra, double dec) {
    int64 ipix=0;
    double theta=0, phi=0;

    hpix_radec_degrees_to_thetaphi_radians(ra, dec, &theta, &phi);

    if (nside < 1 || nside > NS_MAX) {
        printf("nside out of range [%d, %d]\n", 1, NS_MAX);
        exit(EXIT_FAILURE);
    }
    if (theta < 0.0 || theta > M_PI) {
        printf("nside out of range [0,PI]\n");
        exit(EXIT_FAILURE);
    }

    double z = cos(theta);
    double za = fabs(z);

    // in [0,4)
    double tt = fmod(phi, M_TWO_PI)/M_PI_2;

    if (za <= M_TWOTHIRD) {
        double temp1 = nside*(.5 + tt);
        double temp2 = nside*.75*z;

        int64 jp = (int64)(temp1-temp2); // index of  ascending edge line
        int64 jm = (int64)(temp1+temp2); // index of descending edge line
        int64 ir = nside + 1 + jp - jm;  // in {1,2n+1} (ring number counted from z=2/3)
        int64 kshift = 1 - (ir % 2);      // kshift=1 if ir even, 0 otherwise
        
        int64 nl4 = 4*nside;
        int64 ip = (int64)( ( jp+jm - nside + kshift + 1 ) / 2); // in {0,4n-1}

        if (ip >= nl4) {
            ip = ip - nl4;
        }

        ipix = 2*nside*(nside-1) + nl4*(ir-1) + ip;

    } else { 
        // North & South polar caps
        double tp = tt - (int64)(tt);   // MODULO(tt,1.0_dp)

        double tmp = nside * sqrt( 3.0*(1.0 - za) );
        int64 jp = (int64)(tp*tmp);              // increasing edge line index
        int64 jm = (int64)((1.0 - tp) * tmp); // decreasing edge line index

        int64 ir = jp + jm + 1;        // ring number counted from the closest pole
        int64 ip = (int64)( tt * ir);     // in {0,4*ir-1}

        if (ip >= 4*ir) {
            ip = ip - 4*ir;
        }
        if (z>0.) {
            ipix = 2*ir*(ir-1) + ip;
        } else {
            ipix = 12*nside*nside - 2*ir*(ir+1) + ip;
        }

    }

    return ipix;
}


int64 hpix_ring_num(int64 nside, double z) {

    // rounds double to nearest long long int
    int64 iring = llrintl( nside*(2.-1.5*z) );

    // north cap
    if (z > M_TWOTHIRD) {
        iring = llrintl( nside* sqrt(3.*(1.-z)) );
        if (iring == 0) {
            iring = 1;
        }
    } else if (z < -M_TWOTHIRD) {
        iring = llrintl( nside* sqrt(3.*(1.+z)) );

        if (iring == 0) {
            iring = 1;
        }
        iring = 4*nside - iring;
    }

    return iring;
}

void hpix_eq2vec(double ra, double dec, double vector[3]) {

    double theta=0, phi=0;

    hpix_radec_degrees_to_thetaphi_radians(ra, dec, &theta, &phi);
    if (theta < 0.0 || theta > M_PI) {
        printf("nside out of range [0,PI]\n");
        exit(EXIT_FAILURE);
    }

    double sintheta = sin(theta);
    vector[0] = sintheta * cos(phi);
    vector[1] = sintheta * sin(phi);
    vector[2] = cos(theta);

}


void hpix_radec_degrees_to_thetaphi_radians(double ra, double dec, double* theta, double* phi) {
    *phi = ra*D2R;
    *theta = -dec*D2R + M_PI_2;
}
