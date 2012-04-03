#include <stdlib.h>
#include <stdio.h>
#include "vector.h"

#define wlog(...) fprintf(stderr, __VA_ARGS__)

struct test {
    int id;
    double x;
}

int main(int argc, char** argv) {
    wlog("allocating vector\n");
    struct vector* v = vector_new(25, sizeof(struct test));

    wlog("allocated %lu elements\n", v->size);

    struct test* iter  = vector_front(v);
    struct test* last = vector_back(v);
    do {
        wlog("    id: %d  x: %g\n", iter->id, iter->x);
        iter++;
    } while (iter != last);
    /*
    for (size_t i=0; i<v->size; i++) {
        wlog("el: %lu  id: %d  x: %g\n", i, v->d[i].id, v->d[i].x);
    }
    */
}
