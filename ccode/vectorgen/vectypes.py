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

header_head="""// This header was auto-generated using vectorgen
#ifndef _VECTORGEN_H
#define _VECTORGEN_H
#include <stdint.h>
#include <string.h>

#define VECTOR_INITSIZE 1
#define VECTOR_PUSH_REALLOC_MULTVAL 2

// get properties, generic macros
#define vector_size(vec) (vec)->size
#define vector_capacity(vec) (vec)->capacity

// getters and setters, generic macros
// unsafe; maybe make safe?
#define vector_get(vec, i) (vec)->data[i]

#define vector_set(vec, i, val) do {                                         \\
    (vec)->data[(i)] = (val);                                                \\
} while(0)

// pointer to beginning
#define vector_begin(vec) (vec)->data

// pointer past end, don't dereference, just use for stopping iteration
#define vector_end(vec) (vec)->data + (vec)->size

// generic iteration over elements.  The iter name is a pointer
// sadly only works for -std=gnu99
//
// vector_foreach(iter, vec) {
//     printf("val is: %%d\\n", *iter);
// }

#define vector_foreach(itername, vec)                                        \\
    for(typeof((vec)->data) (itername)=vector_begin(vec),                    \\
        _iter_end_##itername=vector_end((vec));                              \\
        (itername) != _iter_end_##itername;                                  \\
        (itername)++)

// frees vec and its data, sets vec==NULL
#define vector_free(vec) do {                                                \\
    if ((vec)) {                                                             \\
        free((vec)->data);                                                   \\
        free((vec));                                                         \\
        (vec)=NULL;                                                          \\
    }                                                                        \\
} while(0)


// perform reallocation on the underlying data vector. This does
// not change the size field unless the new size is smaller
// than the viewed size
//
// note the underlying data will not go below capacity 1
// but if newcap==0 then size will be set to 0

#define vector_realloc(vec, newcap) do {                                    \\
    size_t _newcap=(newcap);                                                \\
    size_t _oldcap=(vec)->capacity;                                         \\
                                                                            \\
    if (_newcap < (vec)->size) {                                            \\
        (vec)->size=_newcap;                                                \\
    }                                                                       \\
                                                                            \\
    if (_newcap < 1) _newcap=1;                                             \\
                                                                            \\
    size_t _sizeof_type = sizeof((vec)->data[0]);                           \\
                                                                            \\
    if (_newcap != _oldcap) {                                               \\
        (vec)->data = realloc((vec)->data, _newcap*_sizeof_type);           \\
        if (!(vec)->data) {                                                 \\
            fprintf(stderr, "failed to reallocate\\n");                     \\
            exit(1);                                                        \\
        }                                                                   \\
        if (_newcap > _oldcap) {                                            \\
            size_t _num_new_bytes = (_newcap-_oldcap)*_sizeof_type;         \\
            memset((vec)->data + _oldcap, 0, _num_new_bytes);               \\
        }                                                                   \\
                                                                            \\
        (vec)->capacity = _newcap;                                          \\
    }                                                                       \\
} while (0)


// if size > capacity, then a reallocation occurs
// if size <= capacity, then only the ->size field is reset

#define vector_resize(self, newsize) do {                                   \\
    if ((newsize) > (self)->capacity) {                                     \\
        vector_realloc((self), (newsize));                                  \\
    }                                                                       \\
    (self)->size=newsize;                                                   \\
} while (0)

// reserve at least the specified amount of slots.  If the new capacity is
// smaller than the current capacity, nothing happens.  If larger, a
// reallocation occurs.  No change to current contents occurs.
//
// currently, the exact requested amount is used but in the future we can
// optimize to page boundaries.

#define vector_reserve(self, newcap) do {                                   \\
    if ((newcap) > (self)->capacity) {                                      \\
        vector_realloc((self), (newcap));                                   \\
    }                                                                       \\
} while (0)

// set size to zero and realloc to have default initial capacity
#define vector_clear(self) do {                                             \\
    vector_realloc((self), (self)->initsize);                               \\
    (self)->size=0;                                                         \\
} while (0)

// push a new element onto the vector
// if reallocation is needed, size is increased by some factor
// unless size is zero, when a fixed amount are allocated

#define vector_push(self, val) do {                                        \\
    if ((self)->size == (self)->capacity) {                                \\
                                                                           \\
        size_t _newsize=0;                                                 \\
        if ((self)->capacity == 0) {                                       \\
            _newsize=(self)->initsize;                                     \\
        } else {                                                           \\
            _newsize = (size_t)((self)->capacity*(self)->realloc_multval); \\
            _newsize++;                                                    \\
        }                                                                  \\
                                                                           \\
        vector_realloc((self), _newsize);                                  \\
                                                                           \\
    }                                                                      \\
                                                                           \\
    (self)->size++;                                                        \\
    (self)->data[self->size-1] = val;                                      \\
} while (0)

// pop the last element and decrement size; no reallocation is performed
// if the vector is empty, an error message is printed and garbage is 
// returned
//
// we rely on the fact that capacity never goes to zero, so the "garbage"
// is the zeroth element

#define vector_pop(self) ({                                                  \\
    size_t _index=(self)->size-1;                                            \\
    if ((self)->size > 0) {                                                  \\
        (self)->size--;                                                      \\
    } else {                                                                 \\
        fprintf(stderr,                                                      \\
                "attempt to pop from empty vector, returning garbage\\n");   \\
    }                                                                        \\
    (self)->data[_index];                                                    \\
})

"""

