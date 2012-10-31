#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fmath.h"

int main(int argc, char **argv)
{

    double xmin=-20;
    double xmax=10;
    size_t nstep=10000000;

    double stepsize=(xmax-xmin)/nstep;

    double max_fdiff=-9999;
    double max_xval=-9999;

    for (size_t i=0; i<nstep; i++) {
        double x = xmin + i*stepsize;

        double val=exp(x);
        double approx_val = expd(x);

        double fdiff=approx_val/val-1;

        if (fdiff > max_fdiff) {
            max_fdiff=fdiff;
            max_xval=x;
        }
    }

    printf("xmin: %.16g xmax: %.16g stepsize: %.16g\n", xmin, xmax, stepsize);
    printf("max fdiff was %.16g for value %.16g\n", max_fdiff, max_xval);
    //printf("val: %.16g approx val: %.16g fdiff: %.16g\n", val, approx_val, approx_val/val-1.);

    return 0;
}
