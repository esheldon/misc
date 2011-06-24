#include "sort.h"
#include "Vector.h"

//int i64compare(const void* a, const void* b) {
//    return ( *(int64_t*)a - *(int64_t*)b );
//}

void szswap(size_t* a, size_t* b) {
    size_t tmp;
    tmp = *b;
    *b = *a;
    *a = tmp;
}

// quicksort but return indices instead of sorting in place[:w
struct szvector* i64sortind(const struct i64vector* vec) {

    struct szvector* sind = szvector_range(vec->size);

    size_t left=0;
    size_t right=vec->size-1;

    i64sortind_recurse(vec->data, sind->data, left, right);

    return sind;
}

// Do NOT call this directly; use i64sortind
void i64sortind_recurse(const int64_t* arr,
                        size_t* sind,
                        size_t left,
                        size_t right) {

    size_t pivot, leftidx, rightidx;

    leftidx=left;
    rightidx=right;

    if ( (right-left+1) > 1) {
        pivot = (left+right)/2;

        while ( (leftidx <= pivot) && (rightidx >= pivot) ) {

            while ( (arr[sind[leftidx]] < arr[sind[pivot]]) && (leftidx <= pivot) ) {
                leftidx = leftidx + 1;
            }
            while ( (arr[sind[rightidx]] > arr[sind[pivot]]) && (rightidx >= pivot)) {
                rightidx = rightidx - 1;
            }

            szswap(&sind[leftidx], &sind[rightidx]);

            leftidx = leftidx+1;
            rightidx = rightidx-1;

            if (leftidx-1 == pivot) {
                rightidx = rightidx + 1;
                pivot = rightidx;
            } else if (rightidx+1 == pivot) {
                leftidx = leftidx-1;
                pivot = leftidx;
            }
        }

        i64sortind_recurse(arr, sind, left, pivot-1);
        i64sortind_recurse(arr, sind, pivot+1, right);

    }

}
