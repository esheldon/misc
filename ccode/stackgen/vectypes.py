import os
import sys

# ctype is the actual C variable type name
# shortname is for the struct name and functions, e.g. 
#   struct dvector, dvector_new
# defval is the value to return from an empty vector
typemap={}
typemap['float'] = {'ctype':'float',  'shortname':'f',  'defval':'FLT_MAX',    'sortype':'float',   'format':'%f'}
typemap['double'] = {'ctype':'double',  'shortname':'d',  'defval':'DBL_MAX',    'sortype':'double',  'format':'%lf'}
typemap['int8']    = {'ctype':'int8_t',   'shortname':'b',  'defval':'INT8_MAX',   'sortype':'int32_t', 'format':'%d'}
typemap['uint8']   = {'ctype':'uint8_t',  'shortname':'ub', 'defval':'UINT8_MAX',  'sortype':'int32_t', 'format':'%u'}
typemap['int16']   = {'ctype':'int16_t',  'shortname':'s',  'defval':'INT16_MAX',  'sortype':'int32_t', 'format':'%d'}
typemap['uint16']  = {'ctype':'uint16_t', 'shortname':'us', 'defval':'UINT16_MAX', 'sortype':'int32_t', 'format':'%u'}
typemap['int32']   = {'ctype':'int32_t',  'shortname':'i',  'defval':'INT32_MAX',  'sortype':'int64_t', 'format':'%d'}
typemap['uint32']  = {'ctype':'uint32_t', 'shortname':'u',  'defval':'UINT32_MAX', 'sortype':'int64_t', 'format':'%u'}
typemap['int64']   = {'ctype':'int64_t',  'shortname':'l',  'defval':'INT64_MAX',  'sortype':'int64_t', 'format':'%ld'}
typemap['uint64']  = {'ctype':'uint64_t', 'shortname':'ul', 'defval':'UINT64_MAX', 'sortype':'int64_t', 'format':'%lu'}
typemap['char']    = {'ctype':'char',     'shortname':'c',  'defval':"'\\0\'",     'sortype':'int32_t', 'format':'%c'}
typemap['uchar']   = {'ctype':'unsigned char', 'shortname': 'uc',  'defval':"'\\0\'", 'sortype':'int32_t',  'format':'%c'}

typemap['size']    = {'ctype':'size_t', 'shortname':'sz', 'defval':'SIZE_MAX',   'sortype':'int64_t', 'format':'%lu'}

keys=list(typemap.keys())
for k in keys:
    t=typemap[k]
    typemap[t['shortname']] = t

hformat='''
struct %(shortname)svector {
    size_t size;            // number of elements that are visible to the user
    size_t allocated_size;  // number of allocated elements in data vector
    size_t push_realloc_style; // Currently always VECTOR_PUSH_REALLOC_MULT, 
                               // which is reallocate to allocated_size*realloc_multval
    size_t push_initsize;      // default size on first push, default VECTOR_PUSH_INITSIZE 
    double realloc_multval; // when allocated size is exceeded while pushing, 
                            // reallocate to allocated_size*realloc_multval, default 
                            // VECTOR_PUSH_REALLOC_MULTVAL
                            // if allocated_size was zero, we allocate to push_initsize
    %(type)s* data;
};

typedef struct %(shortname)svector %(shortname)svector;

%(shortname)svector* %(shortname)svector_new(size_t num);

// if size > allocated size, then a reallocation occurs
// if size <= internal size, then only the ->size field is reset
// use %(shortname)svector_realloc() to reallocate the data vector and set the ->size
void %(shortname)svector_resize(%(shortname)svector* vec, size_t newsize);

// perform reallocation on the underlying data vector. This does
// not change the size field unless the new size is smaller
// than the viewed size
void %(shortname)svector_realloc(%(shortname)svector* vec, size_t newsize);

// completely clears memory in the data vector
void %(shortname)svector_clear(%(shortname)svector* vec);

// clears all memory and sets pointer to NULL
// usage: vector=%(shortname)svector_delete(vec);
%(shortname)svector* %(shortname)svector_delete(%(shortname)svector* vec);

// if reallocation is needed, size is increased by 50 percent
// unless size is zero, when it 100 are allocated
void %(shortname)svector_push(%(shortname)svector* vec, %(type)s val);
// pop the last element and decrement size; no reallocation is performed
// if empty, INT64_MIN is returned
%(type)s %(shortname)svector_pop(%(shortname)svector* vec);

int __%(shortname)svector_compare_el(const void *a, const void *b);
void %(shortname)svector_sort(%(shortname)svector* vec);
%(type)s* %(shortname)svector_find(%(shortname)svector* vec, %(type)s el);
'''

