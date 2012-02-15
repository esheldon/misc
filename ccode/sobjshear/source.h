#ifndef _SOURCE_HEADER
#define _SOURCE_HEADER

#include "Vector.h"
#include "defs.h"
/* 
  We can work in two modes:
    - we have an inverse critical density; in this case the 
      header of the file must contain the associated zlens values
    - We have a specific z value for this object; we will generate
      the associated dc (comoving distance) value for speed

  Note we actually want the sin and cos of ra/dec rather than ra
  dec for our calculations.
*/
struct source {

    double ra;
    double dec;

    double g1;
    double g2;
    double err;

    int64 hpixid;

#ifndef WITH_TRUEZ
    struct f64vector* scinv; // note this is same size as zlens kept in 
                             // catalog structure.
    struct f64vector* zlens; // For convenience; this should just point 
                             // to memory owned by config->zlens; don't 
                             // allocate or free!
#else
    double z;
    double dc; // for speed
#endif

    // calculate these for speed
    double sinra; 
    double cosra;
    double sindec;
    double cosdec;

#ifdef SDSSMASK
    double sinlam;
    double coslam;
    double sineta;
    double coseta;
#endif
};


#ifdef WITH_TRUEZ
struct source* source_new(void);
#else
struct source* source_new(size_t n_zlens);
#endif

int source_read(FILE* stream, struct source* src);

void source_print(struct source* src);

struct source* source_delete(struct source* src);

#endif
