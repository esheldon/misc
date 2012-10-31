#ifndef _FMATH_ESS_HEADER_GUARD
#define _FMATH_ESS_HEADER_GUARD

#include <stdint.h>

#define FMATH_EXPD_TABLE_SIZE  11
#define FMATH_EXPD_S 1UL << FMATH_EXPD_TABLE_SIZE
#define FMATH_EXPD_ADJ (1UL << (FMATH_EXPD_TABLE_SIZE + 10)) - (1UL << FMATH_EXPD_TABLE_SIZE)

#define M_LOG2_INV 1.4426950408889634

union fmath_di {
    double d;
    uint64_t i;
};

static inline unsigned int fmath_mask(int x)
{
    return (1U << x) - 1;
}

static inline uint64_t fmath_mask64(int x)
{
    return (1ULL << x) - 1;
}

static inline double expd(double x)
{

// holds definition of the table and C1,C2,C3, a, ra
#include "fmath-dtbl.c"

    union fmath_di di;

    di.d = x * a + b;
    uint64_t iax = dtbl[di.i & fmath_mask(sbit)];

    double t = (di.d - b) * ra - x;
    uint64_t u = ((di.i + adj) >> sbit) << 52;
    double y = (C3[0] - t) * (t * t) * C2[0] - t + C1[0];

    di.i = u | iax;
    return y * di.d;
}



#endif
