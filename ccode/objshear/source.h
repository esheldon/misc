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

    double sinra;
    double sindec;
    double cosra;
    double cosdec;
    double g1;
    double g2;
    double err;

    int64 hpixid;

#ifdef SOURCE_POFZ
    struct f64vector* scinv; // note this is same size as zlens kept in 
                             // catalog structure.
    struct f64vector* zlens; // For convenience; this should just point 
                             // to memory owned by scat->zlens; don't 
                             // allocate or free!
#else
    double z;
    double dc;
#endif

};

struct scat {
    size_t size;
    struct source* data;
#ifdef SOURCE_POFZ
    struct f64vector* zlens; // for scinv(zlens).  Memory is shared in 
                             // source data
#endif
};


#ifdef SOURCE_POFZ
struct scat* scat_new(size_t n_source, size_t n_zlens);
#else
struct scat* scat_new(size_t n_source);
#endif

struct scat* scat_read(const char* filename);

struct scat* scat_delete(struct scat* scat);

#endif
