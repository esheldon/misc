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
    return NULL;
}
MyStruct *MyStruct_del()
{
    return NULL;
}


void test_declare()
{
    RVECTOR(MyStruct) ms = RVECTOR_NEW(MyStruct,MyStruct_del);
    printf("%lu\n", ms->size);
    RVECTOR_DEL(ms);
    assert(NULL==ms);
}

int main(int argc, char** argv) {
    wlog("testing declare\n");
    test_declare();
}
