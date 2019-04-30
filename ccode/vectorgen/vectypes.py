from __future__ import print_function
import os
import sys
import yaml

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

#include <math.h>
#include <stdint.h>
#include <string.h>

// initial capacity of vectors created with new()
// and capacity of cleared vectors
#define VECTOR_INITCAP 1

// make sure this is an integer for now
#define VECTOR_PUSH_REALLOC_MULTVAL 1.5

// properties, generic macros
#define vector_size(vec) (vec)->size
#define vector_capacity(vec) (vec)->capacity

//
// getters and setters, generic macros
// UNSAFE
#define vector_get(vec, i) (vec)->data[i]

// UNSAFE
#define vector_set(vec, i, val) do {                                         \\
    (vec)->data[(i)] = (val);                                                \\
} while(0)

// pointer to the underlying data
#define vector_data(vec) (vec)->data

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
    vector_realloc((self), VECTOR_INITCAP);                                 \\
    (self)->size=0;                                                         \\
} while (0)

// push a new element onto the vector
// if reallocation is needed, size is increased by some factor
// unless size is zero, when a fixed amount are allocated

#define vector_push(self, val) do {                                        \\
    if ((self)->size == (self)->capacity) {                                \\
                                                                           \\
        double tmp=(self)->capacity*VECTOR_PUSH_REALLOC_MULTVAL;           \\
        size_t _newsize=(size_t)ceil(tmp);                                 \\
        if (_newsize < 2) {                                                \\
            _newsize=2;                                                    \\
        }                                                                  \\
                                                                           \\
        vector_realloc((self), _newsize);                                  \\
                                                                           \\
    }                                                                      \\
                                                                           \\
    (self)->size++;                                                        \\
    (self)->data[self->size-1] = val;                                      \\
} while (0)

// UNSAFE
// pop the last element and decrement size; no reallocation is performed
// if the vector is empty, an error message is printed and garbage is
// returned
//
// we rely on the fact that capacity never goes to zero, so the "garbage"
// is the zeroth element

#define vector_pop(self) ({                                                  \\
    size_t _index=0;                                                         \\
    if ((self)->size > 0) {                                                  \\
        _index=(self)->size-1;                                               \\
        (self)->size--;                                                      \\
    } else {                                                                 \\
        fprintf(stderr,                                                      \\
        "VectorError: attempt to pop from empty vector, returning garbage\\n");   \\
    }                                                                        \\
    (self)->data[_index];                                                    \\
})

// add the elements of v2 to v1
// if the vectors are not the same size, then only the smallest
// number are added
#define vector_add_inplace(v1, v2) do {                                    \\
    size_t _ii;                                                            \\
    size_t _num=0;                                                         \\
    size_t _num1=vector_size( (v1) );                                      \\
    size_t _num2=vector_size( (v2) );                                      \\
    if (_num1 != _num2) {                                                  \\
        fprintf(stderr,                                                    \\
         "VectorWarning: vectors are not the same size, adding subset\\n");\\
        if (_num1 < _num2) {                                               \\
            _num=_num1;                                                    \\
        } else {                                                           \\
            _num=_num2;                                                    \\
        }                                                                  \\
    } else {                                                               \\
        _num=_num1;                                                        \\
    }                                                                      \\
    for (_ii=0; _ii<_num; _ii++) {                                         \\
        (v1)->data[_ii] += (v2)->data[_ii];                                \\
    }                                                                      \\
} while (0)


// not using foreach here since that requires gnu99
#define vector_add_scalar(self, val) do {                                  \\
    size_t _ii=0;                                                          \\
    for (_ii=0; _ii < vector_size( (self) ); _ii++) {                      \\
        (self)->data[_ii] += (val);                                        \\
    }                                                                      \\
} while (0)

