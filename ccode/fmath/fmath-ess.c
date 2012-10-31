#include <stdint.h>
#include "fmath-ess.h"


static inline uint64_t mask64(int x)
{
    return (1ULL << x) - 1;
}

/*
inline double expd(double x)
{
    const ExpdVar<>& c = C<>::expdVar;
    const uint64_t b = 3ULL << 51;
    di di;
    di.d = x * c.a + b;
    uint64_t iax = c.tbl[di.i & mask(c.sbit)];

    double t = (di.d - b) * c.ra - x;
    uint64_t u = ((di.i + c.adj) >> c.sbit) << 52;
    double y = (c.C3[0] - t) * (t * t) * c.C2[0] - t + c.C1[0];
    //	double y = (2.999796930327879362111743 - t) * (t * t) * 0.166677948823102161853172 - t + 1.000000000000000000488181;

    di.i = u | iax;
    return y * di.d;
}
*/



