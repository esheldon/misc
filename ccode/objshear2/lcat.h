#ifndef _LCAT_H
#define _LCAT_H

#include <vector>
#include "types.h"

using std::vector;

// we must put the 8-byte ones at the beginning or the packing
// gets screwed up
struct lens {

    float64 ra;
    float64 dec;

    float32 z;

    // we will fill this in internally
    float32 dc; // Comoving distance in Mpc.

    int32 zindex;

    int32 padding;

};

void read_lens(
        const char* filename, 
        vector<struct lens>& lcat);

void add_lens_distances(
        float H0, 
        float omega_m, 
        vector<struct lens>& lcat);

void print_lens_row(
        vector<struct lens>& lcat, 
        int32 row);

void print_lens_firstlast(vector<struct lens>& lcat);

#endif
