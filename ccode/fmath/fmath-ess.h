#ifndef _FMATH_ESS_HEADER_GUARD
#define _FMATH_ESS_HEADER_GUARD

#include <stdint.h>

union fmath_di {
    double d;
    uint64_t i;
};

static inline double expd(double x)
{

// holds definition of the table and C1,C2,C3, a, ra
#include "fmath-dtbl.c"

    union fmath_di di;

    di.d = x * a + b;
    uint64_t iax = dtbl[di.i & sbit_masked];

    double t = (di.d - b) * ra - x;
    uint64_t u = ((di.i + adj) >> sbit) << 52;
    //double y = (C3[0] - t) * (t * t) * C2[0] - t + C1[0];
    double y = (C3 - t) * (t * t) * C2 - t + C1;

    di.i = u | iax;
    return y * di.d;
}



#endif
