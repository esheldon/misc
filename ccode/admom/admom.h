#ifndef _ADMOM_HEADER_GUARD
#define _ADMOM_HEADER_GUARD

#include <stdio.h>
#include "image.h"
#include "gauss2.h"

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
#define AM_DETTOL 1.e-7

/* Flags: these correspond to what used to be in PHOTO */
#define AM_FLAG_FAINT 0x1
#define AM_FLAG_SHIFT 0x2
#define AM_FLAG_MAXIT 0x4

#define DEBUG
//#define DEBUG2

#ifdef DEBUG
 #define DBG if(1) 
#else
 #define DBG if(0) 
#endif
#ifdef DEBUG2
 #define DBG2 if(1) 
#else
 #define DBG2 if(0) 
#endif



struct am {
    // input parameters for algorithm
    struct gauss2 guess;
    double nsigma;  // number of sigma around center for calculations
    int maxiter;
    double shiftmax;
    double sky;
    double skysig;


    // outputs
    struct gauss2 wt;
    double s2n;
    double rho4;
    double uncer; // error on either e1 or e2 for round object
    int numiter;
    int flags;

    // temporary *measured* moments, used to take the adaptive step
    double irr_tmp, irc_tmp, icc_tmp, det_tmp;
};


void admom(struct am *am, const struct image *image);

void admom_print(const struct am *am, FILE *stream);

/* 
   this is bogus, need to fix

  add gaussian noise to the image to get the requested S/N.  The S/N is defined
  as

     sum(weight*im)/sqrt(sum(weight)/skysig

  Returned are the skysig and the measured s/n which should be equivalent to
  the requested s/n to precision.
*/
void admom_add_noise(struct image *image, double s2n, const struct gauss2 *wt,
                     double *skysig, double *s2n_meas);

#endif
