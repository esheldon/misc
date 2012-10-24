#include <stdlib.h>
#include <stdio.h>
#include "matrix.h"
#include "defs.h"

int main(int argc, char** argv)
{
    struct vec2 *v1 = vec2_fromdata(1., 3.);
    struct vec2 *v2 = vec2_new();
    struct vec2 *v3 = vec2_new();
    struct mtx2 *mat1 = mtx2_fromdata(2, 1, 4);
    struct mtx2 *mat1inv = mtx2_new();
    struct mtx2 *mat2 = mtx2_new();
    struct mtx2 *mat3 = mtx2_new();

    const char *fmt = "%10.6lf";

    mtx2_vec2prod(mat1, v1, v2);

    wlog("mat1\n");
    mtx2_print(mat1, stderr, fmt);
    wlog("det: %g\n", mat1->det);

    wlog("vec1\n");
    vec2_print(v1, stderr, fmt);
    wlog("norm: %g\n", v1->norm);

    wlog("vec2 = mat1*vec1\n");
    vec2_print(v2, stderr, fmt);

    mtx2_invert(mat1, mat1inv);
    wlog("mat1inverse\n");
    mtx2_print(mat1inv, stderr, fmt);


    mtx2_sprod(mat1, 2.0, mat2);
    wlog("\nmat1*2\n");
    mtx2_print(mat2, stderr, fmt);
    wlog("det: %g\n",mat2->det);

    mtx2_sprodi(mat1, 2.0);
    wlog("mat1*2 in place\n");
    mtx2_print(mat1, stderr, fmt);
    wlog("det: %g\n\n",mat1->det);
    

    vec2_sprod(v1, 2.0, v2);
    wlog("v1*2\n");
    vec2_print(v2,stderr,fmt);
    wlog("norm: %g\n",v2->norm);

    vec2_sprodi(v1, 2.0);
    wlog("v1*2 in place\n");
    vec2_print(v1,stderr,fmt);
    wlog("norm: %g\n",v1->norm);


    vec2_sum(v1, v2, v3);
    wlog("\nv1 + v2\n");
    vec2_print(v3, stderr, fmt);
    wlog("norm: %g\n",v3->norm);

    vec2_sumi(v1, v2);
    wlog("v1 + v2 in place\n");
    vec2_print(v1, stderr, fmt);
    wlog("norm: %g\n\n",v1->norm);



    mtx2_sum(mat1, mat2, mat3);
    wlog("\nmat1 + mat2\n");
    mtx2_print(mat3, stderr, fmt);
    wlog("det: %g\n",mat3->det);

    mtx2_sumi(mat1, mat2);
    wlog("mat1 + mat2 in place\n");
    mtx2_print(mat1, stderr, fmt);
    wlog("det: %g\n\n",mat1->det);

}
