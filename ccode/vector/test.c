#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "vector.h"

#define wlog(...) fprintf(stderr, __VA_ARGS__)

struct test {
    int id;
    double x;
};

int compare_test(const void* t1, const void* t2) {
    int temp = 
        ((struct test*) t1)->id 
        -
        ((struct test*) t2)->id ;

    if (temp > 0)
        return 1;
    else if (temp < 0)
        return -1;
    else
        return 0;
}

void test_sort() {
    struct vector* v = vector_new(7, sizeof(struct test));

    struct test* t = NULL;

    t = vector_get(v,0);
    t->id = 4;
    t = vector_get(v,1);
    t->id = 1;
    t = vector_get(v,2);
    t->id = 2;
    t = vector_get(v,3);
    t->id = 0;
    t = vector_get(v,4);
    t->id = 3;
    t = vector_get(v,5);
    t->id = 6;
    t = vector_get(v,6);
    t->id = 5;

    vector_sort(v, &compare_test);

    size_t i=0;
    struct test* iter = vector_front(v);
    struct test* end  = vector_end(v);
    while (iter != end) {
        assert(iter->id == i);
        iter++;
        i++;
    }
}

void test_create_and_access() {
    size_t n=10;
    struct vector* v = vector_new(n, sizeof(struct test));

    assert(v->size == n);
    assert(n == vector_size(v));
    assert(v->capacity == n);

    struct test* iter = vector_front(v);
    struct test* end  = vector_end(v);
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
    struct test t;
    t.id = 57;
    t.x = -8.2457;
    vector_set(v, i, &t);
    struct test* el = vector_get(v, i);
    assert(el->id == t.id);
    assert(el->x == t.x);

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

void test_long() {

    struct vector* v = vector_new(0, sizeof(long));
    long n=10;
    long i=0;
    for (i=0; i<n; i++) {
        vector_push(v, &i);
    }

    long* iter = vector_front(v);
    long* end  = vector_end(v);
    i=0;
    while (iter != end) {
        assert(i == *iter);
        iter++;
        i++;
    }

    long* lptr = vector_get(v,3);
    assert(3 == *lptr);

    lptr = vector_pop(v);
    assert((n-1) == *lptr);

    v=vector_delete(v);
    assert(v==NULL);
}

/*
 * A test using a vector to hold pointers.  Note the vector does not "own" the
 * pointers, so allocation and free must happen separately
 *
 * If you want a vector of pointer that owns the data, use a pvector
 */

void test_ptr() {
    struct vector* v = vector_new(0, sizeof(struct test*));

    size_t i=0, n=10;

    // note we never own the pointers in the vector! So we must allocat and
    // free them separately
    struct test* tvec = calloc(n, sizeof(struct test));

    for (i=0; i<n; i++) {
        struct test *t = &tvec[i];
        t->id = i;
        t->x = 2*i;

        // this copies the pointer to t
        vector_push(v, &t);
    }

    // two different ways to use vector_get for pointers
    for (i=0; i<n; i++) {
        struct test **t = vector_get(v, i);
        assert((*t)->id == i);
        assert((*t)->x == 2*i);
    }
    for (i=0; i<n; i++) {
        struct test *t = *(struct test**) vector_get(v, i);
        assert(t->id == i);
        assert(t->x == 2*i);
    }

    // iteration
    i=0;
    struct test **iter = vector_front(v);
    struct test **end  = vector_end(v);
    while (iter != end) {
        assert((*iter)->id == i);
        iter++;
        i++;
    }

    v=vector_delete(v);
    assert(v==NULL);
    free(tvec);
}
int main(int argc, char** argv) {
    wlog("testing creating, get, set and iteration\n");
    test_create_and_access();
    wlog("testing realloc resize clear and freedata\n");
    test_realloc_resize();
    wlog("testing push pop and extend\n");
    test_pushpop();
    wlog("testing sort\n");
    test_sort();
    wlog("testing basic type long\n");
    test_long();
    wlog("testing pointer\n");
    test_ptr();
}
