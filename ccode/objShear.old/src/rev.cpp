#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "rev.h"

void read_rev(const char* filename, struct revhtm& rev) {

    FILE* fptr;
    printf("Reading revind file %s\n", filename);
    if (! (fptr = fopen(filename, "r")) ) {
        printf("Cannot open revind file %s\n", filename);
        exit(45);
    }

    int32 nread;
    int32 nrev;

    nread = fread(&nrev, sizeof(int32), 1, fptr);
    assert(nread == 1);

    nread = fread(&rev.minid, sizeof(int32), 1, fptr);
    assert(nread == 1);

    nread = fread(&rev.maxid, sizeof(int32), 1, fptr);
    assert(nread == 1);

    printf("Reading %d...", nrev);fflush(stdout);
    rev.revind.resize(nrev);
    nread = fread(&rev.revind[0], sizeof(int32), nrev, fptr);

    if (nread != nrev) {
        printf("Error reading rev.  Expected %d but only read %d\n",
               nrev, nread);
        exit(45);
    }
    printf("Done.\n");

    fclose(fptr);

}
void print_rev_sample(struct revhtm& rev) {
    printf("Sample of rev htm:\n");
    size_t nrev = rev.revind.size();
    printf("    nrev:      %ld\n", nrev);
    printf("    minid:     %d\n", rev.minid);
    printf("    maxid:     %d\n", rev.maxid);
    printf("    revind[0]: %d\n", rev.revind[0]);
    printf("    revind[%ld]: %d\n", nrev-1, rev.revind[nrev-1]);
}
