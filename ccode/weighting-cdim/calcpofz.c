#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "PhotoCatalog.h"
#include "TrainCatalog.h"
#include "KDTree.h"
#include "WHist.h"
#include "util.h"

// prototypes
void calcpofz(struct TrainCatalog* tcat, const char* photofile, 
        int nnear, int nz, double zmin, double zmax, 
        const char* pzfile, const char* zfile);
void PofZWriteOne(FILE* fptr, int64_t id, struct WHist* wh);

int main(int argc, char **argv) {
    if (argc < 9) {
        printf("Usage: \n");
        printf("  calcpofz%d weightsfile photofile nnear nz ", NDIM);
        printf(                            " zmin zmax pzfile zfile\n\n");

        printf("    NDIM: %d\n", NDIM);
        printf("    weightsfile is the output from calcweights. \n");
        printf("    nnear should be about 100\n");
        printf("    nz is the number of z points in the p(z), e.g. 20 or 30\n");
        return(1);
    }

    const char* wtrainfile = argv[1];
    const char* photofile  = argv[2];
    int nnear              = atoi(argv[3]);
    int nz                 = atoi(argv[4]);
    double zmin            = atof(argv[5]);
    double zmax            = atof(argv[6]);

    const char* pzfile     = argv[7];
    const char* zfile      = argv[8];


    pflush("NDIM: %d\n", NDIM);
    pflush("number of nearest neighbors: %d\n",nnear);
    pflush("nz:   %d\n",nz);
    pflush("zmin: %lf\n",zmin);
    pflush("zmax: %lf\n",zmax);

    struct TrainCatalog* tcat=TrainCatalogRead(wtrainfile);

    calcpofz(tcat, photofile, nnear, nz, zmin, zmax, pzfile, zfile);

    pflush("Done\n");
}

void calcpofz(
        struct TrainCatalog* tcat, 
        const char* photofile, 
        int nnear, 
        int nz, 
        double zmin, 
        double zmax, 
        const char* pzfile, 
        const char* zfile) {

    pflush("Creating KDTree for training catalog\n");
    struct KDTree* kd_train = KDTreeCreate(tcat->pts);

    pflush("Opening PhotoCatalog file: '%s'\n", photofile);
    FILE* phot_fptr = open_or_exit(photofile, "r");

    // fixed size arrays to hold results
    pflush("Allocating dist,indices,zvals,weights\n");
    double* dist  = malloc(nnear*sizeof(double));
    int* indices  = malloc(nnear*sizeof(int));
    double* zvals = malloc(nnear*sizeof(double));
    double* weights = malloc(nnear*sizeof(double));

    pflush("Allocating WHist\n");
    struct WHist* wh = WHistAlloc(nz, zmin, zmax);

    int64_t id;
    double point[NDIM];

    // Output file
    pflush("Opening pofz file for output: '%s'\n", pzfile);
    FILE* pz_fptr = open_or_exit(pzfile,"w");

    int step=10000; size_t i=1;
    while (PhotoCatalogReadOne(phot_fptr, &id, point)) {
        KDTreeNNeighOfPoint(kd_train, nnear, point, dist, indices);
        for (int j=0; j<nnear; j++) {
            zvals[j]   = tcat->zspec[ indices[j] ];
            weights[j] = tcat->weights[ indices[j] ];
        }

        WHistCalc(zvals, weights, nnear, wh);
        PofZWriteOne(pz_fptr, id, wh);

        if ( (i % step) == 0 ) {
            pflush(".");
        }
        i++;
    }
    pflush("\n");


    fclose(phot_fptr);
    fclose(pz_fptr);


    // Output z file
    pflush("Opening z file for output: '%s'\n", zfile);
    FILE* z_fptr = open_or_exit(zfile,"w");
    for (int i=0; i<wh->nbin; i++) {
        fprintf(z_fptr,"%g %g\n", wh->binmin[i], wh->binmax[i]);
    }
    fclose(z_fptr);

    free(dist);
    free(indices);
    free(zvals);
    free(weights);
    free(wh);
}


void PofZWriteOne(FILE* fptr, int64_t id, struct WHist* wh) {
    fprintf(fptr,"%ld ", id);
    for (int j=0; j<wh->nbin; j++) {
        fprintf(fptr,"%g",wh->whist[j]);
        if (j < (wh->nbin-1)) {
            fprintf(fptr, " ");
        }
    }
    fprintf(fptr,"\n");
}
