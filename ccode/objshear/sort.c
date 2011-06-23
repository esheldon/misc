#include "sort.h"
#include "Vector.h"
#include "defs.h"

//int i64compare(const void* a, const void* b) {
//    return ( *(int64*)a - *(int64*)b );
//}

// quicksort but return indices instead of sorting in place[:w
struct i64vector* i64qsortind(struct i64vector* vec) {
    struct i64vector* sind = i64vector_range(vec->size);

    int64 left=0;
    int64 right=vec->size-1;

    i64qsortind_recurse(vec, sind, left, right);
}

void i64qsortind_recurse(struct i64vector* vec,
                         struct i64vector* sind,
                         int64 left,
                         int64 right) {

    int64 pivot, leftidx, rightidx;

    leftidx=left;
    rightidx=right;


}
