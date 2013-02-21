#ifndef _CATALOG_HGUARD
#define _CATALOG_HGUARD

#include "object.h"

struct catalog {
    ssize_t size;

    struct object *data;
};

struct catalog *catalog_read(const char *filename);
struct catalog *catalog_free(struct catalog *self);

#endif
