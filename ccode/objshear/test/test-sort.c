#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../Vector.h"
#include "../sort.h"

int main(int argc, char** argv) {
    struct i64vector* v = i64vector_new(25);

    srand(time(NULL));

    for (size_t i=0; i<v->size; i++) {
        v->data[i] = rand();
        printf("  v[%ld]: %ld\n", i, v->data[i]);
    }

    struct szvector* s = i64sortind(v);

    for (size_t i=0; i<v->size; i++) {
        printf("  v[s[%ld]]: %ld\n", i, v->data[s->data[i]]);
    }

    szvector_delete(s);

}
