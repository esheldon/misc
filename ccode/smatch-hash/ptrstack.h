#ifndef _PTRSTACK_H
#define _PTRSTACK_H
#include <stdint.h>

#define PTRSTACK_PUSH_REALLOC_MULT 1
#define PTRSTACK_PUSH_REALLOC_MULTVAL 2
#define PTRSTACK_PUSH_INITSIZE 1

struct ptrstack {
    size_t size;            // number of elements that are visible to the user
    size_t allocated_size;  // number of allocated elements in data vector
    size_t push_realloc_style; // Currently always STACK_PUSH_REALLOC_MULT, 
                               // which is reallocate to allocated_size*realloc_multval
    size_t push_initsize;      // default size on first push, default STACK_PUSH_INITSIZE 
    double realloc_multval; // when allocated size is exceeded while pushing, 
                            // reallocate to allocated_size*realloc_multval, default 
                            // STACK_PUSH_REALLOC_MULTVAL
                            // if allocated_size was zero, we allocate to push_initsize
    void** data;
};

struct ptrstack* ptrstack_new(size_t num);

// if size > allocated size, then a reallocation occurs
// if size <= internal size, then only the ->size field is reset
// use ptrstack_realloc() to reallocate the data vector and set the ->size
void ptrstack_resize(struct ptrstack* stack, size_t newsize);

// perform reallocation on the underlying data vector. This does
// not change the size field unless the new size is smaller
// than the viewed size
void ptrstack_realloc(struct ptrstack* stack, size_t newsize);

// completely clears memory in the data vector
void ptrstack_clear(struct ptrstack* stack);

// clears all memory and sets pointer to NULL
// usage: stack=ptrstack_delete(stack);
struct ptrstack* ptrstack_delete(struct ptrstack* stack);

// if reallocation is needed, size is increased by 50 percent
// unless size is zero, when it 100 are allocated
void ptrstack_push(struct ptrstack* stack, void* val);
// pop the last element and decrement size; no reallocation is performed
// if empty, INT64_MIN is returned
void* ptrstack_pop(struct ptrstack* stack);

#endif  // header guard
