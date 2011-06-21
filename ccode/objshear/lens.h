#ifndef _LENS_HEADER
#define _lens_HEADER

#include "defs.h"

struct lens {

    double ra;
    double dec;
    double z;
    int64 zindex;

    // calculate these for speed later
    double dc;
    double sinra;
    double cosra;
    double sindec;
    double cosdec;
};


struct lcat {
    size_t size;
    struct lens* data;
};

struct lcat* lcat_new(size_t n_lens);
struct lcat* lcat_read(const char* filename);

void lcat_print_one(struct lcat* lcat, size_t el);
void lcat_print_firstlast(struct lcat* lcat);

struct lcat* lcat_delete(struct lcat* lcat);



#endif