// multiply the elements of v2 to v1
// if the vectors are not the same size, then only the smallest
// number are multiplied
#define vector_mult_inplace(v1, v2) do {                                   \\
    size_t _ii=0;                                                          \\
    size_t _num=0;                                                         \\
    size_t _num1=vector_size( (v1) );                                      \\
    size_t _num2=vector_size( (v2) );                                      \\
    if (_num1 != _num2) {                                                  \\
        fprintf(stderr,                                                    \\
         "warning: vectors are not the same size, multiplying subset\\n"); \\
        if (_num1 < _num2) {                                               \\
            _num=_num1;                                                    \\
        } else {                                                           \\
            _num=_num2;                                                    \\
        }                                                                  \\
    } else {                                                               \\
        _num=_num1;                                                        \\
    }                                                                      \\
    for (_ii=0; _ii<_num; _ii++) {                                         \\
        (v1)->data[_ii] *= (v2)->data[_ii];                                \\
    }                                                                      \\
} while (0)


// not using foreach here since that requires gnu99
#define vector_mult_scalar(self, val) do {                                 \\
    size_t _ii=0;                                                          \\
    for (_ii=0; _ii < vector_size( (self) ); _ii++) {                      \\
        (self)->data[_ii] *= (val);                                        \\
    }                                                                      \\
} while (0)



