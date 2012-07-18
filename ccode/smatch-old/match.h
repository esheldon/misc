#ifndef _MATCH_H_TOKEN
#define _MATCH_H_TOKEN

#include <stdlib.h>
struct match {
    size_t index;
    double cos_dist;
};

int match_compare(const void *a, const void *b);

#endif
