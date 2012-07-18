#ifndef _POINT_HASH_H
#define _POINT_HASH_H

#include "defs.h"
#include "uthash.h"
#include "point.h"
#include "vector.h"

#define HASH_FIND_INT64(head,findint64,out)        \
    HASH_FIND(hh,head,findint64,sizeof(int64),out)
#define HASH_ADD_INT64(head,int64field,add)        \
    HASH_ADD(hh,head,int64field,sizeof(int64),add)




struct point_hash {
    int64 hpixid;
    struct vector* points;  // these will be pointers to struct point*
    UT_hash_handle hh; /* makes this structure hashable */
};


struct point_hash* point_hash_new();
struct point_hash* point_hash_find(struct point_hash* self, int64 id);
//void point_hash_insert(struct point_hash* self, int64 id, 
//                       struct point* point);

// it is *key* to return self here, since it gets set
// when the first value is added
struct point_hash* point_hash_insert(struct point_hash* self, 
                                     int64 id, 
                                     struct point* point);
#endif
