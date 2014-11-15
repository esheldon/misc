from __future__ import print_function
import os
import sys
import yaml

# type map for built-in C types.  User defined types will not use short names
# or have sorting/searching functions written
#
# format is just for the test program
#
# ctype is the actual C variable type name
# shortname is for the struct name and functions, e.g. 
#   dvector* vec = dvector_new();

def add_defaults(d, type):
    """
    defaults
        - shortname is type
        - not builtin
        - add entry for short name
    """

    d['type'] = type
    d['shortname']=d.get('shortname',type)
    d['is_builtin']=d.get('is_builtin',False)
    d[d['shortname']] = d


def read_builtins():
    with open('builtins.yaml') as fobj:
        typemap=yaml.load(fobj)

    types=list(typemap.keys())
    for type in types:
        t=typemap[type]
        t['is_builtin']=True

    for type in typemap:
        add_defaults(typemap[type], type)
    return typemap

def read_config(config_file):
    with open(config_file) as fobj:
        conf=yaml.load(fobj)

    bi=read_builtins()
    for type in conf:
        tconf = conf[type]

        if type in bi:
            # it is in the builtin, copy from there first
            conf[type] = {}
            conf[type].update(bi[type])
            if isinstance(tconf, dict):
                # if a dict, update the type map
                conf[type].update(tconf)
        else:
            # user defined type
            add_defaults(tconf, type)

    return conf

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
// usage: vector=%(shortname)svector_free(vec);
%(shortname)svector* %(shortname)svector_free(%(shortname)svector* self);

// push a new element onto the vector
// if reallocation is needed, size is increased by some factor
// unless size is zero, when a fixed amount are allocated
void %(shortname)svector_push(%(shortname)svector* self, %(type)s val);

// pop the last element and decrement size; no reallocation is performed
// if empty, an error message is printed and a zerod version of
// the type is returned
%(type)s %(shortname)svector_pop(%(shortname)svector* self);

// make a new copy of the vector
%(shortname)svector* %(shortname)svector_copy(%(shortname)svector* self);

// make a new vector with data copied from the input array
%(shortname)svector* %(shortname)svector_fromarray(%(type)s* data, size_t size);

// make a vector with the specified initial size, zeroed
%(shortname)svector* %(shortname)svector_zeros(size_t num);
'''

hformat_builtin='''
//
// these are only written builtins
//

// make a vector with the specified initial size, set to 1
%(shortname)svector* %(shortname)svector_ones(size_t num);

%(shortname)svector* %(shortname)svector_range(long min, long max);

int __%(shortname)svector_compare_el(const void *a, const void *b);
void %(shortname)svector_sort(%(shortname)svector* self);
%(type)s* %(shortname)svector_find(%(shortname)svector* self, %(type)s el);
'''

c_format='''
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
            memset(newdata + oldcap, 0, num_new_bytes);
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