"""

header_foot="""
#endif
"""



hformat='''

/*
 *
 * definitions for %(type)s vectors
 *
 */

typedef struct {
    size_t size;            // number of elements that are visible to the user
    size_t capacity;        // number of allocated elements in data vector
    %(type)s* data;
} %(shortname)svector;

// create a new vector with VECTOR_INITCAP capacity and zero visible size
%(shortname)svector* %(shortname)svector_new(void);

// make a new copy of the vector
%(shortname)svector* %(shortname)svector_copy(%(shortname)svector* self);

// make a new vector with data copied from the input array
%(shortname)svector* %(shortname)svector_fromarray(%(type)s* data, size_t size);

// make a vector with the specified initial size, zeroed
%(shortname)svector* %(shortname)svector_zeros(size_t num);
'''

hformat_builtin='''
//
// these are only written for the builtins
//

// make a vector with the specified initial size, set to 1
%(shortname)svector* %(shortname)svector_ones(size_t num);

// make a vector with elements [0,1,2,...max)
%(shortname)svector* %(shortname)svector_range(long min, long max);

// compare elements of type %(type)s
int __%(shortname)svector_compare_el(const void *a, const void *b);

// sort the vector using the above comparison function
void %(shortname)svector_sort(%(shortname)svector* self);

// find an occurence of the requested value. Assumes the vector is sorted.
%(type)s* %(shortname)svector_find(%(shortname)svector* self, %(type)s el);
'''

c_format='''
%(shortname)svector* %(shortname)svector_new(void) {
    %(shortname)svector* self = calloc(1,sizeof(%(shortname)svector));
    if (self == NULL) {
        fprintf(stderr,"Could not allocate %(shortname)svector\\n");
        return NULL;
    }

    self->capacity = VECTOR_INITCAP;

    self->data = calloc(self->capacity, sizeof(%(type)s));
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
    size_t i=0;
    %(shortname)svector* self=%(shortname)svector_new();

    vector_resize(self, num);

    for (i=0; i<num; i++) {
        self->data[i] = 1;
    }
    return self;
}

%(shortname)svector* %(shortname)svector_range(long min, long max) {
    long i=0;
    %(shortname)svector* self=%(shortname)svector_new();
    for (i=min; i<max; i++) {
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

void print_sizecap(%(shortname)svector* vec) {
    printf("size: %%ld capacity: %%ld\\n",
           vector_size(vec), vector_capacity(vec));
}

int main(int argc, char** argv) {
    %(shortname)svector* vec = %(shortname)svector_new();

    for (size_t i=0;i<15; i++) {
        vector_push(vec, i);
        printf("push: %(format)s cap: %%lu\\n",
               (%(type)s)i, vector_capacity(vec));
    }

    print_sizecap(vec);

    size_t newsize=10;

    printf("reallocating to size %%ld\\n", newsize);
    vector_realloc(vec, newsize);

    print_sizecap(vec);

    printf("popping everything\\n");
    while (vector_size(vec) > 0) {
        printf("pop: %(format)s\\n", vector_pop(vec));
    }

    print_sizecap(vec);

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

    vector_set(vec, 3, 88);
    vector_set(vec, 5,25);
    vec->data[9] = 1.3;

    printf("sorting\\n");
    %(shortname)svector_sort(vec);
    for (size_t i=0; i < vector_size(vec); i++) {
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
    %(shortname)svector* vcopy_arr=%(shortname)svector_fromarray(vector_data(vec), vector_size(vec));

    for (size_t i=0; i < vector_size(vec); i++) {
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

    size_t n_zero=3;
    printf("making zeros[%%lu]\\n", n_zero);
    %(shortname)svector* vzeros=%(shortname)svector_zeros(n_zero);
    for (size_t i=0; i<n_zero; i++) {
        printf("    %%lu %(format)s\\n", i, vector_get(vzeros,i));
    }

    size_t n_one=3;
    printf("making ones[%%lu]\\n", n_one);
    %(shortname)svector* vones=%(shortname)svector_ones(n_one);
    for (size_t i=0; i<vector_size(vones); i++) {
        printf("    %%lu %(format)s\\n", i, vector_get(vones,i));
    }

    int scalar=3;
    printf("adding scalar to vzeros: %%d\\n", scalar);
    vector_add_scalar(vzeros, scalar);
    for (size_t i=0; i<vector_size(vzeros); i++) {
        printf("    %%lu %(format)s\\n", i, vector_get(vzeros,i));
    }

    printf("adding vones to vzeros in place\\n");
    vector_add_inplace(vzeros, vones);
    for (size_t i=0; i<vector_size(vzeros); i++) {
        printf("    %%lu %(format)s\\n", i, vector_get(vzeros,i));
    }

    printf("intentionally adding mismatch sizes, expect a warning\\n");
    vector_add_inplace(vzeros, vrng);
    for (size_t i=0; i<vector_size(vzeros); i++) {
        printf("    %%lu %(format)s\\n", i, vector_get(vzeros,i));
    }

    scalar=27;
    printf("multiplying scalar by vones: %%d\\n", scalar);
    vector_mult_scalar(vones, scalar);
    for (size_t i=0; i<vector_size(vones); i++) {
        printf("    %%lu %(format)s\\n", i, vector_get(vones,i));
    }

    printf("multiplying vones to vzeros in place\\n");
    vector_mult_inplace(vzeros, vones);
    for (size_t i=0; i<vector_size(vzeros); i++) {
        printf("    %%lu %(format)s\\n", i, vector_get(vzeros,i));
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

void print_sizecap(%(shortname)svector* vec) {
    printf("size: %%ld capacity: %%ld\\n",
           vector_size(vec), vector_capacity(vec));
}


int main(int argc, char** argv) {
    %(shortname)svector* vec = %(shortname)svector_new();

    %(type)s var;
    memset(&var, 0, sizeof(%(type)s));
    for (size_t i=0;i<75; i++) {
        vector_push(vec, var);
    }

    print_sizecap(vec);

    size_t newsize=25;
    printf("reallocating to size %%ld\\n", newsize);
    vector_realloc(vec, newsize);

    print_sizecap(vec);

    printf("making a copy\\n");
    %(shortname)svector* vcopy=%(shortname)svector_copy(vec);

    printf("making a copy from array\\n");
    %(shortname)svector* vcopy_arr=%(shortname)svector_fromarray(vector_data(vec), vector_size(vec));

    for (size_t i=0; i<vector_size(vec); i++) {
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

    printf("popping everything\\n");
    while (vector_size(vec) > 0) {
        var = vector_pop(vec);
    }

    print_sizecap(vec);

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
// most array methods are generic, see vector.h

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

    def write(self, prefix='vector'):
        """
        write all
        """

        self.write_header(prefix)
        self.write_c(prefix)
        self.write_tests()

    def write_header(self, prefix):
        """
        write the header
        """

        fname=prefix+'.h'
        print('writing:',fname)
        with open(fname,'w') as fobj:

            fobj.write(header_head)

            for type in self:
                ti = self[type]

                text = hformat % ti
                fobj.write(text)

                if ti['is_builtin']:
                    text = hformat_builtin % ti
                    fobj.write(text)

            fobj.write(header_foot)

    def write_c(self, prefix):
        fname=prefix+'.c'
        print('writing:',fname)
        with open(fname,'w') as fobj:
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


