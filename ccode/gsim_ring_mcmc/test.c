#include <stdlib.h>
#include <stdio.h>
#include "gmix.h"
#include "gsim_ring.h"
#include "shape.h"
#include "image.h"
#include "time.h"

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
/*
void skip_line(FILE *stream) {
    int c=0;
    while (c != '\n') {
        c=fgetc(stream);
    }
}
*/
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
    return 0;
}