%(shortname)svector* %(shortname)svector_free(%(shortname)svector* self) {
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


%(shortname)svector* %(shortname)svector_copy(%(shortname)svector* self) {
    %(shortname)svector* vcopy=%(shortname)svector_new();
    %(shortname)svector_resize(vcopy, self->size);

    if (self->size > 0) {
        memcpy(vcopy->data, self->data, self->size*sizeof(%(type)s));
    }

    return vcopy;
}

%(shortname)svector* %(shortname)svector_fromarray(%(type)s* data, size_t size) {
    %(shortname)svector* self=%(shortname)svector_new();
    %(shortname)svector_resize(self, size);

    if (self->size > 0) {
        memcpy(self->data, data, size*sizeof(%(type)s));
    }

    return self;
}

%(shortname)svector* %(shortname)svector_zeros(size_t num) {

    %(shortname)svector* self=%(shortname)svector_new();
    %(shortname)svector_resize(self, num);
    return self;
}

%(shortname)svector* %(shortname)svector_ones(size_t num) {

    %(shortname)svector* self=%(shortname)svector_new();
    for (size_t i=0; i<num; i++) {
        %(shortname)svector_push(self,1);
    }
    return self;
}
'''


c_format_builtin='''
%(shortname)svector* %(shortname)svector_range(long min, long max) {

    %(shortname)svector* self=%(shortname)svector_new();
    for (long i=min; i<max; i++) {
        %(shortname)svector_push(self,i);
    }
    
    return self;
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

tformat_builtin='''// This file was auto-generated
#include <stdio.h>
#include <stdlib.h>
#include "../vector.h"

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


    printf("making a copy\\n");
    %(shortname)svector* vcopy=%(shortname)svector_copy(vec);

    printf("making a copy from array\\n");
    %(shortname)svector* vcopy_arr=%(shortname)svector_fromarray(vec->data, vec->size);

    for (size_t i=0; i<vec->size; i++) {
        printf("    compare: %(format)s %(format)s %(format)s\\n",
               vec->data[i], vcopy->data[i], vcopy_arr->data[i]);
    }

    long min=3, max=10;
    printf("making range [%%ld,%%ld)\\n", min, max);

    %(shortname)svector* vrng=%(shortname)svector_range(min, max);
    for (size_t i=0; i<vrng->size; i++) {
        printf("    %%lu %(format)s\\n", i, vrng->data[i]);
    }

    size_t nzero=3;
    printf("making zeros[%%lu]\\n", nzero);
    %(shortname)svector* vzeros=%(shortname)svector_zeros(nzero);
    for (size_t i=0; i<nzero; i++) {
        printf("    %%lu %(format)s\\n", i, vzeros->data[i]);
    }

    size_t none=3;
    printf("making ones[%%lu]\\n", none);
    %(shortname)svector* vones=%(shortname)svector_ones(none);
    for (size_t i=0; i<none; i++) {
        printf("    %%lu %(format)s\\n", i, vones->data[i]);
    }



    printf("freeing vectors\\n");
    printf("freeing vec\\n");
    vec=%(shortname)svector_free(vec);
    printf("freeing vcopy\\n");
    vcopy=%(shortname)svector_free(vcopy);
    printf("freeing vcopy_arr\\n");
    vcopy_arr=%(shortname)svector_free(vcopy_arr);
    printf("freeing vrng\\n");
    vrng=%(shortname)svector_free(vrng);
    printf("freeing vzeros\\n");
    vzeros=%(shortname)svector_free(vzeros);
    printf("freeing vones\\n");
    vones=%(shortname)svector_free(vones);
}
'''

tformat_user='''// This file was auto-generated
// since the type is user defined, we don't know how to print it.
// so nothing will be printed!
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../vector.h"

int main(int argc, char** argv) {
    %(shortname)svector* vec = %(shortname)svector_new();

    %(type)s var;
    memset(&var, 0, sizeof(%(type)s));
    for (size_t i=0;i<75; i++) {
        %(shortname)svector_push(vec, var);
    }

    printf("size: %%ld\\n", vec->size);
    printf("capacity: %%ld\\n", vec->capacity);

    size_t newsize=25;
    printf("reallocating to size %%ld\\n", newsize);
    %(shortname)svector_realloc(vec, newsize);
    printf("size: %%ld\\n", vec->size);
    printf("capacity: %%ld\\n", vec->capacity);

    while (vec->size > 0) {
        var = %(shortname)svector_pop(vec);
    }

    printf("size: %%ld\\n", vec->size);
    printf("capacity: %%ld\\n", vec->capacity);

    printf("popping the now empty vector, should give an error message: \\n");
    %(shortname)svector_pop(vec);

    printf("making a copy\\n");
    %(shortname)svector* vcopy=%(shortname)svector_copy(vec);

    printf("making a copy from array\\n");
    %(shortname)svector* vcopy_arr=%(shortname)svector_fromarray(vec->data, vec->size);


    vec=%(shortname)svector_free(vec);
    vcopy=%(shortname)svector_free(vcopy);
    vcopy_arr=%(shortname)svector_free(vcopy_arr);
}
'''

header_head="""// This header was auto-generated
#ifndef _VECTOR_H
#define _VECTOR_H
#include <stdint.h>

#define VECTOR_INITSIZE 1
#define VECTOR_PUSH_REALLOC_MULTVAL 2
"""

header_foot="""
#endif
"""

c_head="""// This file was auto-generated

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <float.h>
#include "vector.h"
"""

class Generator(dict):
    def __init__(self, config):
        self.update(config)

    def write(self):
        """
        write all
        """

        self.write_header()
        self.write_c()
        self.write_tests()

    def write_header(self):
        """
        write the header
        """
        print("writing vector.h")
        with open("vector.h",'w') as fobj:

            fobj.write(header_head)

            for type in self:
                ti = self[type]

                text = hformat % ti
                fobj.write(text)

                if ti['is_builtin']:
                    text = hformat_builtin % ti
                    fobj.write(text)

            fobj.write(header_foot)

    def write_c(self):
        print("writing vector.c")
        with open('vector.c','w') as fobj:
            fobj.write(c_head)

            for type in self:
                ti=self[type]

                text = c_format % ti
                fobj.write(text)
                
                if ti['is_builtin']:
                    text = c_format_builtin % ti
                    fobj.write(text)

    def write_tests(self):
        print("writing tests")
        outdir="tests"
        if not os.path.exists(outdir):
            print("making dir:",outdir)
            os.makedirs(outdir)

        for type in self:
            ti=self[type]

            cname = 'test-%(shortname)svector.c' % ti
            cname = os.path.join(outdir, cname)
            
            print("    writing:",cname)
            with open(cname,'w') as fobj:
                if ti['is_builtin']:
                    text = tformat_builtin % ti
                else:
                    text = tformat_user % ti

                fobj.write(text)
                fobj.close()


