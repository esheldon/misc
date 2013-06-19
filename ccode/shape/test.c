#include <stdlib.h>
#include <stdio.h>
#include "shape.h"

int main(int argc, char **argv)
{
    struct shape *shape=shape_new_e1e2(0.2, -0.3);

    shape_show(shape,stdout);

    shape_set_g1g2(shape,0.2, -0.3);

    shape_show(shape,stdout);

    struct shape *shear=shape_new_e1e2(0.02,0.04);

    struct shape *new=shape_add(shape,shear);
    shape_show(new,stdout);

    shape=shape_free(shape);
    shear=shape_free(shear);
    return 0;
}
