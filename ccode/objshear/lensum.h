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

struct lensums {
    size_t size;
    struct lensum* data;
};


struct lensums* lensums_new(size_t nlens, size_t nbin);
struct lensums* lensums_delete(struct lensums* lensum);

struct lensum* lensum_new(size_t nbin);
void lensum_clear(struct lensum* lensum);
struct lensum* lensum_delete(struct lensum* lensum);



#endif
