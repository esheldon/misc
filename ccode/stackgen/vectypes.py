import os
import sys

# type map for built-in C types.  User defined types will not use short names
# or have sorting/searching functions written
#
# format is just for the test program
#
# ctype is the actual C variable type name
# shortname is for the struct name and functions, e.g. 
#   dvector* vec = dvector_new();

typemap={}
typemap['float']   = {'ctype':'float',    'shortname':'f',  'sortype':'float',   'format':'%f'}
typemap['double']  = {'ctype':'double',   'shortname':'d',  'sortype':'double',  'format':'%lf'}
typemap['int8']    = {'ctype':'int8_t',   'shortname':'b',  'sortype':'int32_t', 'format':'%d'}
typemap['uint8']   = {'ctype':'uint8_t',  'shortname':'ub', 'sortype':'int32_t', 'format':'%u'}
typemap['int16']   = {'ctype':'int16_t',  'shortname':'s',  'sortype':'int32_t', 'format':'%d'}
typemap['uint16']  = {'ctype':'uint16_t', 'shortname':'us', 'sortype':'int32_t', 'format':'%u'}
typemap['int32']   = {'ctype':'int32_t',  'shortname':'i',  'sortype':'int64_t', 'format':'%d'}
typemap['uint32']  = {'ctype':'uint32_t', 'shortname':'u',  'sortype':'int64_t', 'format':'%u'}
typemap['int64']   = {'ctype':'int64_t',  'shortname':'l',  'sortype':'int64_t', 'format':'%ld'}
typemap['uint64']  = {'ctype':'uint64_t', 'shortname':'ul', 'sortype':'int64_t', 'format':'%lu'}
typemap['char']    = {'ctype':'char',     'shortname':'c',  'sortype':'int32_t', 'format':'%c'}

typemap['uchar']   = {'ctype':'unsigned char', 'shortname': 'uc',  'sortype':'int32_t',  'format':'%c'}

typemap['size']    = {'ctype':'size_t', 'shortname':'sz', 'sortype':'int64_t', 'format':'%lu'}

keys=list(typemap.keys())
for k in keys:
    t=typemap[k]
    typemap[t['shortname']] = t

hformat='''
struct %(shortname)svector {
    size_t size;            // number of elements that are visible to the user
    size_t capacity;        // number of allocated elements in data vector
    size_t initsize;        // default size on creation, default VECTOR_INITSIZE 
    double realloc_multval; // when capacity is exceeded while pushing, 
                            // reallocate to capacity*realloc_multval,
                            // default VECTOR_PUSH_REALLOC_MULTVAL
                            // if capacity was zero, we allocate to initsize
    %(type)s* data;
};

typedef struct %(shortname)svector %(shortname)svector;

%(shortname)svector* %(shortname)svector_new();

// if size > capacity, then a reallocation occurs
// if size <= capacity, then only the ->size field is reset
// use %(shortname)svector_realloc() to reallocate the data vector and set the ->size
void %(shortname)svector_resize(%(shortname)svector* self, size_t newsize);

// reserve at least the specified amount of slots.  If the new capacity is
// smaller than the current capacity, nothing happens.  If larger, a
// reallocation occurs.  No change to current contents occurs.
//
// currently, the exact requested amount is used but in the future we can
// optimize to page boundaries.

void %(shortname)svector_reserve(%(shortname)svector* self, size_t newcap);

// perform reallocation on the underlying data vector. This does
// not change the size field unless the new size is smaller
// than the viewed size
void %(shortname)svector_realloc(%(shortname)svector* self, size_t newsize);

// completely clears memory in the data vector
void %(shortname)svector_clear(%(shortname)svector* self);

// clears all memory and sets pointer to NULL
// usage: vector=%(shortname)svector_delete(vec);
%(shortname)svector* %(shortname)svector_delete(%(shortname)svector* self);

// push a new element onto the vector
// if reallocation is needed, size is increased by some factor
// unless size is zero, when a fixed amount are allocated
void %(shortname)svector_push(%(shortname)svector* self, %(type)s val);

// pop the last element and decrement size; no reallocation is performed
// if empty, an error message is printed and a zerod version of
// the type is returned
%(type)s %(shortname)svector_pop(%(shortname)svector* self);

// when we allow user defined types, these should not be
// written
int __%(shortname)svector_compare_el(const void *a, const void *b);
void %(shortname)svector_sort(%(shortname)svector* self);
%(type)s* %(shortname)svector_find(%(shortname)svector* self, %(type)s el);
'''

