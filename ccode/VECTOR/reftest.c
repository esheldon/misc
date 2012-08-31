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

MyStruct *MyStruct_new()
{
    MyStruct *ms=NULL;
    ms = calloc(1, sizeof(MyStruct));
    ms->x = calloc(3,sizeof(double));
    return ms;
}

MyStruct *MyStruct_del(MyStruct *self)
{
    if (self) {
        free(self->x);
        free(self);
    }
    return NULL;
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
    ms=MyStruct_del(ms);

    RVECTOR_CLEAR(mv);
    assert(cap==mv->capacity);
    assert(0==mv->size);

    RVECTOR_DROP(mv);
    assert(1==mv->capacity);
    assert(0==mv->size);


    RVECTOR_DEL(mv);
    assert(NULL==mv);
}

int main(int argc, char** argv) {
    wlog("testing push/pop\n");
    test_pushpop();
}
