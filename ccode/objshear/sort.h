#ifndef _SORTLIB_H
#define _SORTLIB_H

#include "Vector.h"

struct szvector* i64sortind(const struct i64vector* vec);

void szswap(size_t* a, size_t* b);

// don't call directly
void i64sortind_recurse(const int64_t* arr,
                        size_t* sind,
                        size_t left,
                        size_t right);

#endif
