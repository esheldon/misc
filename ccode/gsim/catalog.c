#include <stdio.h>
#include <stdlib.h>
#include "object_simple.h"
#include "catalog.h"

static ssize_t catalog_count(FILE *fobj)
{
    struct object_simple object = {{0}};

    ssize_t count=0;

    while (object_simple_read_one(&object, fobj)) {
        count += 1;
    }

    rewind(fobj);
    return count;
}

static FILE *open_catalog(const char *filename)
{
    FILE *catfile = fopen(filename,"r");
    if (catfile==NULL) {
        fprintf(stderr,"failed to open catalog: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    return catfile;
}


struct catalog *catalog_read(const char *filename)
{
    struct catalog *self=NULL;
    struct object_simple *object=NULL;

    fprintf(stderr,"reading catalog: %s\n", filename);
    FILE *fobj=open_catalog(filename);

    self=calloc(1,sizeof(struct catalog));
    if (!self) {
        fprintf(stderr,"could not allocate catalog\n");
        exit(EXIT_FAILURE);
    }

    ssize_t count=catalog_count(fobj);

    self->size=count;
    self->data=calloc(count, sizeof(struct object_simple));


    object=self->data;
    for (ssize_t i=0; i< count; i++) {
        object_simple_read_one(object, fobj);
        object++;
    }

    fclose(fobj);
    return self;

}

struct catalog *catalog_free(struct catalog *self)
{
    if (self) {
        if (self->data) {
            free(self->data);
            free(self);
        }
    }
    return NULL;
}
