#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "VECTOR.h"

#define wlog(...) fprintf(stderr, __VA_ARGS__)

struct test {
    int id;
    double x;
};

// we need these for structs and pointers to work
typedef struct test struct_test;
typedef struct test* struct_testp;

VECTOR_DECLARE(long);
VECTOR_DECLARE(struct_test);
VECTOR_DECLARE(struct_testp);

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
    VECTOR(struct_test) v=NULL;
    VECTOR_INIT(struct_test, v);

    struct test t;

    t.id = 4;
    VECTOR_PUSH(struct_test, v, t);
    t.id = 1;
    VECTOR_PUSH(struct_test, v, t);
    t.id = 2;
    VECTOR_PUSH(struct_test, v, t);
    t.id = 0;
    VECTOR_PUSH(struct_test, v, t);
    t.id = 3;
    VECTOR_PUSH(struct_test, v, t);
    t.id = 6;
    VECTOR_PUSH(struct_test, v, t);
    t.id = 5;
    VECTOR_PUSH(struct_test, v, t);

    VECTOR_SORT(struct_test, v, &compare_test);

    size_t i=0;
    struct test* iter = VECTOR_ITER(v);
    struct test* end  = VECTOR_END(v);
    while (iter != end) {
        assert(iter->id == i);
        iter++;
        i++;
    }
}

void test_pass_struct(VECTOR(struct_test) v) {
    struct test *t=VECTOR_GETPTR(v,2);
    assert(2 == t->id);
    assert(2*2 == t->x);
}
void test_struct() {
    size_t n=10, cap=16, i=0;

    VECTOR(struct_test) v=NULL;
    VECTOR_INIT(struct_test, v);

    struct test tmp;
    for (i=0; i<n; i++) {
        tmp.id = i;
        tmp.x = 2*i;
        VECTOR_PUSH(struct_test,v,tmp);
        assert((i+1) == VECTOR_SIZE(v));
    }

    assert(cap == VECTOR_CAPACITY(v));
    assert(n == VECTOR_SIZE(v));

    struct test *iter = VECTOR_ITER(v);
    struct test *end  = VECTOR_END(v);
    i=0;
    while (iter != end) {
        assert(i == iter->id);
        assert(2*i == iter->x);
        iter++;
        i++;
    }

    // no copy
    struct test *tp = VECTOR_GETPTR(v,5);
    assert(tp->id == 5);
    assert(tp->x == 2*5);

    // copy made
    tmp = VECTOR_GET(v,5);
    assert(tmp.id == 5);
    assert(tmp.x == 2*5);

    tmp = VECTOR_FRONT(v);
    assert(tmp.id == 0);
    assert(tmp.x == 0);

    tmp = VECTOR_BACK(v);
    assert(tmp.id == (n-1));
    assert(tmp.x == 2*(n-1));

    wlog("  testing pass vector of structs\n");
    test_pass_struct(v);

    tmp.id = 3423;
    tmp.x = 500;
    VECTOR_SET(v, 3, tmp);
    tp = VECTOR_GETPTR(v,3);
    assert(tp->id == tmp.id);
    assert(tp->x == tmp.x);


    VECTOR_RESIZE(struct_test, v, 10);
    assert(cap == VECTOR_CAPACITY(v));
    assert(10 == VECTOR_SIZE(v));

    VECTOR_CLEAR(struct_test, v);
    assert(cap == VECTOR_CAPACITY(v));
    assert(0 == VECTOR_SIZE(v));

    VECTOR_DROP(struct_test, v);
    assert(1 == VECTOR_CAPACITY(v));
    assert(0 == VECTOR_SIZE(v));


    VECTOR_DELETE(struct_test,v);
    assert(NULL==v);
}

void test_long() {

    long n=10, cap=16, i=0;

    VECTOR(long) v=NULL;
    VECTOR_INIT(long, v);

    for (i=0; i<n; i++) {
        VECTOR_PUSH(long,v,i);
        assert((i+1) == VECTOR_SIZE(v));
    }
    assert(cap == VECTOR_CAPACITY(v));
    assert(n == VECTOR_SIZE(v));

    long *iter=VECTOR_ITER(v);
    long *end=VECTOR_END(v);
    i=0;
    while (iter != end) {
        assert(*iter == i);
        i++;
        iter++;
    }


    VECTOR_RESIZE(long, v, 10);
    assert(cap == VECTOR_CAPACITY(v));
    assert(10 == VECTOR_SIZE(v));


    VECTOR_SET(v,3,12);
    assert(12 == VECTOR_GET(v,3));

    long *p = VECTOR_GETPTR(v,3);
    assert(12 == *p);

    VECTOR_RESIZE(long, v, 3);
    assert(cap == VECTOR_CAPACITY(v));
    assert(3 == VECTOR_SIZE(v));


    VECTOR_CLEAR(long, v);
    assert(cap == VECTOR_CAPACITY(v));
    assert(0 == VECTOR_SIZE(v));
    VECTOR_DROP(long, v);
    assert(1 == VECTOR_CAPACITY(v));
    assert(0 == VECTOR_SIZE(v));

    VECTOR_DELETE(long, v);
    assert(NULL == v);
}

/*
 * A test using a vector to hold pointers.  Note the vector does not "own" the
 * pointers, so allocation and free must happen separately
 *
 * If you want a vector of pointer that owns the data, use a pvector
 */

void test_ptr() {
    size_t i=0, n=10;

    VECTOR(struct_testp) v=NULL;
    VECTOR_INIT(struct_testp, v);

    // note we never own the pointers in the vector! So we must allocat and
    // free them separately
    struct test* tvec = calloc(n, sizeof(struct test));

    for (i=0; i<n; i++) {
        struct test *t = &tvec[i];
        t->id = i;
        t->x = 2*i;

        // this copies the pointer to t
        VECTOR_PUSH(struct_testp, v, t);
    }

    // two different ways to use vector_get for pointers
    for (i=0; i<n; i++) {
        struct test *t = VECTOR_GET(v, i);
        assert(t->id == i);
        assert(t->x == 2*i);
    }

    // iteration
    // We could also use declaration as 
    //     struct_testp *iter=VECTOR_ITER(v);
    i=0;
    struct test **iter = VECTOR_ITER(v);
    struct test **end  = VECTOR_END(v);
    while (iter != end) {
        assert((*iter)->id == i);
        iter++;
        i++;
    }
    VECTOR_DELETE(struct_testp, v);
    assert(v==NULL);
    free(tvec);
}
int main(int argc, char** argv) {
    wlog("testing struct vector\n");
    test_struct();
    wlog("testing sort struct vector\n");
    test_sort();
    wlog("testing long vector\n");
    test_long();
    wlog("testing pointers to structs\n");
    test_ptr();
}