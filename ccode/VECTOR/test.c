#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "VECTOR.h"

#define wlog(...) fprintf(stderr, __VA_ARGS__)

struct mystruct {
    int id;
    double x;
};

// we need these for structs and pointers to work
typedef struct mystruct MyStruct;
typedef struct mystruct* MyStruct_p;

VECTOR_DECLARE(long);
VECTOR_DECLARE(MyStruct);
VECTOR_DECLARE(MyStruct_p);

int compare_test(const void* t1, const void* t2) {
    int temp = 
        ((struct mystruct*) t1)->id 
        -
        ((struct mystruct*) t2)->id ;

    if (temp > 0)
        return 1;
    else if (temp < 0)
        return -1;
    else
        return 0;
}

void test_sort() {
    VECTOR(MyStruct) v=NULL;
    VECTOR_INIT(MyStruct, v);

    struct mystruct t;

    t.id = 4;
    VECTOR_PUSH(MyStruct, v, t);
    t.id = 1;
    VECTOR_PUSH(MyStruct, v, t);
    t.id = 2;
    VECTOR_PUSH(MyStruct, v, t);
    t.id = 0;
    VECTOR_PUSH(MyStruct, v, t);
    t.id = 3;
    VECTOR_PUSH(MyStruct, v, t);
    t.id = 6;
    VECTOR_PUSH(MyStruct, v, t);
    t.id = 5;
    VECTOR_PUSH(MyStruct, v, t);

    VECTOR_SORT(MyStruct, v, &compare_test);

    size_t i=0;
    struct mystruct* iter = VECTOR_ITER(v);
    struct mystruct* end  = VECTOR_END(v);
    while (iter != end) {
        assert(iter->id == i);
        iter++;
        i++;
    }
}

void test_pass_struct(VECTOR(MyStruct) v) {
    struct mystruct *t=VECTOR_GETPTR(v,2);
    assert(2 == t->id);
    assert(2*2 == t->x);
}
void test_struct() {
    size_t n=10, cap=16, i=0;

    VECTOR(MyStruct) v=NULL;
    VECTOR_INIT(MyStruct, v);

    struct mystruct tmp;
    for (i=0; i<n; i++) {
        tmp.id = i;
        tmp.x = 2*i;
        VECTOR_PUSH(MyStruct,v,tmp);
        assert((i+1) == VECTOR_SIZE(v));
    }

    assert(cap == VECTOR_CAPACITY(v));
    assert(n == VECTOR_SIZE(v));


    MyStruct val = VECTOR_POP(MyStruct,v);
    assert((n-1) == val.id);
    assert((n-1) == VECTOR_SIZE(v));
    n--;

    // faster but unsafe
    val = VECTOR_POPFAST(v);
    assert((n-1) == val.id);
    assert((n-1) == VECTOR_SIZE(v));
    n--;

    struct mystruct *iter = VECTOR_ITER(v);
    struct mystruct *end  = VECTOR_END(v);
    i=0;
    //while (iter != end) {
    for (; iter != end; iter++) {
        assert(i == iter->id);
        assert(2*i == iter->x);
        i++;
    }

    // no copy
    struct mystruct *tp = VECTOR_GETPTR(v,5);
    assert(tp->id == 5);
    assert(tp->x == 2*5);

    // copy made
    tmp = VECTOR_GET(v,5);
    assert(tmp.id == 5);
    assert(tmp.x == 2*5);

    tmp = VECTOR_GETFRONT(v);
    assert(tmp.id == 0);
    assert(tmp.x == 0);

    n=VECTOR_SIZE(v);
    tmp = VECTOR_GETBACK(v);
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


    VECTOR_RESIZE(MyStruct, v, 10);
    assert(cap == VECTOR_CAPACITY(v));
    assert(10 == VECTOR_SIZE(v));

    VECTOR_CLEAR(MyStruct, v);
    assert(cap == VECTOR_CAPACITY(v));
    assert(0 == VECTOR_SIZE(v));

    VECTOR_DROP(MyStruct, v);
    assert(1 == VECTOR_CAPACITY(v));
    assert(0 == VECTOR_SIZE(v));

    // nothing left, so popping should give a zeroed struct
    val = VECTOR_POP(MyStruct,v);
    assert(0 == val.id);
    assert(0 == val.x);
    assert(0 == VECTOR_SIZE(v));

    VECTOR_DELETE(MyStruct,v);
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


    size_t newsize=10;
    VECTOR_RESIZE(long, v, newsize);
    assert(cap == VECTOR_CAPACITY(v));
    assert(newsize == VECTOR_SIZE(v));


    VECTOR_SET(v,3,12);
    assert(12 == VECTOR_GET(v,3));

    long *p = VECTOR_GETPTR(v,3);
    assert(12 == *p);

    long val = VECTOR_POP(long,v);
    assert((newsize-1) == val);
    assert((newsize-1) == VECTOR_SIZE(v));

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

    VECTOR(MyStruct_p) v=NULL;
    VECTOR_INIT(MyStruct_p, v);

    // note we never own the pointers in the vector! So we must allocat and
    // free them separately
    struct mystruct* tvec = calloc(n, sizeof(struct mystruct));

    for (i=0; i<n; i++) {
        struct mystruct *t = &tvec[i];
        t->id = i;
        t->x = 2*i;

        // this copies the pointer to t
        VECTOR_PUSH(MyStruct_p, v, t);
    }

    for (i=0; i<n; i++) {
        struct mystruct *t = VECTOR_GET(v, i);
        assert(t->id == i);
        assert(t->x == 2*i);
    }

    // iteration
    // note different pointer declarations
    i=0;
    MyStruct_p *iter = VECTOR_ITER(v);
    MyStruct **end  = VECTOR_END(v);
    while (iter != end) {
        assert((*iter)->id == i);
        iter++;
        i++;
    }
    VECTOR_DELETE(MyStruct_p, v);

    // make sure the data still exist!
    assert(3 == tvec[3].id);
    assert(2*3 == tvec[3].x);

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
