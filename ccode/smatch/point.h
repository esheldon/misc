#ifndef _POINT_H
#define _POINT_H

#include "defs.h"
#include "VEC.h"

struct point {
    int64 index;
    double x;
    double y;
    double z;
    double cos_radius;
};

typedef struct point Point;
VEC_DEF(Point);

#endif
