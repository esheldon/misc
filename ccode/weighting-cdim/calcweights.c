#include <stdlib.h>
#include <stdio.h>
#include "PhotoCatalog.h"
#include "TrainCatalog.h"
#include "KDTree.h"
#include "Vector.h"
#include "util.h"

// prototype
void calcweights(struct TrainCatalog* tcat, struct PhotoCatalog* pcat, int nnear);

int main(int argc, char **argv) {

    if (argc < 6) {
        wlog("Usage: \n");
        wlog("   calcweights%d trainfile photfile n_near weightfile numfile\n\n", NDIM);

        wlog("     NDIM: %d\n", NDIM);
        wlog("     weightsfile and numfile are outputs\n");
        wlog("     n_near=5 is typical first run, 100 second run\n");
        wlog("     don't forget to remove weight=0 objects for second run\n");
        return(1);
    }

    const char* trainfile = argv[1];
    const char* photofile = argv[2];
    //int n_near = charstar2int(argv[3]);
    int n_near = atoi(argv[3]);
    const char* weightsfile = argv[4];
    const char* numfile = argv[5];

    wlog("NDIM: %d\n", NDIM);
    wlog("number of nearest neighbors: %d\n",n_near);

    struct TrainCatalog* tcat=TrainCatalogRead(trainfile);
    struct PhotoCatalog* pcat=PhotoCatalogRead(photofile);

    calcweights(tcat, pcat, n_near);

    wlog("Writing weights file: %s\n", weightsfile);
    TrainCatalogWrite(weightsfile, tcat);
    wlog("Writing num file: %s\n", numfile);
    PhotoCatalogWriteNum(numfile, pcat);
    wlog("Done\n");
}


void calcweights(struct TrainCatalog* tcat, struct PhotoCatalog* pcat, int nnear) {

    size_t ntrain = tcat->size;
    wlog("Creating KDTree for training catalog\n");
    struct KDTree* kd_train = KDTreeCreate(tcat->pts);

    wlog("Creating KDTree for photometric catalog\n");
    struct KDTree* kd_photo = KDTreeCreate(pcat->pts);

    wlog("Allocating photometric num field\n");
    PhotoCatalogMakeNum(pcat);


    // fixed size arrays to hold results
    wlog("Allocating dist,indices\n");
    double* dist  = malloc(nnear*sizeof(double));
    int* indices = malloc(nnear*sizeof(int));

    // this may be expanded
    struct I4Vector* matchind = I4VectorZeros(MAX_RADMATCH);

    double point[NDIM];
    double total_weights = 0.0;
    wlog("Getting weights/num\n");

    int step=10000;
    if (ntrain > step) {
        wlog("Each dot is %d\n", step);
    }
    for (size_t i=0; i<ntrain; i++) {

        if ( ((i+1) % 200000) == 0) {
            wlog("\n%ld/%ld  (%0.1f%%)\n", i+1, ntrain, 100.*(float)(i+1)/ntrain);
            fflush(stdout);
        }

        // will get distance to closest n_near neighbors in the train sample
        // the zeroth element will hold the farthest. also get the indices

        KDTreeNNeighOfIndex(kd_train, nnear, i, dist, indices);

        // Get number of photometric objects within dist[0] of the
        // training set object.  dist[0] is gauranteed to be the largest
        // distance in the training set neighbors

        KDTreeCopyPoint(kd_train, i, point);
        int nmatch = KDTreePointRadMatch(kd_photo,point,dist[0],matchind);

        // can we make this weighted?  Count a weight instead of "1" for
        // each photometric object?
        tcat->weights[i] = ((double)nmatch)/((double)nnear);
        total_weights += tcat->weights[i];

        // count how many times each data object was used
        for (size_t j=0; j<nmatch; j++) {
            pcat->num[ matchind->data[j] ] ++;
        }

        if ( (ntrain > step) && (i % step) == 0 ) {
            wlog("."); 
        }
    }
    if (ntrain > step) {
        wlog("\n");
    }

    // normalize weights
    for (size_t i=0; i<ntrain; i++) {
        tcat->weights[i] /= total_weights;
    }

    free(dist);
    free(indices);
    I4VectorFree(matchind);
    KDTreeFree(kd_train);
    KDTreeFree(kd_photo);
}
