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

    shape_set_g(shape, 0.2, 0.1);
    shape_set_g(shear, 0.04, 0.0);

    double jacob_g=shape_dgs_by_dgo_jacob(shape, shear);
    double jacob_g_num=shape_dgs_by_dgo_jacob_num(shape, shear);
    double jacob_eta=shape_detas_by_detao_jacob(shape, shear);

    printf("\n");
    printf("jacob g:     %.16g\n", jacob_g);
    printf("jacob g num: %.16g\n", jacob_g_num);
    printf("jacob eta:   %.16g\n", jacob_eta);

    shape=shape_free(shape);
    shear=shape_free(shear);

    return 0;
}
