#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "match.h"


struct matchstack* matchstack_new(void) {
    struct matchstack* ms=NULL;

    ms=calloc(1,sizeof(struct matchstack));
    if (ms==NULL) {
        fprintf(stderr,"Error allocating matchstack\n");
        return NULL;
    }
    return ms;
}


void matchstack_realloc(struct matchstack* ms, size_t newsize) {

    size_t oldsize = ms->capacity;
    if (newsize != oldsize) {
        struct match* newdata=NULL;
        size_t elsize = sizeof(struct match);

        newdata = realloc(ms->data, newsize*elsize);
        if (newdata == NULL) {
            fprintf(stderr,"failed to reallocate\n");
            return;
        }

        if (newsize > ms->capacity) {
            // the allocated size is larger.  make sure to initialize the new
            // memory region.  This is the area starting from index [oldsize]
            size_t num_new_bytes = (newsize-oldsize)*elsize;
            memset(&newdata[oldsize], 0, num_new_bytes);
        } else if (ms->size > newsize) {
            // The viewed size is larger than the allocated size in this case,
            // we must set the size to the maximum it can be, which is the
            // allocated size
            ms->size = newsize;
        }

        ms->data = newdata;
        ms->capacity = newsize;
    }

}

void matchstack_resize(struct matchstack* ms, size_t newsize) {
   if (newsize > ms->capacity) {
       matchstack_realloc(ms, newsize);
   }

   ms->size = newsize;
}

void matchstack_push(struct matchstack* ms, size_t index, double cosdist) {
    // see if we have already filled the available data vector
    // if so, reallocate to larger storage
    struct match* m;
    if (ms->size == ms->capacity) {

        size_t newsize;
        if (ms->capacity == 0) {
            newsize=MATCHSTACK_PUSH_INITSIZE;
        } else {
            // this will "floor" the size
            newsize = (size_t)(ms->capacity*MATCHSTACK_PUSH_REALLOC_MULTVAL);
            // we want ceiling
            newsize++;
        }

        matchstack_realloc(ms, newsize);

    }

    ms->size++;

    m = &ms->data[ms->size-1];
    m->index = index;
    m->cosdist = cosdist;

}


int match_compare(const void *a, const void *b) {
    // we want to sort largest first, so will
    // reverse the normal trend
    double temp = 
        ((struct match*)b)->cosdist
         -
        ((struct match*)a)->cosdist;
    if (temp > 0)
        return 1;
    else if (temp < 0)
        return -1;
    else
        return 0;
}

void matchstack_sort(struct matchstack* ms) {
    qsort(ms->data, ms->size, sizeof(struct match), match_compare);
}


struct matchstack* matchstack_delete(struct matchstack* ms) {
    if (ms != NULL) {
        free(ms->data);
        free(ms);
    }

    return NULL;
}
