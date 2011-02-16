#ifndef _KDTREE_H
#define _KDTREE_H

#include "params.h"
#include "TreeNode.h"
#include "Points.h"
#include "Vector.h"


struct KDTree {
    // these are the nodes of the tree.  Free this.
    struct TreeNodeVector* nvec;


    // this will point to existing data: do not free!
    const struct Points* pts;
    size_t npts;

    // the index array, same size as pts.  Free this.
    int* ptind;

    // reverse indices (sort of).  Same length as pvec.
    // connects ptind to index j.
    // revind[ptind[j]] = j
    // Free this.
	int *revind;
};


// create the KDTree from the input points. The points
// are not copied or altered in any way.
struct KDTree* KDTreeCreate(const struct Points* pts);


// This is the workhorse that actually builds the tree.
// this should not be called directly
void _KDTreeBuildTree(struct KDTree* kdtree);
// calculate the number of boxes needed
int _KDTreeGetBoxnum(int npts);

void _KDTreeMakeBounds(double low[NDIM], double high[NDIM]);


// Find neighbors within radius r and return their indices.
// dist is a pre-allocated array of fixed size.  The MAX_RADMATCH
// is set in params.h

int KDTreePointRadMatch(
        struct KDTree* kdtree, 
        double point[NDIM], 
        double radius, 
        struct I4Vector* indices);



//
// find the n nearest neighbors to the point.
//
// The distances are returned in the dist array, which is sorted ds[0] biggest
//
// indices holds the indices of the neighbors
//
// These arrays should be allocated before calling this function
//

void KDTreeNNeighOfPoint(
        struct KDTree* kdtree, 
        int nnear,
        double point[NDIM],
        double* dist, 
        int* indices);


//
// find the n nearest neighbors to the point int the tree specifed by the
// integer "index"
//
// The distances are returned in the dist array, which is sorted ds[0] biggest
//
// indices holds the indices of the neighbors
//
// These arrays should *not* be allocated before calling this function
//

void KDTreeNNeighOfIndex(
        struct KDTree* kdtree, 
        int nnear,
        int index,
        double* dist, 
        int* indices);

//
// distance between the node hcube and the point indicated by the index
//
double KDTreeDistNodeIndex(
        struct KDTree* kdtree, 
        int node_index, int point_index);

//
// distance between the node hcube and the point
//
double KDTreeDistNodePoint(
        struct KDTree* kdtree, 
        int node_index, double point[NDIM]);

//
// distance between the two points indicated by the indices
//
double KDTreeDistIndices(struct KDTree* kdtree, int i1, int i2);

//
// check the point indicated by point_index is equal to the input point
//
int KDTreePointsEqual(struct KDTree* kdtree, int point_index, double point[NDIM]);

//
// get distance between point indicated by point_index and the input point
//
double KDTreePointsDist(struct KDTree* kdtree, int point_index, double point[NDIM]);

//
// same as above but returns large number if equal
//
double KDTreePointsDistDeflarge(
        struct KDTree* kdtree, int point_index, double point[NDIM]);

//
// distance between the two points indicated by the indices If any of the
// elements differs, return diff, otherwise a large number
//
double KDTreeDistIndicesDeflarge(struct KDTree* kdtree, int i1, int i2);


//
// returns the index of the cube containing the point
//
int KDTreeFindCube(struct KDTree* kdtree, double point[NDIM]);
//
// find the specified box
//
int KDTreeFindBox(struct KDTree* kdtree, int p);

// copy out the point indicated by point_index
void KDTreeCopyPoint(struct KDTree* kdtree, int point_index, double out[NDIM]);

void KDTreeFree(struct KDTree* kdtree);

#endif
