#ifndef _LCAT_H
#define _LCAT_H
#include "types.h"

// we must put the 8-byte ones at the beginning or the packing
// gets screwed up
struct lens {

    float64 ra;
    float64 dec;

    float32 z;

    // we will fill these in internally
    float32 aeta_rel;
    float32 Da; // angdist in kpc.  angmax = rmax/DL

    int32 zindex;

};

void read_lens(
        const char* filename, 
        std::vector<struct lens>& lcat);

void add_lens_distances(
        float H0, 
        float omega_m, 
        std::vector<struct lens>& lcat);

void print_lens_row(
        std::vector<struct lens>& lcat, 
        int32 row);

void print_lens_firstlast(std::vector<struct lens>& lcat);

#endif
