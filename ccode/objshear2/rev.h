#ifndef _REV_H
#define _REV_H

#include <vector>
#include "types.h"

using std::vector;

struct revhtm {
    int32 minid;
    int32 maxid;
    vector<int32> revind;
};

void read_rev(const char* filename, struct revhtm& rev);
void print_rev_sample(struct revhtm& rev);

#endif
