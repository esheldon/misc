#ifndef _lensum_HEADER
#define _lensum_HEADER

#include "defs.h"

struct lensum {
    int64 zindex;
    double weight;

    int64 nbin;
    int64* npair;
    double* wsum;
    double* dsum;
    double* osum;
    double* rsum;
};

struct lensum* lensum_new(size_t nbin);
void lensum_clear(struct lensum* lensum);
struct lensum* lensum_delete(struct lensum* lensum);



#endif
