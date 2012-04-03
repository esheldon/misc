#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "vector.h"

#define wlog(...) fprintf(stderr, __VA_ARGS__)

struct test {
    int id;
    double x;
};


void test_create_and_access() {
    size_t n=10;
    struct vector* v = vector_new(n, sizeof(struct test));

    assert(v->size == n);
    assert(v->capacity == n);

    struct test* iter  = vector_front(v);
    struct test* end = vector_end(v);
    size_t i=0;
    while (iter != end) {
        iter->id = i;
        iter->x  = 2*i;
        wlog("    id: %d  x: %g\n", iter->id, iter->x);
        iter++;
        i++;
    }

    iter = vector_front(v);
    i=0;
    while (iter != end) {
        assert(iter->id == i);
        assert(iter->x == 2*i);
        iter++;
        i++;
    }


    i=7;
    struct test* el = vector_get(v, i);
    assert(el->id == i);
    assert(el->x == 2*i);

    v = vector_delete(v);
    assert(v == NULL);
}
void test_realloc_resize() {
    size_t n=10;
    struct vector* v = vector_new(n, sizeof(struct test));

    assert(v->size == n);
    assert(v->capacity == n);

    size_t new_n = 12;
    vector_realloc(v, new_n);
    assert(v->size == n);
    assert(v->capacity == new_n);

    new_n = 7;
    vector_realloc(v, new_n);
    assert(v->size == new_n);
    assert(v->capacity == new_n);

    size_t rsn = 6;
    vector_resize(v,rsn);
    assert(v->size == rsn);
    assert(v->capacity == new_n);

    rsn = 12;
    vector_resize(v,rsn);
    assert(v->size == rsn);
    assert(v->capacity == rsn);

    vector_clear(v);
    assert(v->size == 0);
    assert(v->capacity == rsn);

    vector_freedata(v);
    assert(v->size == 0);
    assert(v->capacity == 0);
    assert(v->d == NULL);

    v = vector_delete(v);
    assert(v == NULL);
}
void test_pushpop() {
    struct vector* v = vector_new(0, sizeof(struct test));

    size_t i=0;
    size_t n=10;
    struct test t;
    for (i=0; i<n; i++) {
        t.id = i;
        t.x = 2*i;
        vector_push(v, &t);
    }

    struct test *tptr=NULL;
    for (i=0; i<n; i++) {
        tptr = vector_get(v,i);
        assert(tptr->id == i);
        assert(tptr->x == 2*i);
    }

    i=n-1;
    while (NULL != (tptr=vector_pop(v))) {
        assert(tptr->id == i);
        assert(tptr->x == 2*i);
        i--;
    }

    assert(v->size == 0);
    assert(v->capacity >= n);

    v = vector_delete(v);
    assert(v == NULL);
}

void test_extend() {
    struct vector* v = vector_new(0, sizeof(struct test));

    size_t i=0;
    size_t n=10;
    struct test* tptr;
    for (i=0; i<n; i++) {
        tptr = vector_extend(v);
        tptr->id = i;
        tptr->x = 2*i;
    }

    for (i=0; i<n; i++) {
        tptr = vector_get(v,i);
        assert(tptr->id == i);
        assert(tptr->x == 2*i);
    }

    v = vector_delete(v);
    assert(v == NULL);
}


int main(int argc, char** argv) {
    test_create_and_access();
    test_realloc_resize();
    test_pushpop();
}