header_foot="""
#endif
"""



hformat='''
typedef struct {
    size_t size;            // number of elements that are visible to the user
    size_t capacity;        // number of allocated elements in data vector
    size_t initsize;        // default size on creation, default VECTOR_INITSIZE 
    double realloc_multval; // when capacity is exceeded while pushing, 
                            // reallocate to capacity*realloc_multval,
                            // default VECTOR_PUSH_REALLOC_MULTVAL
                            // if capacity was zero, we allocate to initsize
    %(type)s* data;
} %(shortname)svector;

%(shortname)svector* %(shortname)svector_new();

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


%(shortname)svector* %(shortname)svector_copy(%(shortname)svector* self) {
    %(shortname)svector* vcopy=%(shortname)svector_new();
    vector_resize(vcopy, self->size);

    if (self->size > 0) {
        memcpy(vcopy->data, self->data, self->size*sizeof(%(type)s));
    }

    return vcopy;
}

%(shortname)svector* %(shortname)svector_fromarray(%(type)s* data, size_t size) {
    %(shortname)svector* self=%(shortname)svector_new();
    vector_resize(self, size);

    if (self->size > 0) {
        memcpy(self->data, data, size*sizeof(%(type)s));
    }

    return self;
}

%(shortname)svector* %(shortname)svector_zeros(size_t num) {

    %(shortname)svector* self=%(shortname)svector_new();
    vector_resize(self, num);
    return self;
}

'''


c_format_builtin='''
%(shortname)svector* %(shortname)svector_ones(size_t num) {

    %(shortname)svector* self=%(shortname)svector_new();
    for (size_t i=0; i<num; i++) {
        //%(shortname)svector_push(self,1);
        vector_push(self,1);
    }
    return self;
}

%(shortname)svector* %(shortname)svector_range(long min, long max) {

    %(shortname)svector* self=%(shortname)svector_new();
    for (long i=min; i<max; i++) {
        //%(shortname)svector_push(self,i);
        vector_push(self,i);
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

tformat_builtin='''// This file was auto-generated using vectorgen
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../vector.h"

