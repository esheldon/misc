#ifndef _MATCH_H_TOKEN
#define _MATCH_H_TOKEN

#include <stdlib.h>
#include "VEC.h"

struct match {
    size_t index;
    double cos_dist;
};

typedef struct match Match;
VEC_DEF(Match);

int match_compare(const void *a, const void *b);

#endif
