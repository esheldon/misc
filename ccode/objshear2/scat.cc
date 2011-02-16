#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "scat.h"
#include "assert.h"

#include "Cosmology.h"

using std::vector;

void read_source(const char* filename, vector<struct source>& scat) {

    int32 nsource;
    FILE* fptr;
    printf("Reading source file %s\n", filename);
    if (! (fptr = fopen(filename, "r")) ) {
        printf("Cannot open source file %s\n", filename);
        exit(45);
    }

    int32 nread = fread(&nsource, sizeof(int32), 1, fptr);
    assert(nread == 1);

    printf("    allocating for %d rows\n", nsource);

    scat.resize(nsource);

    nread = fread(&scat[0], sizeof(struct source), nsource, fptr);

    if (nread != nsource) {
        printf("Error reading sources.  Expected %d but only read %d\n",
               nsource, nread);
        exit(45);
    }
    printf("Done.\n");

    fclose(fptr);
}

void add_source_distances(
        float H0,
        float omega_m, 
        vector<struct source>& scat) {

    printf("Calculating source dc...");fflush(stdout);

    Cosmology cosmo(H0, omega_m);

    for (int i=0; i<scat.size(); i++) {
        scat[i].dc = cosmo.dc(0.0, scat[i].z);
    }
    printf("Done.\n");
}

void print_source_row(vector<struct source>& scat, int32 row) {

#ifdef INTERP_SCINV
    printf("%lf %lf %f %f %f %d %f\n", 
#else
    printf("%lf %lf %f %f %f %d %f %f\n", 
#endif
           scat[row].ra,
           scat[row].dec,
           scat[row].e1,
           scat[row].e2,
           scat[row].err,
           scat[row].htm_index,
#ifdef INTERP_SCINV
           scat[row].mean_scinv[5]
#else
           scat[row].z,
           scat[row].dc
#endif
    );

}
void print_source_firstlast(vector<struct source>& scat) {
    printf("First and last source_true rows\n");

    print_source_row(scat, 0);
    print_source_row(scat, scat.size()-1);
}
