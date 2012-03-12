#include <stdlib.h>
#include <stdio.h>
#include "dims.h"
#include "HCube.h"
#include "TreeNode.h"
#include "Points.h"
#include "KDTree.h"
#include "PhotoCatalog.h"
#include "TrainCatalog.h"

#include "util.h"

#include "params.h"

void test_nnear_ndim5() {

    wlog("\nTesting nearest neighbors\n");
    wlog("Testing Read PhotoCatalog\n");
    const char pfilename[]="data/photo.dat";
    struct PhotoCatalog* pcat=PhotoCatalogRead(pfilename);

    wlog("Creating KDTree for pcat\n");
    struct KDTree* kd_photo = KDTreeCreate(pcat->pts);


    // find the n nearest neighbors of a given point
    int nnear = 5;
    int index=1137;
    double *dist;
    int* indices;

    dist  = malloc(nnear*sizeof(double));
    indices = malloc(nnear*sizeof(int));
    assert( (dist != NULL) && (indices != NULL) );

    wlog("Getting %d nearest neighbors of point %d\n", nnear, index);
    KDTreeNNeighOfIndex(kd_photo, nnear, index, dist, indices);
    wlog("Writing results\n");
    for (int i=0; i<nnear; i++) {
        wlog("    %d    %lf\n", indices[i], dist[i]);
    }

    double point[NDIM] = {22.50043, 20.96843, 20.30887, 19.65721, 19.95109};

    wlog("Getting %d nearest neighbors of point\n    { ",nnear);
    for (int i=0; i<nnear; i++) wlog("%lf ",point[i]);
    wlog("}\n");

    KDTreeNNeighOfPoint(kd_photo, nnear, point, dist, indices);
    wlog("Writing results\n");
    for (int i=0; i<nnear; i++) {
        wlog("    %d    %lf\n", indices[i], dist[i]);
    }

    double radius=0.1;
    struct I4Vector* matchind = I4VectorZeros(MAX_RADMATCH);

    wlog("Getting matches within radius %lf\n", radius);
    int nmatch = KDTreePointRadMatch(kd_photo,point,radius,matchind);
    wlog("    Found %d matches\n", nmatch);
    for (int i=0; i<nmatch; i++) {
        wlog("        %d\n", matchind->data[i]);
    }

    //wlog("Freeing KDTree\n");
    KDTreeFree(kd_photo);

    //wlog("Freeing PhotoCatalog\n");
    PhotoCatalogFree(pcat);

}

void test_read_catalog_ndim5() {
    wlog("NDIM = %d\n", NDIM);
    if (5 != NDIM) {
        wlog("Compile with NDIM=5 for this test\n");
        exit(1);
    }


    wlog("Testing Read PhotoCatalog\n");
    const char pfilename[]="data/photo.dat";
    struct PhotoCatalog* pcat=PhotoCatalogRead(pfilename);

    size_t nprint = 10;
    //size_t nprint = pcat->size;
    wlog("Printing %ld rows from pcat\n", nprint);

    PhotoCatalogWriteSome(stdout, pcat, nprint);

    wlog("Creating KDTree for pcat\n");
    struct KDTree* kd_photo = KDTreeCreate(pcat->pts);


    wlog("Freeing KDTree\n");
    KDTreeFree(kd_photo);

    wlog("Freeing PhotoCatalog\n");
    PhotoCatalogFree(pcat);

    wlog("Testing Read TrainCatalog\n");
    const char tfilename[]="data/train.dat";
    struct TrainCatalog* tcat=TrainCatalogRead(tfilename);

    nprint = 10;
    //nprint = tcat->size;
    wlog("Printing %ld rows from tcat\n", nprint);
    struct KDTree* kd_train = KDTreeCreate(tcat->pts);
    KDTreeFree(kd_train);

    TrainCatalogWriteSome(stdout, tcat, nprint);

    wlog("Freeing TrainCatalog\n");
    TrainCatalogFree(tcat);


}

