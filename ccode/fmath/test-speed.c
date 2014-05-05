#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fmath.h"
#include <time.h>

int main(int argc, char **argv)
{

    double tot=0;
    int n=100000;
    int nrepeat=10000;
    double *d=malloc(n*sizeof(double));
    for (size_t i=0; i<n; i++) {
        d[i] = drand48();
    }

    time_t t1,t2;
    
    t1=clock();
    for (size_t irep=0; irep<nrepeat; irep++) {
        tot=0;
        for (size_t i=0; i<n; i++) {
            tot += exp(d[i]);
        }
    }
    t2=clock();
    double tstd = (t2-t1)/( (double)CLOCKS_PER_SEC );
    printf("time for std:  %.16g s\n", tstd);
    printf("total sum: %.16g\n", tot);



    t1=clock();
    for (size_t irep=0; irep<nrepeat; irep++) {
        tot=0;
        for (size_t i=0; i<n; i++) {
            tot += expd(d[i]);
        }
    }
    t2=clock();
    double tfast = (t2-t1)/( (double)CLOCKS_PER_SEC );
    printf("time for std:  %.16g s\n", tfast);
    printf("total sum: %.16g\n", tot);


    printf("fmath is faster by %.16g\n", tstd/tfast);


    return 0;
}
