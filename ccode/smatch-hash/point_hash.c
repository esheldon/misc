#include <stdlib.h>
#include "defs.h"
#include "point.h"
#include "point_hash.h"
#include "alloc.h"
#include "uthash.h"

// for when testing hash
#include <unistd.h>

struct point_hash* point_hash_new(int64 hpixid) {
    struct point_hash* pth = 
        alloc_or_die(sizeof(struct point_hash),"hash entry");
    pth->points = ptrstack_new(0);
    pth->hpixid=hpixid;
    return pth;
}
struct point_hash* point_hash_find(struct point_hash* self, int64 id) {
    struct point_hash* pth=NULL;
    HASH_FIND_INT64(self, &id, pth);
    return pth;
}

// it is *key* to return self here, since it gets set
// when the first value is added
struct point_hash* point_hash_insert(struct point_hash* self, 
                                     int64 id, 
                                     struct point* point) {
    struct point_hash* pth = point_hash_find(self, id);
    if (pth == NULL) {
        pth = point_hash_new(id);
        // note this modifies self when self is NULL
        // hpixid is expanded to pth->hpixid
        HASH_ADD_INT64(self, hpixid, pth);
    }

    // add a new pointer as data
    ptrstack_push(pth->points,point);

    return self;
}

