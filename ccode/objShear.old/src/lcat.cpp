#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "lcat.h"
#include "assert.h"

#include "angDist.h"

void read_lens(const char* filename, std::vector<struct lens>& lcat) {

    int32 nlens;
    FILE* fptr;
    printf("Reading lens file %s\n", filename);
    if (! (fptr = fopen(filename, "r")) ) {
        printf("Cannot open lens file %s\n", filename);
        exit(45);
    }

    int32 nread = fread(&nlens, sizeof(int32), 1, fptr);
    assert(nread == 1);

    printf("    allocating for %d rows\n", nlens);

    lcat.resize(nlens);

    printf("sizeof(struct lens): %ld\n", sizeof(struct lens));
    printf("Reading this many bytes: %ld\n", sizeof(struct lens)*nlens);
    nread = fread(&lcat[0], sizeof(struct lens), nlens, fptr);

    if (nread != nlens) {
        printf("Error reading lenses.  Expected %d but only read %d\n",
               nlens, nread);
        exit(45);
    }

    fclose(fptr);
}

void add_lens_distances(float H0, float omega_m, std::vector<struct lens>& lcat) {

    printf("Calculating lens aeta and Da...");fflush(stdout);

    float aeta0 = aeta(0.0, omega_m);
    for (int i=0; i<lcat.size(); i++) {
        lcat[i].aeta_rel = aeta0 - aeta(lcat[i].z, omega_m);
        lcat[i].Da = angDist(H0, omega_m, lcat[i].z);
    }
    printf("Done.\n");
}



void print_lens_row(std::vector<struct lens>& lcat, int32 row) {
    printf("%lf %lf %f %f %f %d\n", 
           lcat[row].ra,
           lcat[row].dec,
           lcat[row].z,
           lcat[row].aeta_rel,
           lcat[row].Da,
           lcat[row].zindex);

}
void print_lens_firstlast(std::vector<struct lens>& lcat) {
    printf("First and last lens rows\n");

    print_lens_row(lcat, 0);
    print_lens_row(lcat, lcat.size()-1);
}
