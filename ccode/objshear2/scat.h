#ifndef _SCAT_H
#define _SCAT_H

#include <vector>
#include "types.h"

using std::vector;

// Either we include the mean inverse critical density as a function of
// lens redshift or we include DS.
//
// note no padding is currently needed

#define NZL 20
struct source {
  
    float64 ra;
    float64 dec;

    float32 e1;
    float32 e2;
    float32 err; // error in each component, so we can use as-is in the tangential shear
                 // since it is a projection
    int32 htm_index;

#ifdef INTERP_SCINV
    // for this one we have to do the cosmology stuff in python and make sure
    // we use the same cosmology in the C++ code.  Maybe add an omega_m and
    // H0 to the binary source file?
    float32 mean_scinv[NZL];
#else
    // treat z as true
    float32 z;
    float32 dc; // calculate this in the C++ code.  Can be dummy
#endif

};



void read_source(
        const char* filename, 
        vector<struct source>& scat);

void add_source_distances(
        float H0,
        float omega_m,
        vector<struct source>& scat);

void print_source_row(
        vector<struct source>& scat, 
        int32 row);

void print_source_firstlast(vector<struct source>& scat);

#endif
