#include <stdlib.h>
#include <stdio.h>
#include "shape.h"

int main(int argc, char **argv)
{
    double val1=0.2, val2=-0.3;

    printf("setting each time from %g %g\n", val1, val2);
    printf("\nsetting from e\n");
    struct shape *shape=shape_new_e(val1,val2);
    shape_show(shape,stdout);

    printf("\nsetting from g\n");
    shape_set_g(shape,val1,val2);
    shape_show(shape,stdout);

    printf("\nsetting from eta\n");
    shape_set_eta(shape,0.2, -0.3);
    shape_show(shape,stdout);

    double sh1=0.02, sh2=0.04;
    printf("\nshearing: %g %g\n", sh1, sh2);
    struct shape *shear=shape_new_e(sh1,sh2);

    shape_add_inplace(shape,shear);
    shape_show(shape,stdout);

    shape=shape_free(shape);
    shear=shape_free(shear);
    return 0;
}