fformat='''
%(shortname)svector* %(shortname)svector_new(size_t num) {
    %(shortname)svector* vec = malloc(sizeof(%(shortname)svector));
    if (vec == NULL) {
        fprintf(stderr,"Could not allocate %(shortname)svector\\n");
        return NULL;
    }

    vec->size = 0;
    vec->allocated_size = num;
    vec->push_realloc_style = VECTOR_PUSH_REALLOC_MULT;
    vec->push_initsize = VECTOR_PUSH_INITSIZE;
    vec->realloc_multval = VECTOR_PUSH_REALLOC_MULTVAL;

    if (num == 0) {
        vec->data = NULL;
    } else {
        vec->data = calloc(num, sizeof(%(type)s));
        if (vec->data == NULL) {
            free(vec);
            fprintf(stderr,"Could not allocate data for vector\\n");
            return NULL;
        }
    }

    return vec;
}

void %(shortname)svector_realloc(%(shortname)svector* vec, size_t newsize) {

    size_t oldsize = vec->allocated_size;
    if (newsize != oldsize) {
        size_t elsize = sizeof(%(type)s);

        %(type)s* newdata = realloc(vec->data, newsize*elsize);
        if (newdata == NULL) {
            fprintf(stderr,"failed to reallocate\\n");
            return;
        }

        if (newsize > vec->allocated_size) {
            // the allocated size is larger.  make sure to initialize the new
            // memory region.  This is the area starting from index [oldsize]
            size_t num_new_bytes = (newsize-oldsize)*elsize;
            memset(&newdata[oldsize], 0, num_new_bytes);
        } else if (vec->size > newsize) {
            // The viewed size is larger than the allocated size in this case,
            // we must set the size to the maximum it can be, which is the
            // allocated size
            vec->size = newsize;
        }

        vec->data = newdata;
        vec->allocated_size = newsize;
    }

}
void %(shortname)svector_resize(%(shortname)svector* vec, size_t newsize) {
   if (newsize > vec->allocated_size) {
       %(shortname)svector_realloc(vec, newsize);
   }

   vec->size = newsize;
}

void %(shortname)svector_clear(%(shortname)svector* vec) {
    vec->size=0;
    vec->allocated_size=0;
    free(vec->data);
    vec->data=NULL;
}

%(shortname)svector* %(shortname)svector_delete(%(shortname)svector* vec) {
    if (vec != NULL) {
        %(shortname)svector_clear(vec);
        free(vec);
    }
    return NULL;
}

void %(shortname)svector_push(%(shortname)svector* vec, %(type)s val) {
    // see if we have already filled the available data vector
    // if so, reallocate to larger storage
    if (vec->size == vec->allocated_size) {

        size_t newsize;
        if (vec->allocated_size == 0) {
            newsize=vec->push_initsize;
        } else {
            // currenly we always use the multiplier reallocation  method.
            if (vec->push_realloc_style != VECTOR_PUSH_REALLOC_MULT) {
                fprintf(stderr,"Currently only support push realloc style VECTOR_PUSH_REALLOC_MULT\\n");
                exit(EXIT_FAILURE);
            }
            // this will "floor" the size
            newsize = (size_t)(vec->allocated_size*vec->realloc_multval);
            // we want ceiling
            newsize++;
        }

        %(shortname)svector_realloc(vec, newsize);

    }

    vec->size++;
    vec->data[vec->size-1] = val;
}

%(type)s %(shortname)svector_pop(%(shortname)svector* vec) {
    if (vec->size == 0) {
        return %(defval)s;
    }

    %(type)s val=vec->data[vec->size-1];
    vec->size--;
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


void %(shortname)svector_sort(%(shortname)svector* vec) {
    qsort(vec->data, vec->size, sizeof(%(type)s), __%(shortname)svector_compare_el);
}
%(type)s* %(shortname)svector_find(%(shortname)svector* vec, %(type)s el) {
    return (%(type)s*) bsearch(&el, vec->data, vec->size, sizeof(%(type)s), __%(shortname)svector_compare_el);
}
'''

tformat='''// This file was auto-generated
#include <stdio.h>
#include <stdlib.h>
#include "vector.h"

int main(int argc, char** argv) {
    %(shortname)svector* vec = %(shortname)svector_new(0);

    for (size_t i=0;i<75; i++) {
        printf("push: %(format)s\\n", (%(type)s)i);
        %(shortname)svector_push(vec, i);
    }

    printf("size: %%ld\\n", vec->size);
    printf("allocated size: %%ld\\n", vec->allocated_size);

    size_t newsize=25;
    printf("reallocating to size %%ld\\n", newsize);
    %(shortname)svector_realloc(vec, newsize);
    printf("size: %%ld\\n", vec->size);
    printf("allocated size: %%ld\\n", vec->allocated_size);

    while (vec->size > 0) {
        printf("pop: %(format)s\\n", %(shortname)svector_pop(vec));
    }

    printf("size: %%ld\\n", vec->size);
    printf("allocated size: %%ld\\n", vec->allocated_size);

    printf("popping the now empty list: \\n    %(format)s\\n", %(shortname)svector_pop(vec));


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

#define VECTOR_PUSH_REALLOC_MULT 1
#define VECTOR_PUSH_REALLOC_MULTVAL 2
#define VECTOR_PUSH_INITSIZE 1
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
                          'defval':typemap[type]['defval'],
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

