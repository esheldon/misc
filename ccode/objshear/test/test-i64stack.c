// This header was auto-generated
#include <stdio.h>
#include <stdlib.h>
#include "../stack.h"

int main(int argc, char** argv) {
    struct i64stack* stack = i64stack_new(0);

    for (size_t i=0;i<75; i++) {
        printf("push: %ld\n", (int64_t)i);
        i64stack_push(stack, i);
    }

    printf("size: %ld\n", stack->size);
    printf("allocated size: %ld\n", stack->allocated_size);

    size_t newsize=25;
    printf("reallocating to size %ld\n", newsize);
    i64stack_realloc(stack, newsize);
    printf("size: %ld\n", stack->size);
    printf("allocated size: %ld\n", stack->allocated_size);

    while (stack->size > 0) {
        printf("pop: %ld\n", i64stack_pop(stack));
    }

    printf("size: %ld\n", stack->size);
    printf("allocated size: %ld\n", stack->allocated_size);

    printf("popping the now empty list: \n    %ld\n", i64stack_pop(stack));

}
