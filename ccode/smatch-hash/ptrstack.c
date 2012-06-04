#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <float.h>
#include "ptrstack.h"


struct ptrstack* ptrstack_new(size_t num) {
    struct ptrstack* stack = calloc(1,sizeof(struct ptrstack));
    if (stack == NULL) {
        fprintf(stderr,"Could not allocate struct ptrstack\n");
        return NULL;
    }

    stack->size = 0;
    stack->allocated_size = num;
    stack->push_realloc_style = PTRSTACK_PUSH_REALLOC_MULT;
    stack->push_initsize = PTRSTACK_PUSH_INITSIZE;
    stack->realloc_multval = PTRSTACK_PUSH_REALLOC_MULTVAL;

    if (num == 0) {
        stack->data = NULL;
    } else {
        stack->data = calloc(num, sizeof(void*));
        if (stack->data == NULL) {
            free(stack);
            fprintf(stderr,"Could not allocate data for stack\n");
            return NULL;
        }
    }

    return stack;
}

void ptrstack_realloc(struct ptrstack* stack, size_t newsize) {

    size_t oldsize = stack->allocated_size;
    if (newsize != oldsize) {
        size_t elsize = sizeof(void*);

        void** newdata = realloc(stack->data, newsize*elsize);
        if (newdata == NULL) {
            fprintf(stderr,"failed to reallocate\n");
            return;
        }

        if (newsize > stack->allocated_size) {
            // the allocated size is larger.  make sure to initialize the new
            // memory region.  This is the area starting from index [oldsize]
            size_t num_new_bytes = (newsize-oldsize)*elsize;
            memset(&newdata[oldsize], 0, num_new_bytes);
        } else if (stack->size > newsize) {
            // The viewed size is larger than the allocated size in this case,
            // we must set the size to the maximum it can be, which is the
            // allocated size
            stack->size = newsize;
        }

        stack->data = newdata;
        stack->allocated_size = newsize;
    }

}
void ptrstack_resize(struct ptrstack* stack, size_t newsize) {
   if (newsize > stack->allocated_size) {
       ptrstack_realloc(stack, newsize);
   }

   stack->size = newsize;
}

void ptrstack_clear(struct ptrstack* stack) {
    stack->size=0;
    stack->allocated_size=0;
    free(stack->data);
    stack->data=NULL;
}

struct ptrstack* ptrstack_delete(struct ptrstack* stack) {
    if (stack != NULL) {
        ptrstack_clear(stack);
        free(stack);
    }
    return NULL;
}

void ptrstack_push(struct ptrstack* stack, void* val) {
    // see if we have already filled the available data vector
    // if so, reallocate to larger storage
    if (stack->size == stack->allocated_size) {

        size_t newsize;
        if (stack->allocated_size == 0) {
            newsize=stack->push_initsize;
        } else {
            // currenly we always use the multiplier reallocation  method.
            if (stack->push_realloc_style != PTRSTACK_PUSH_REALLOC_MULT) {
                fprintf(stderr,"Currently only support push realloc style PTRSTACK_PUSH_REALLOC_MULT\n");
                exit(EXIT_FAILURE);
            }
            // this will "floor" the size
            newsize = (size_t)(stack->allocated_size*stack->realloc_multval);
            // we want ceiling
            newsize++;
        }

        ptrstack_realloc(stack, newsize);

    }

    stack->size++;
    stack->data[stack->size-1] = val;
}

void* ptrstack_pop(struct ptrstack* stack) {
    if (stack->size == 0) {
        return NULL;
    }

    void* val=stack->data[stack->size-1];
    stack->size--;
    return val;
        
}
