#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "RVECTOR.h"


#define wlog(...) fprintf(stderr, __VA_ARGS__)

typedef struct mystruct {
    int id;
    double *x;
} MyStruct;

RVECTOR_DEF(MyStruct);
RVECTOR_DEF(long);

MyStruct *MyStruct_new()
{
    MyStruct *ms=NULL;
    ms = calloc(1, sizeof(MyStruct));
    ms->x = calloc(3,sizeof(double));
    return ms;
}

void MyStruct_del(MyStruct *self)
{
    if (self) {
        fprintf(stderr,"    freeing x\n");
        free(self->x);
        fprintf(stderr,"    freeing self\n");
        free(self);
    }
}

void test_pushpop()
{
    MyStruct *ms=NULL;
    size_t cap=0;

    RVECTOR(MyStruct) mv = RVECTOR_NEW(MyStruct,MyStruct_del);
    assert(0==mv->size);

    ms=MyStruct_new();
    ms->x[0] = 5; ms->x[1] = 10; ms->x[2] = 15; 
    RVECTOR_PUSH(mv,ms);
    assert(1==mv->size);

    ms=MyStruct_new();
    ms->x[0] = 50; ms->x[1] = 100; ms->x[2] = 150; 
    RVECTOR_PUSH(mv,ms);
    assert(2==mv->size);

    ms=RVECTOR_GET(mv,0);
    assert(5==ms->x[0]);
    assert(10==ms->x[1]);
    assert(15==ms->x[2]);

    ms=RVECTOR_GET(mv,1);
    assert(50==ms->x[0]);
    assert(100==ms->x[1]);
    assert(150==ms->x[2]);

    cap=RVECTOR_CAPACITY(mv);

    // ownership transferred to ms
    ms=RVECTOR_POP(mv);
    assert(1==mv->size);

    // ms owns this now, must free it
    MyStruct_del(ms); ms=NULL;

    RVECTOR_CLEAR(mv);
    assert(cap==mv->capacity);
    assert(0==mv->size);

    RVECTOR_DROP(mv);
    assert(1==mv->capacity);
    assert(0==mv->size);


    RVECTOR_DEL(mv);
    assert(NULL==mv);
}

void test_lonarr() {
    size_t i=0, n=3, m=10;
    long *larr=NULL, *lptr=NULL;

    RVECTOR(long) v = RVECTOR_NEW(long,free);

    larr=calloc(n,sizeof(long));
    for (i=0; i<n; i++) {
        larr[i] = i+1;
    }

    // ownership passed
    RVECTOR_PUSH(v, larr);
    assert(1 == RVECTOR_SIZE(v));

    larr=calloc(m,sizeof(long));
    for (i=0; i<m; i++) {
        larr[i] = i+1;
    }

    // ownership passed
    RVECTOR_PUSH(v, larr);
    assert(2 == RVECTOR_SIZE(v));


    lptr=RVECTOR_GET(v, 0);
    for (i=0; i<n; i++) {
        assert((i+1) == lptr[i]);
    }

    lptr=RVECTOR_GET(v, 1);
    for (i=0; i<m; i++) {
        assert((i+1) == lptr[i]);
    }


    RVECTOR_DEL(v);
    assert(NULL == v);
}

void test_reserve() {
    size_t n=10, cap=0;
    long *larr=NULL;

    RVECTOR(long) v = RVECTOR_NEW(long,free);

    RVECTOR_RESERVE(v, n);
    cap = RVECTOR_CAPACITY(v);

    assert(0 == RVECTOR_SIZE(v));
    assert(n <= cap);

    larr=calloc(3,sizeof(long));

    RVECTOR_PUSH(v, larr);
    assert(1 == RVECTOR_SIZE(v));
    assert(cap == RVECTOR_CAPACITY(v));

    RVECTOR_DEL(v);
    assert(NULL == v);
}
int main(int argc, char** argv) {
    wlog("testing long arrays\n");
    test_lonarr();
    wlog("testing push/pop\n");
    test_pushpop();
    wlog("testing reserve\n");
    test_reserve();
}