int main(int argc, char** argv) {
    %(shortname)svector* vec = %(shortname)svector_new();

    for (size_t i=0;i<75; i++) {
        printf("push: %(format)s\\n", (%(type)s)i);
        vector_push(vec, i);
    }

    printf("size: %%ld\\n", vec->size);
    printf("capacity: %%ld\\n", vec->capacity);

    size_t newsize=25;

    printf("reallocating to size %%ld\\n", newsize);
    vector_realloc(vec, newsize);

    printf("size: %%ld\\n", vec->size);
    printf("capacity: %%ld\\n", vec->capacity);

    while (vec->size > 0) {
        printf("pop: %(format)s\\n", vector_pop(vec));
    }

    // size, vec->size is also exposed
    printf("size: %%ld\\n", vector_size(vec));
    // capacity, vec->capacity is also exposed
    printf("capacity: %%ld\\n", vector_capacity(vec));

    printf("popping the now empty vector, should give zero and an error message: \\n");
    printf("    %(format)s\\n", vector_pop(vec));


    for (size_t i=0;i<10; i++) {
        vector_push(vec, i);
    }

    // only works with -std=gnu99
    printf("testing foreach\\n");
    size_t index=0;
    vector_foreach(iter, vec) {
        assert(*iter == index);
        index++;
    }

    printf("putting unordered elements\\n");

    // note vec->data is also exposed
    vector_set(vec, 3, 88);
    vector_set(vec, 5,25);
    vec->data[9] = 1.3;

    printf("sorting\\n");
    %(shortname)svector_sort(vec);
    for (size_t i=0; i<vec->size; i++) {
        printf("    vec[%%ld]: %(format)s\\n", i, vector_get(vec,i));
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
        assert(vector_get(vec,i)==vector_get(vcopy,i));
        assert(vector_get(vec,i)==vector_get(vcopy_arr,i));
        printf("    compare: %(format)s %(format)s %(format)s\\n",
               vector_get(vec,i),
               vector_get(vcopy,i),
               vector_get(vcopy_arr,i));
    }

    long min=3, max=10;
    printf("making range [%%ld,%%ld)\\n", min, max);

    %(shortname)svector* vrng=%(shortname)svector_range(min, max);
    for (size_t i=0; i<vrng->size; i++) {
        printf("    %%lu %(format)s\\n", i, vector_get(vrng,i));
    }

    size_t nzero=3;
    printf("making zeros[%%lu]\\n", nzero);
    %(shortname)svector* vzeros=%(shortname)svector_zeros(nzero);
    for (size_t i=0; i<nzero; i++) {
        printf("    %%lu %(format)s\\n", i, vector_get(vzeros,i));
    }

    size_t none=3;
    printf("making ones[%%lu]\\n", none);
    %(shortname)svector* vones=%(shortname)svector_ones(none);
    for (size_t i=0; i<none; i++) {
        printf("    %%lu %(format)s\\n", i, vector_get(vones,i));
    }



    printf("freeing vectors\\n");
    printf("freeing vec\\n");
    vector_free(vec);
    assert(vec==NULL);

    printf("freeing vcopy\\n");
    vector_free(vcopy);
    assert(vcopy==NULL);

    printf("freeing vcopy_arr\\n");
    vector_free(vcopy_arr);
    assert(vcopy_arr==NULL);

    printf("freeing vrng\\n");
    vector_free(vrng);
    assert(vrng==NULL);

    printf("freeing vzeros\\n");
    vector_free(vzeros);
    assert(vzeros==NULL);

    printf("freeing vones\\n");
    vector_free(vones);
    assert(vones==NULL);
}
'''

tformat_user='''// This file was auto-generated using vectorgen
// since the type is user defined, we don't know how to print it.
// so nothing will be printed!
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../vector.h"

int main(int argc, char** argv) {
    %(shortname)svector* vec = %(shortname)svector_new();

    %(type)s var;
    memset(&var, 0, sizeof(%(type)s));
    for (size_t i=0;i<75; i++) {
        vector_push(vec, var);
    }

    printf("size: %%ld\\n", vec->size);
    printf("capacity: %%ld\\n", vec->capacity);

    size_t newsize=25;
    printf("reallocating to size %%ld\\n", newsize);
    vector_realloc(vec, newsize);

    printf("size: %%ld\\n", vec->size);
    printf("capacity: %%ld\\n", vec->capacity);

    printf("making a copy\\n");
    %(shortname)svector* vcopy=%(shortname)svector_copy(vec);

    printf("making a copy from array\\n");
    %(shortname)svector* vcopy_arr=%(shortname)svector_fromarray(vec->data, vec->size);

    for (size_t i=0; i<vec->size; i++) {
        %(type)s val=vector_get(vec,i);
        %(type)s valcpy=vector_get(vcopy,i);
        %(type)s valcpy_arr=vector_get(vcopy_arr,i);

        assert(memcmp(&val, &valcpy, sizeof(%(type)s)) == 0);
        assert(memcmp(&val, &valcpy_arr, sizeof(%(type)s)) == 0);
    }

    // only works with -std=gnu99
    printf("testing foreach\\n");
    size_t index=0;
    vector_foreach(iter, vec) {
        %(type)s val = vector_get(vcopy,index);
        assert(memcmp(iter, &val, sizeof(%(type)s)) == 0);
        index++;
    }

    while (vec->size > 0) {
        var = vector_pop(vec);
    }

    printf("size: %%ld\\n", vec->size);
    printf("capacity: %%ld\\n", vec->capacity);

    printf("popping the now empty vector, should give an error message: \\n");
    vector_pop(vec);

    vector_free(vec);
    assert(vec==NULL);
    vector_free(vcopy);
    assert(vcopy==NULL);
    vector_free(vcopy_arr);
    assert(vcopy_arr==NULL);
}
'''

c_head="""// This file was auto-generated using vectorgen

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


