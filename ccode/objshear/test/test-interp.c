#include <stdio.h>
#include <stdlib.h>
#include "../Vector.h"
#include "../interp.h"

int main(int argc, char** argv) {
    size_t ndata=10;
    size_t ninterp=100;
    double step = ((double)ndata)/ninterp;

    struct f64vector* x=f64vector_range(ndata);
    struct f64vector* y=f64vector_new(ndata);

    for (size_t i=0; i<x->size; i++) {
        y->data[i] = x->data[i]*x->data[i];
    }

    for (size_t i=0; i<ninterp; i++) {
        double u = step*i;
        double v = f64interplin(x, y, u);

        printf("%lf %lf\n", u, v);
    }
}
