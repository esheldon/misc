#include <stdlib.h>
#include "mca_rand.h"

long mca_randlong(long n)
{
    return lrand48() % n;
}

long mca_rand_complement(long current, long n)
{
    long i=current;
    while (i == current) {
        i = mca_randlong(n);
    }
    return i;
}
