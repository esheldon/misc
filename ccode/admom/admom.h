#ifndef _ADMOM_HEADER_GUARD
#define _ADMOM_HEADER_GUARD

#include <stdio.h>
#include "image.h"
#include "gauss.h"

#define AM_SHIFTMAX 5

/*
c       2**0: sum  <= 0
c       2**1: abs(xcen-xcenorig) > shiftmax or abs(ycen-ycenorig) > shiftmax
c       2**2: another sum <= 0, but what's the diff?
c       2**3: m(1,1) <= 0 and m(2,2) <= 0
c       2**4: detm.le.1.e-7
c       2**5: detn.le.0
c       2**6: w(1,1).lt.0..or.w(2,2).lt.0.
c       2**7: imom.eq.maxit
c       2**8: detw <= 0
*/

#define AM_MAXIT 100
#define AM_XINTERP 0.0
#define AM_XINTERP2 0.0
#define AM_TOL1 0.001
#define AM_TOL2 0.01
#define AM_DETTOL 1.e-6

/* Flags: these correspond to what used to be in PHOTO */
#define AM_FLAG_FAINT 0x1
#define AM_FLAG_SHIFT 0x2
#define AM_FLAG_MAXIT 0x4


struct am {
    // input parameters for algorithm
    struct gauss guess;
    double nsigma;  // number of sigma around center for calculations
    int maxiter;
    double shiftmax;
    double sky;
    double skysig;


    // outputs
    struct gauss wt;
    double s2n;
    double rho4;
    double uncer; // error on either e1 or e2 for round object
    int numiter;
    int flags;

    // *measured* moments, used to take the adaptive step
    double irr, irc, icc;
};


void admom(struct am *am, const struct image *image);

void admom_print(const struct am *am, FILE *stream);

#endif