fformat='''
%(shortname)svector* %(shortname)svector_new() {
    %(shortname)svector* self = calloc(1,sizeof(%(shortname)svector));
    if (self == NULL) {
        fprintf(stderr,"Could not allocate %(shortname)svector\\n");
        return NULL;
    }

    self->capacity        = VECTOR_INITSIZE;
    self->initsize        = VECTOR_INITSIZE;
    self->realloc_multval = VECTOR_PUSH_REALLOC_MULTVAL;

    self->data = calloc(self->initsize, sizeof(%(type)s));
    if (self->data == NULL) {
        fprintf(stderr,"Could not allocate data for vector\\n");
        exit(1);
    }

    return self;
}

void %(shortname)svector_realloc(%(shortname)svector* self, size_t newcap) {

    size_t oldcap = self->capacity;
    if (newcap != oldcap) {
        size_t elsize = sizeof(%(type)s);

        %(type)s* newdata = realloc(self->data, newcap*elsize);
        if (newdata == NULL) {
            fprintf(stderr,"failed to reallocate\\n");
            return;
        }

        if (newcap > self->capacity) {
            // the capacity is larger.  Make sure to initialize the new
            // memory region.  This is the area starting from index [oldcap]
            size_t num_new_bytes = (newcap-oldcap)*elsize;
            memset(&newdata[oldcap], 0, num_new_bytes);
        } else if (self->size > newcap) {
            // The viewed size is larger than the capacity in this case,
            // we must set the size to the maximum it can be, which is the
            // capacity
            self->size = newcap;
        }

        self->data = newdata;
        self->capacity = newcap;
    }

}

void %(shortname)svector_resize(%(shortname)svector* self, size_t newsize) {
   if (newsize > self->capacity) {
       %(shortname)svector_realloc(self, newsize);
   }

   self->size = newsize;
}

void %(shortname)svector_reserve(%(shortname)svector* self, size_t newcap) {
   if (newcap > self->capacity) {
       %(shortname)svector_realloc(self, newcap);
   }
}

void %(shortname)svector_clear(%(shortname)svector* self) {
    self->size=0;
    self->capacity=0;
    free(self->data);
    self->data=NULL;
}

%(shortname)svector* %(shortname)svector_delete(%(shortname)svector* self) {
    if (self != NULL) {
        %(shortname)svector_clear(self);
        free(self);
    }
    return NULL;
}

void %(shortname)svector_push(%(shortname)svector* self, %(type)s val) {
    // see if we have already filled the available data vector
    // if so, reallocate to larger storage
    if (self->size == self->capacity) {

        size_t newsize=0;
        if (self->capacity == 0) {
            newsize=self->initsize;
        } else {
            // this will "floor" the size
            newsize = (size_t)(self->capacity*self->realloc_multval);
            // we want ceiling
            newsize++;
        }

        %(shortname)svector_realloc(self, newsize);

    }

    self->size++;
    self->data[self->size-1] = val;
}

%(type)s %(shortname)svector_pop(%(shortname)svector* self) {
    %(type)s val;
    if (self->size == 0) {
        fprintf(stderr,"attempt to pop from empty vector, returning zerod value\\n");
        memset(&val, 0, sizeof(%(type)s));
        return val;
    }

    val=self->data[self->size-1];
    self->size--;
    return val;
}

int __%(shortname)svector_compare_el(const void *a, const void *b) {
    %(sortype)s temp = 
        (  (%(sortype)s) *( (%(type)s*)a ) ) 
         -
        (  (%(sortype)s) *( (%(type)s*)b ) );
    if (temp > 0)
        return 1;
    else if (temp < 0)
        return -1;
    else
        return 0;
}


void %(shortname)svector_sort(%(shortname)svector* self) {
    qsort(self->data, self->size, sizeof(%(type)s), __%(shortname)svector_compare_el);
}
%(type)s* %(shortname)svector_find(%(shortname)svector* self, %(type)s el) {
    return (%(type)s*) bsearch(&el, self->data, self->size, sizeof(%(type)s), __%(shortname)svector_compare_el);
}
'''

