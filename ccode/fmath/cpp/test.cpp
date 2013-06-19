#include <stdio.h>
#include <stdlib.h>
#include "fmath.hpp"
#include <math.h>

int main(int argc, char **argv)
{

    double x=1.0;

    double val=exp(x);
    double approx_val = fmath::expd(x);

    printf("val:        %.16g\n", val);
    printf("approx val: %.16g\n", approx_val);

    return 0;
}
