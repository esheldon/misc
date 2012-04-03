#ifndef _MATCH_H
#define _MATCH_H

#include <stdint.h>

#define MATCHSTACK_PUSH_REALLOC_MULTVAL 2
#define MATCHSTACK_PUSH_INITSIZE 1

struct match {
    size_t index;
    double cosdist;
};

struct matchstack {

    size_t size;        // number of elements that are visible to the user
    size_t capacity;    // number of allocated elements in data vector
    struct match* data;

};

struct matchstack* matchstack_new(void);
void matchstack_realloc(struct matchstack* ms, size_t newsize);
void matchstack_resize(struct matchstack* ms, size_t newsize);
void matchstack_push(struct matchstack* ms, size_t index, double cosdist);

int match_compare(const void *a, const void *b);
void matchstack_sort(struct matchstack* ms);
struct matchstack* matchstack_delete(struct matchstack* ms);
#endif