tformat='''// This file was auto-generated
#include <stdio.h>
#include <stdlib.h>
#include "vector.h"

int main(int argc, char** argv) {
    %(shortname)svector* vec = %(shortname)svector_new();

    for (size_t i=0;i<75; i++) {
        printf("push: %(format)s\\n", (%(type)s)i);
        %(shortname)svector_push(vec, i);
    }

    printf("size: %%ld\\n", vec->size);
    printf("capacity: %%ld\\n", vec->capacity);

    size_t newsize=25;
    printf("reallocating to size %%ld\\n", newsize);
    %(shortname)svector_realloc(vec, newsize);
    printf("size: %%ld\\n", vec->size);
    printf("capacity: %%ld\\n", vec->capacity);

    while (vec->size > 0) {
        printf("pop: %(format)s\\n", %(shortname)svector_pop(vec));
    }

    printf("size: %%ld\\n", vec->size);
    printf("capacity: %%ld\\n", vec->capacity);

    printf("popping the now empty vector, should give zero and an error message: \\n");
    printf("    %(format)s\\n", %(shortname)svector_pop(vec));


    for (size_t i=0;i<10; i++) {
        %(shortname)svector_push(vec, i);
    }
    printf("putting unordered elements\\n");
    vec->data[3] = 88;
    vec->data[5] = 25;
    vec->data[9] = 1.3;
    printf("sorting\\n");
    %(shortname)svector_sort(vec);
    for (size_t i=0; i<vec->size; i++) {
        printf("    vec[%%ld]: %(format)s\\n", i, vec->data[i]);
    }

    printf("finding elements\\n");
    %(type)s vals[]={88,7,100};
    %(type)s* elptr=NULL;
    for (size_t i=0; i<3; i++) {
        elptr=%(shortname)svector_find(vec, vals[i]);
        if (elptr == NULL) {
            printf("Did not find %(format)s\\n", vals[i]);
        } else {
            printf("Found value %(format)s: %(format)s\\n", vals[i], *elptr);
        }
    }

    %(shortname)svector_delete(vec);
}
'''

def generate_h(types):
    fobj=open('vector.h','w')
    head="""// This header was auto-generated
#ifndef _VECTOR_H
#define _VECTOR_H
#include <stdint.h>

#define VECTOR_INITSIZE 1
#define VECTOR_PUSH_REALLOC_MULTVAL 2
"""

    fobj.write(head)

    for type in types:
        if type not in typemap:
            raise ValueError("type not supported: %s" % type)
        text = hformat % {'type':typemap[type]['ctype'],
                          'shortname':typemap[type]['shortname']}
        fobj.write(text)

    fobj.write('\n#endif  // header guard\n')
    fobj.close()

def generate_c(types):
    fobj=open('vector.c','w')
    head="""// This file was auto-generated

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <float.h>
#include "vector.h"

"""
    fobj.write(head)

    for type in types:
        if type not in typemap:
            raise ValueError("type not supported: %s" % type)
        text = fformat % {'type':typemap[type]['ctype'],
                          'shortname':typemap[type]['shortname'], 
                          'sortype':typemap[type]['sortype']}
        fobj.write(text)

    fobj.close()

def generate_tests(types):
    '''
    Files associated with tests not in the type list are removed.
    '''

    for type in typemap:
        sname=typemap[type]['shortname']

        front = 'test-%svector' % sname
        
        if type not in types and sname not in types:
            for ext in ['.c','.o','']:
                tname = front+ext
                if os.path.exists(tname):
                    os.remove(tname)
        else:
            cname = front+'.c'
            fobj=open(cname,'w')
            text = tformat % {'shortname':sname,
                              'format':typemap[type]['format'],
                              'type':typemap[type]['ctype']}
            fobj.write(text)
            fobj.close()

