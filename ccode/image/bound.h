#ifndef _BOUND_HEADER_GUARD_H
#define _BOUND_HEADER_GUARD_H

#include <stdlib.h>

// note bounds can have negative indices; the image code
// should deal with it properly
struct bound {
    ssize_t rowmin;
    ssize_t rowmax;
    ssize_t colmin;
    ssize_t colmax;
};


struct bound *bound_new(ssize_t rowmin, 
                        ssize_t rowmax, 
                        ssize_t colmin, 
                        ssize_t colmax);
struct bound *bound_free(struct bound *self);

void bound_set(struct bound* self,
               ssize_t rowmin, 
               ssize_t rowmax, 
               ssize_t colmin, 
               ssize_t colmax);

void bound_print(const struct bound *bound, FILE *stream);

#endif
