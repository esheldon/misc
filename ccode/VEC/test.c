#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "VEC.h"

#define wlog(...) fprintf(stderr, __VA_ARGS__)

// must use typedefs for use in vectors
typedef struct mystruct {
    int id;
    double x;
} MyStruct;

typedef MyStruct* MyStruct_p;

VEC_DEF(long);
VEC_DEF(MyStruct);
VEC_DEF(MyStruct_p);
VEC_DEF(VEC(long));

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
    VEC(MyStruct) v = VEC_NEW(MyStruct);

    MyStruct t;

    t.id = 4;
    VEC_PUSH(v, t);
    t.id = 1;
    VEC_PUSH(v, t);
    t.id = 2;
    VEC_PUSH(v, t);
    t.id = 0;
    VEC_PUSH(v, t);
    t.id = 3;
    VEC_PUSH(v, t);
    t.id = 6;
    VEC_PUSH(v, t);
    t.id = 5;
    VEC_PUSH(v, t);

    VEC_SORT(v, &compare_test);

    size_t i=0;
    MyStruct* iter = VEC_BEGIN(v);
    MyStruct* end  = VEC_END(v);
    for (; iter != end; iter++) {
        assert(iter->id == i);
        i++;
    }
}

void test_pass_struct(VEC(MyStruct) v) {
    struct mystruct *t=VEC_GETPTR(v,2);
    assert(2 == t->id);
    assert(2*2 == t->x);
}
void test_struct() {
    size_t n=10, cap=16, i=0;

    VEC(MyStruct) v = VEC_NEW(MyStruct);

    struct mystruct tmp;
    for (i=0; i<n; i++) {
        tmp.id = i;
        tmp.x = 2*i;
        VEC_PUSH(v,tmp);
        assert((i+1) == VEC_SIZE(v));
    }

    assert(cap == VEC_CAPACITY(v));
    assert(n == VEC_SIZE(v));


    MyStruct val = VEC_POP(v);
    assert((n-1) == val.id);
    assert((n-1) == VEC_SIZE(v));
    n--;

    // faster but unsafe
    val = VEC_POPFAST(v);
    assert((n-1) == val.id);
    assert((n-1) == VEC_SIZE(v));
    n--;


    i=0;
    VEC_FOREACH_BEG(iter, v)
        assert(i == iter->id);
        assert(2*i == iter->x);
        i++;
    VEC_FOREACH_END

    i=0;
    VEC_FOREACH(iter, v) {
        assert(i == iter->id);
        assert(2*i == iter->x);
        i++;
    }

    // no copy
    struct mystruct *tp = VEC_GETPTR(v,5);
    assert(tp->id == 5);
    assert(tp->x == 2*5);

    // copy made
    tmp = VEC_GET(v,5);
    assert(tmp.id == 5);
    assert(tmp.x == 2*5);

    tmp = VEC_GETFRONT(v);
    assert(tmp.id == 0);
    assert(tmp.x == 0);

    n=VEC_SIZE(v);
    tmp = VEC_GETBACK(v);
    assert(tmp.id == (n-1));
    assert(tmp.x == 2*(n-1));

    wlog("  testing pass vector of structs\n");
    test_pass_struct(v);

    tmp.id = 3423;
    tmp.x = 500;
    VEC_SET(v, 3) = tmp;
    tp = VEC_GETPTR(v,3);
    assert(tp->id == tmp.id);
    assert(tp->x == tmp.x);


    VEC_RESIZE(v, 10);
    assert(cap == VEC_CAPACITY(v));
    assert(10 == VEC_SIZE(v));

    VEC_CLEAR(v);
    assert(cap == VEC_CAPACITY(v));
    assert(0 == VEC_SIZE(v));

    VEC_DROP(v);
    assert(1 == VEC_CAPACITY(v));
    assert(0 == VEC_SIZE(v));

    // nothing left, so popping should give a zeroed struct
    val = VEC_POP(v);
    assert(0 == val.id);
    assert(0 == val.x);
    assert(0 == VEC_SIZE(v));

    VEC_FREE(v);
    assert(NULL==v);
}

