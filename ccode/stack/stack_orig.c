#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "stack.h"

struct pixlist* i64stack_new(size_t num) {
    struct pixlist* plist = malloc(sizeof(struct pixlist));
    if (plist == NULL) {
        printf("Could not allocate struct pixlist\n");
        exit(EXIT_FAILURE);
    }

    plist->size = 0;
    plist->allocated_size = num;
    plist->realloc_mult = 1.5;

    plist->data = calloc(num, sizeof(int64_t));
    if (plist->data == NULL) {
        printf("Could not allocate data in pixlist\n");
        exit(EXIT_FAILURE);
    }

    return plist;
}

void i64stack_realloc(struct pixlist* plist, size_t newsize) {
    if (newsize != plist->allocated_size) {
        int64_t* newdata = realloc(plist->data, newsize*sizeof(int64_t));
        if (newdata == NULL) {
            printf("failed to reallocate\n");
            exit(EXIT_FAILURE);
        }

        if (newsize > plist->allocated_size) {
            // make sure to initialize the new memory regions
            memset(&newdata[plist->size], 0, (newsize-plist->allocated_size)*sizeof(int64_t));
        } else {
            // in this case, we must adjust the viewed size also
            plist->size = newsize;
        }

        plist->data = newdata;
        plist->allocated_size = newsize;
    }

}
void i64stack_resize(struct pixlist* plist, size_t newsize) {
   if (newsize > plist->allocated_size) {
       i64stack_realloc(plist, newsize);
   }

   plist->size = newsize;
}

void i64stack_clear(struct pixlist* plist) {
    plist->size=0;
    plist->allocated_size=0;
    free(plist->data);
    plist->data=NULL;
}

void i64stack_delete(struct pixlist* plist) {
    if (plist != NULL) {
        i64stack_clear(plist);
        free(plist);
        plist=NULL;
    }
}

void i64stack_push(struct pixlist* plist, int64_t pixnum) {
    // see if we have already filled the available data vector
    // if so, reallocate to "mult" times existing storage
    if (plist->size == plist->allocated_size) {

        size_t newsize;
        if (plist->allocated_size == 0) {
            newsize=100;
        } else {
            newsize = (int64_t)(plist->allocated_size*plist->realloc_mult);
            newsize += 1;
        }

        i64stack_realloc(plist, newsize);

    }

    plist->size += 1;
    plist->data[plist->size-1] = pixnum;
}

int64_t i64stack_pop(struct pixlist* plist) {
    if (plist->size == 0) {
        return INT64_MAX;
    }

    int64_t val = plist->data[plist->size-1];
    plist->size -= 1;
    return val;
        
}