void test_kdtree_ndim2() {

    wlog("Testing KDTree\n");
    wlog("NDIM = %d\n", NDIM);
    if (2 != NDIM) {
        wlog("Compile with NDIM=2 for this test\n");
        exit(1);
    }

    wlog("    Allocating PointVector 3\n");
    size_t npts=3;
    struct Points* pts = PointsAlloc(npts);

    // note bizarre layout in memory required
    for (int j=0; j<NDIM; j++) {
        for (int i=0; i<npts; i++) {
            //int index = i*NDIM + j;
            int index = i + j*npts;

            pts->data[index] = i+j;
            wlog("pts[%d] = %lf\n", index, pts->data[index]);
        }
    }

    wlog("    Creating KDTree\n");
    struct KDTree* kdtree = KDTreeCreate((const struct Points*)pts);

    wlog("Freeing KDTree\n");
    KDTreeFree(kdtree);
    wlog("Freeing PointVector\n");
    PointsFree(pts);
}


void test_treenode_ndim2() {
    wlog("\nTesting TreeNode\n");
    wlog("NDIM = %d\n", NDIM);
    if (2 != NDIM) {
        wlog("Compile with NDIM=2 for this test\n");
        exit(1);
    }


    double low[NDIM];
    double high[NDIM];

    low[0] = 0.0;
    low[1] = 0.0;

    high[0] = 1.0;
    high[1] = 1.0;

    // the indices here are arbitrary, we are just testing
    // the hcube code
    struct TreeNode* node = TreeNodeAllocWithData(
            low, high, 1, 10, 3, 5, 6);


    double inside[NDIM];
    inside[0] = 0.5;
    inside[1] = 0.5;

    if (TreeNodeContains(node, inside)) {
        wlog("inside point correctly found to be contained\n");
    } else {
        wlog("inside point incorrectly found to be outside\n");
    }

    double outside[NDIM];
    outside[0] = 1.5;
    outside[1] = 1.5;

    if (TreeNodeContains(node, outside)) {
        wlog("outside point incorrectly found to be inside\n");
    } else {
        wlog("outside point correctly found to be outside\n");
    }

    TreeNodeFree(node);


    wlog("Making a TreeNodeVector\n");
    struct TreeNodeVector* node_vector = TreeNodeVectorAlloc(35);
    for (int i=0; i<node_vector->size; i++) {
        node_vector->nodes[i].parent = i+1;
    }
    wlog("  node_vector->size: %ld\n", node_vector->size);
    wlog("  node_vector->nodes[20].parent: %d\n", node_vector->nodes[20].parent);

    TreeNodeVectorFree(node_vector);

}

void test_hcube_ndim2() {
    wlog("\nTesting hypercube\n");
    wlog("NDIM = %d\n", NDIM);
    if (2 != NDIM) {
        wlog("Compile with NDIM=2 for this test\n");
        exit(1);
    }

    double low[NDIM];
    double high[NDIM];

    low[0] = 0.0;
    low[1] = 0.0;

    high[0] = 1.0;
    high[1] = 1.0;

    struct HCube* h = HCubeAllocWithBounds(low, high);

    double inside[NDIM];
    inside[0] = 0.5;
    inside[1] = 0.5;

    if (HCubeContains(h, inside)) {
        wlog("inside point correctly found to be contained\n");
    } else {
        wlog("inside point incorrectly found to be outside\n");
    }

    double outside[NDIM];
    outside[0] = 1.5;
    outside[1] = 1.5;

    if (HCubeContains(h, outside)) {
        wlog("outside point incorrectly found to be inside\n");
    } else {
        wlog("outside point correctly found to be outside\n");
    }

    HCubeFree(h);
}

int main(int argc, char** argv) {
    //test_point();
    if (2 == NDIM) {
        test_hcube_ndim2();
        test_treenode_ndim2();
        test_kdtree_ndim2();
    } else if (5 == NDIM) {
        //test_read_catalog_ndim5();
        test_nnear_ndim5();
    } else {
        wlog("No test for NDIM = %d\n", NDIM);
    }
    return 0;
}
