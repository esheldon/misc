#ifndef _POINT_H
#define _POINT_H

#include "defs.h"

struct point {
    int64 index;
    double x;
    double y;
    double z;
    double cos_radius;
};

#endif
