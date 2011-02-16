#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "lcat.h"
#include "assert.h"

#include "Cosmology.h"

using std::vector;

void read_lens(const char* filename, vector<struct lens>& lcat) {

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

void add_lens_distances(float H0, float omega_m, vector<struct lens>& lcat) {

    printf("Calculating lens dc...");fflush(stdout);

    Cosmology cosmo(H0, omega_m);

    for (int i=0; i<lcat.size(); i++) {
        lcat[i].dc = cosmo.dc(0.0, lcat[i].z);
    }
    printf("Done.\n");
}



void print_lens_row(vector<struct lens>& lcat, int32 row) {
    printf("%lf %lf %f %f %d %d\n", 
           lcat[row].ra,
           lcat[row].dec,
           lcat[row].z,
           lcat[row].dc,
           lcat[row].zindex,
           lcat[row].padding);

}
void print_lens_firstlast(vector<struct lens>& lcat) {
    printf("First and last lens rows\n");

    print_lens_row(lcat, 0);
    print_lens_row(lcat, lcat.size()-1);
}