void test_not_owned() {
    long v[3];
    long z[4];

    v[0]=-32;
    v[1]=8;
    v[2]=234;

    z[0]=25;
    z[1]=3;
    z[2]=7;
    z[3]=9;

    // not owned
    VEC(long) vref=VEC_REFDATA(long, v, 3);

    assert(0 == VEC_OWNER(vref));
    assert(3 == VEC_SIZE(vref));
    assert(3 == VEC_CAPACITY(vref));
    assert(-32 == VEC_GET(vref,0));
    assert(8 == VEC_GET(vref,1));
    assert(234 == VEC_GET(vref,2));

    // associate new data
    VEC_ASSOC(vref, z, 4);
    assert(0 == VEC_OWNER(vref));
    assert(4 == VEC_SIZE(vref));
    assert(4 == VEC_CAPACITY(vref));
    assert(25 == VEC_GET(vref,0));
    assert(3 == VEC_GET(vref,1));
    assert(7 == VEC_GET(vref,2));
    assert(9 == VEC_GET(vref,3));

    VEC_SET(vref,2) = -9999;
    assert(-9999 == z[2]);

    fprintf(stderr,"  expect error message here: ");
    VEC_RESIZE(vref,20);

    VEC_FREE(vref);
}
void test_long() {

    long n=10, cap=16, i=0;

    VEC(long) v = VEC_NEW(long);

    for (i=0; i<n; i++) {
        VEC_PUSH(v,i);
        assert((i+1) == VEC_SIZE(v));
    }
    assert(cap == VEC_CAPACITY(v));
    assert(n == VEC_SIZE(v));

    long *iter=VEC_BEGIN(v);
    long *end=VEC_END(v);
    i=0;
    while (iter != end) {
        assert(*iter == i);
        i++;
        iter++;
    }


    size_t newsize=10;
    VEC_RESIZE(v, newsize);
    assert(cap == VEC_CAPACITY(v));
    assert(newsize == VEC_SIZE(v));

    // different ways to set.  VEC_SET is the
    // same as VEC_GET but reads better when setting
    VEC_SET(v,3) = 12;
    assert(12 == VEC_GET(v,3));

    long *p = VEC_GETPTR(v,3);
    assert(12 == *p);

    long val = VEC_POP(v);
    assert((newsize-1) == val);
    assert((newsize-1) == VEC_SIZE(v));

    VEC_RESIZE(v, 3);
    assert(cap == VEC_CAPACITY(v));
    assert(3 == VEC_SIZE(v));


    VEC_CLEAR(v);
    assert(cap == VEC_CAPACITY(v));
    assert(0 == VEC_SIZE(v));
    VEC_DROP(v);
    assert(1 == VEC_CAPACITY(v));
    assert(0 == VEC_SIZE(v));

    VEC_FREE(v);
    assert(NULL == v);
}

void test_vector_of_vectors() {

    // vector of vectors.  Each is a reference type, so we must use VEC_NEW
    // to initialize. Also we have to destroy the data pointed at by these
    // before destroying the vector

    VEC(VEC(long)) v = VEC_NEW(VEC(long));;

    VEC_PUSH(v, VEC_NEW(long));
    VEC_PUSH(v, VEC_NEW(long));

    assert(2 == VEC_SIZE(v));
    assert(NULL != VEC_GET(v,0));
    assert(NULL != VEC_GET(v,1));

    VEC_RESIZE(v,3);
    VEC_SET(v,2) = VEC_NEW(long);
    assert(NULL != VEC_GET(v,2));

    // add data to one of the sub-vectors
    VEC(long) tmp = VEC_GET(v,0);
    VEC_PUSH(tmp, 3);

    assert(1 == VEC_SIZE(VEC_GET(v,0)) );
    long x = VEC_GETFRONT(VEC_GET(v,0));
    assert(x == 3);

    // special method to delete vectors of vectors
    VEC_VEC_FREE(v);
    assert(NULL==v);

}

void test_raii() {

    VEC_RAII(long, lvec) {
        VEC_RAII(MyStruct, msvec) {
            VEC_PUSH(lvec,3);
            VEC_PUSH(lvec,5);

            long t = VEC_POP(lvec);
            assert(t == 5);
            t = VEC_POP(lvec);
            assert(t == 3);
            assert(0==VEC_SIZE(lvec));

            MyStruct ms;
            ms.id = 3;
            ms.x = -3.;
            VEC_PUSH(msvec,ms);
            ms.id = 5;
            ms.x = -5.;
            VEC_PUSH(msvec,ms);

            MyStruct tms = VEC_POP(msvec);
            assert(tms.id == 5);
            tms = VEC_POP(msvec);
            assert(tms.id == 3);
            assert(0==VEC_SIZE(msvec));
        } // msvec cleanup
    } // lvec  cleanup
}

void test_reserve() {
    size_t n=10, cap=0;

    VEC(long) v = VEC_NEW(long);

    VEC_RESERVE(v, n);
    cap = VEC_CAPACITY(v);

    assert(0 == VEC_SIZE(v));
    assert(n <= cap);

    VEC_PUSH(v, 3);
    assert(1 == VEC_SIZE(v));
    assert(cap == VEC_CAPACITY(v));

    VEC_FREE(v);
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

    VEC(MyStruct_p) v = VEC_NEW(MyStruct_p);

    // note we never own the pointers in the vector! So we must allocat and
    // free them separately
    struct mystruct* tvec = calloc(n, sizeof(struct mystruct));

    for (i=0; i<n; i++) {
        struct mystruct *t = &tvec[i];
        t->id = i;
        t->x = 2*i;

        // this copies the pointer to t
        VEC_PUSH(v, t);
    }

    for (i=0; i<n; i++) {
        struct mystruct *t = VEC_GET(v, i);
        assert(t->id == i);
        assert(t->x == 2*i);
    }

    // iteration
    // note different pointer declarations
    i=0;
    MyStruct_p *iter = VEC_BEGIN(v);
    MyStruct **end  = VEC_END(v);
    while (iter != end) {
        assert((*iter)->id == i);
        iter++;
        i++;
    }
    VEC_FREE(v);

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
    wlog("testing not owned\n");
    test_not_owned();
    wlog("testing reserve\n");
    test_reserve();
    wlog("testing vector of vectors\n");
    test_vector_of_vectors();
    wlog("testing raii\n");
    test_raii();
}
