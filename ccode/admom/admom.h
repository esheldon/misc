#ifndef _ADMOM_HEADER_GUARD
#define _ADMOM_HEADER_GUARD

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
c       2**9: nsub is not a postive integer
*/

struct am {
    double row;
    double col;
    double irr;
    double irc;
    double icc;
    double det;

    double rho4;
    double s2n;
    int nsub;
    int numiter;
    int flags;
};


#endif
