#include <stdlib.h>
#include <stdio.h>
#include "gmix.h"
#include "gsim_ring.h"
#include "shape.h"
#include "image.h"
#include "time.h"
#include "fileio.h"

#include "object.h"

/*
void show_image(const struct image *self, const char *name)
{
    char cmd[256];
    printf("writing temporary image to: %s\n", name);
    FILE *fobj=fopen(name,"w");
    int ret=0;
    image_write(self, fobj);

    fclose(fobj);

    sprintf(cmd,"image-view -m %s", name);
    printf("%s\n",cmd);
    ret=system(cmd);

    sprintf(cmd,"rm %s", name);
    printf("%s\n",cmd);
    ret=system(cmd);
    printf("ret: %d\n", ret);
}
*/

static FILE *open_file(const char *name)
{
    FILE *fobj=fopen(name,"r");
    if (!fobj) {
        fprintf(stderr,"error opening file: %s\n", name);
    }
    return fobj;
}

int main(int argc, char **argv)
{

    if (argc < 2) {
        printf("usage: test objlist\n");
        exit(1);
    }

    FILE *stream = open_file(argv[1]);
    if (!stream) {
        exit(1);
    }

    long nlines = fileio_count_lines(stream);
    rewind(stream);

    fprintf(stderr,"reading %ld objects\n", nlines);

    struct object obj={{0}};
    for (long i=0; i<nlines; i++) {
        if (!object_read(&obj, stream)) {
            fprintf(stderr, "error reading object, aborting: %s: %d",
                    __FILE__,__LINE__);
            exit(1);

        }

        object_print(&obj, stdout);
    }
    return 0;
}
