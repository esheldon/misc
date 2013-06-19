#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "params.h"
#include "KDTree.h"
#include "partition.h"
#include "heap.h"
#include "util.h"


struct KDTree* KDTreeCreate(const struct Points* pts) {

    wlog("  KDTreeCreate, NDIM=%d\n", NDIM);
    assert(pts != NULL);
    assert(pts->size > 0);

    struct KDTree* kdtree = malloc(sizeof(struct KDTree));
    assert(kdtree != NULL);

    // This points to the input data, do not free!
    kdtree->pts = pts;
    int npts = pts->npts;
    kdtree->npts = npts;

    //
    // The following arrays are owned by the kd tree.  These must be freed.
    //
    kdtree->ptind = malloc(npts*sizeof(int));
    assert(kdtree->ptind != NULL);
    for (size_t i=0; i < npts; i++) {
        kdtree->ptind[i] = i;
    }
    kdtree->revind = malloc(npts*sizeof(int));
    assert(kdtree->revind != NULL);

    // Build the tree
    _KDTreeBuildTree(kdtree);

    return kdtree;

}


void _KDTreeBuildTree(struct KDTree* kdtree) {

    int npts = kdtree->npts;
    int boxnum = _KDTreeGetBoxnum(npts);

    kdtree->nvec = TreeNodeVectorAlloc(boxnum);

    // no copy made!
    const double* pts = kdtree->pts->data;

    // low and high start as overall bounds
    //struct Point low,high;
    double low[NDIM],high[NDIM];
    _KDTreeMakeBounds(low,high);

    // temporary pointers to the nodes and ptind
    struct TreeNode* nodes = kdtree->nvec->nodes;
    int* ptind = kdtree->ptind;

    // set the first node from the overall bounds.
    TreeNodeCopyData(&nodes[0],low,high,0,npts-1,0,0,0);

    // Now the main loop building the tree
    int parents[MAXTEMP],dims[MAXTEMP];
    parents[1] = 0;
    dims[1] = 0;
    int cur = 1;
    int boxind = 0;

    // task loop vars
    int tparent, tdim, curlo, curhi, np, k;
    int* indp;
    const double* coordp;

    while(cur) {
        tparent = parents[cur];
        tdim = dims[cur];
        cur--;

        curlo = nodes[tparent].lowind;
        curhi = nodes[tparent].highind;

        indp   = &ptind[curlo];

        // pts is laid out so the entire dimension is contiguous.  so this
        // points to the first point in that dimension
        coordp = &pts[tdim*npts];

        np	 = curhi- curlo + 1; //points 
        k = (np-1)/2; 

        partition(k, coordp, indp, np);

        memcpy(low,  nodes[tparent].hcube.low,  NDIM*sizeof(double));
        memcpy(high, nodes[tparent].hcube.high, NDIM*sizeof(double));

        low[tdim] = pts[tdim*npts + indp[k]];
        high[tdim] = pts[tdim*npts + indp[k]];

        // Creates the smallest daughter box
        boxind++;
        TreeNodeCopyData(&nodes[boxind],
                         nodes[tparent].hcube.low,
                         high,
                         curlo,
                         curlo+k,
                         tparent,
                         0, 0);
        // Creates the larger daughter box
        boxind++;
        TreeNodeCopyData(&nodes[boxind],
                         low,
                         nodes[tparent].hcube.high,
                         curlo+k+1,
                         curhi,
                         tparent,
                         0, 0);

        // sets the children
        nodes[tparent].schild = boxind - 1;
        nodes[tparent].bchild = boxind;

        //if left box needs to be subdivided
        if(k > 1) {
            cur++;
            parents[cur] = boxind-1;
            // Increments the dimension. sets back to 0 if tdim = dim
            dims[cur] = (tdim+1) % NDIM; 
        }
        //if right box needs subdivisions
        if(np-k > 3) {
            cur++;
            parents[cur] = boxind;
            dims[cur] = (tdim+1) % NDIM; 
        }

    }

    for (size_t i=0; i < kdtree->npts; i++) {
        kdtree->revind[kdtree->ptind[i]] = i;
    }
}

int _KDTreeGetBoxnum(int npts) {
    int boxnum, m;

    //Calculating the number of boxes needed
    m = 1;
    while(m < npts) {
        m*= 2;
    }
    boxnum = m -1;
    if(2*npts - m/2 -1 < boxnum) {
        boxnum= 2*npts - m/2 -1;
    }

    return boxnum;
	
}


// make the overall bounds
void _KDTreeMakeBounds(double low[NDIM], double high[NDIM]) {
    for (int i=0; i<NDIM; i++) {
        low[i]  = -BIGVAL;
        high[i] =  BIGVAL;
    }
}

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
        int* indices) {

    if (nnear > (kdtree->npts-1)) {
        wlog("Not enough points, need at least %d\n", nnear);
        exit(1);
    }

    for (int i=0; i<nnear; i++) {
        indices[i] = 0;
        dist[i] = BIGVAL*NDIM;
    }

    // temporary pointers
    struct TreeNode* nodes = kdtree->nvec->nodes;
    int* ptind = kdtree->ptind;

    double dcur=0;
    // get the box holding the point
    int boxi = nodes[KDTreeFindCube(kdtree,point)].parent;

    while( nodes[boxi].highind - nodes[boxi].lowind < nnear ) {
        boxi = nodes[boxi].parent;
    }

    for(int i = nodes[boxi].lowind; i <= nodes[boxi].highind; i++) {
        if ( KDTreePointsEqual(kdtree, ptind[i], point) ) continue;

        dcur = KDTreePointsDistDeflarge(kdtree,ptind[i],point);
        if(dcur < dist[0]) {
            dist[0] = dcur;
            indices[0] = ptind[i];
            if (nnear > 1) {
                fixheap(dist,indices,nnear);
            }
        }
    }

    int cur = 1;
    int task[100];
    task[1] = 0;
    int curbox;

    while(cur)//cur > 0
    {
        curbox = task[cur];
        cur--;
        if(boxi == curbox) continue;

        double dtmp = KDTreeDistNodePoint(kdtree, curbox, point);
        if ( dtmp  < dist[0] ) {

            if(nodes[curbox].schild > 0) {	
                cur++;
                task[cur] = nodes[curbox].schild;
                cur++;
                task[cur] = nodes[curbox].bchild;
            } else {
                for(int i = nodes[curbox].lowind; i <= nodes[curbox].highind; i++) {
                    dcur = KDTreePointsDistDeflarge(kdtree,ptind[i],point);
                    if(dcur < dist[0]) {
                        dist[0] = dcur;
                        indices[0] = ptind[i];
                        if(nnear > 1) fixheap(dist,indices,nnear);
                    }
                }
            }
        }
    }

}


//
// find the n nearest neighbors to the point int the tree specifed by the
// integer "index"
//
// The distances are returned in the dist array, which is sorted ds[0] biggest
//
// indices holds the indices of the neighbors
//
// These arrays should be allocated before calling this function
//

void KDTreeNNeighOfIndex(
        struct KDTree* kdtree, 
        int nnear,
        int index,
        double* dist, 
        int* indices) {

    if (nnear > (kdtree->npts-1)) {
        wlog("Not enough points, need at least %d\n", nnear);
        exit(1);
    }

    for (int i=0; i<nnear; i++) {
        indices[i] = 0;
        dist[i] = BIGVAL*NDIM;
    }

    // temporary pointers
    struct TreeNode* nodes = kdtree->nvec->nodes;
    int* ptind = kdtree->ptind;

    double dcur=0;
    // get the box holding the point
    int boxi = nodes[KDTreeFindBox(kdtree,index)].parent;

    while( nodes[boxi].highind - nodes[boxi].lowind < nnear ) {
        boxi = nodes[boxi].parent;
    }

    for(int i = nodes[boxi].lowind; i <= nodes[boxi].highind; i++) {
        if(index == ptind[i]) continue;
        dcur = KDTreeDistIndicesDeflarge(kdtree,index,ptind[i]);
        if(dcur < dist[0]) {
            dist[0] = dcur;
            indices[0] = ptind[i];
            if (nnear > 0) {
                fixheap(dist,indices,nnear);
            }
        }
    }

    int cur = 1;
    int task[100];
    task[1] = 0;
    int curbox;

    while(cur)//cur > 0
    {
        curbox = task[cur];
        cur--;
        if(boxi == curbox) continue;

        double dtmp = KDTreeDistNodeIndex(kdtree, curbox, index);
        if ( dtmp  < dist[0] ) {

            if(nodes[curbox].schild > 0) {	
                cur++;
                task[cur] = nodes[curbox].schild;
                cur++;
                task[cur] = nodes[curbox].bchild;
            } else {
                for(int i = nodes[curbox].lowind; i <= nodes[curbox].highind; i++) {
                    dcur = KDTreeDistIndicesDeflarge(kdtree,ptind[i],index);
                    if(dcur < dist[0]) {
                        dist[0] = dcur;
                        indices[0] = ptind[i];
                        if(nnear > 1) fixheap(dist,indices,nnear);
                    }
                }
            }
        }
    }

}


// Find neighbors within radius r and return their indices.
// dist is a pre-allocated array of fixed size.  The MAX_RADMATCH
// is set in params.h
int KDTreePointRadMatch(
        struct KDTree* kdtree, 
        double point[NDIM], 
        double radius, 
        struct I4Vector* indices) {

    int nmatch=0;

    int box = 0;
    int curdim = 0;
    int oldbox,dau1,dau2;

    // temporary pointers
    struct TreeNode* nodes = kdtree->nvec->nodes;
    int* ptind = kdtree->ptind;

    while(nodes[box].schild > 0) {
        oldbox = box;
        dau1 = nodes[box].schild;
        dau2 = nodes[box].bchild;
        if(point[curdim] + radius <= nodes[dau1].hcube.high[curdim]) {
            box = dau1;
        } else if (point[curdim] - radius >= nodes[dau2].hcube.low[curdim]) {
            box = dau2;
        }
        curdim++;
        curdim % NDIM;
        if(box == oldbox) 
            break;
    }

    int task[100];
    int cur = 1;
    task[1] = box;
    int curbox;
    while(cur)
    {
        box = task[cur];
        cur--;
        if(nodes[box].schild != 0) {
            if( KDTreeDistNodePoint(kdtree, nodes[box].schild, point) <= radius ) {
                cur++;
                task[cur] = nodes[box].schild;
            }
            if( KDTreeDistNodePoint(kdtree, nodes[box].bchild, point) <= radius ) {
                cur++;
                task[cur] = nodes[box].bchild;
            }
        } else {
            for(int j= nodes[box].lowind; j <= nodes[box].highind; j++) {
                if ( KDTreePointsDist(kdtree,ptind[j],point) <= radius) {
                    if (nmatch >= indices->size) {
                        wlog("\nNumber of matches too large: %d\n", nmatch);
                        wlog("Re-allocating to %d\n", indices->size*2);
                        indices = I4VectorRealloc(indices, indices->size*2);
                    }
                    indices->data[nmatch] = ptind[j];
                    nmatch++;
                }
            }
        }
    }

    return nmatch;


}



// distance between the node hcube and the point indicated by the index
double KDTreeDistNodeIndex(
        struct KDTree* kdtree, 
        int node_index, int point_index) {

    struct HCube* h = &kdtree->nvec->nodes[node_index].hcube;

    double sum=0, diff=0, pdata, hlow, hhigh;

    for(int dim=0; dim < NDIM; dim++) {
        pdata = kdtree->pts->data[point_index + dim*kdtree->npts];

        hlow = h->low[dim];
        hhigh = h->high[dim];

        if (pdata < hlow) {
            diff = pdata - hlow;
            sum += diff*diff;
        } else if (pdata > hhigh) {
            diff = pdata - hhigh;
            sum += diff*diff;
        }
    }
    return sqrt(sum);
}

// distance between the node hcube and the point
double KDTreeDistNodePoint(
        struct KDTree* kdtree, 
        int node_index, double point[NDIM]) {

    struct HCube* h = &kdtree->nvec->nodes[node_index].hcube;

    double sum=0, diff=0, pdata, hlow, hhigh;

    for(int dim=0; dim < NDIM; dim++) {
        pdata = point[dim];

        hlow = h->low[dim];
        hhigh = h->high[dim];

        if (pdata < hlow) {
            diff = pdata - hlow;
            sum += diff*diff;
        } else if (pdata > hhigh) {
            diff = pdata - hhigh;
            sum += diff*diff;
        }

    }
    return sqrt(sum);
}




// distance between the two points indicated by the indices
double KDTreeDistIndices(struct KDTree* kdtree, int i1, int i2) {
    double val1,val2,diff;
    double sum=0;
    for (int dim=0; dim<NDIM; dim++) {
        val1 = kdtree->pts->data[i1 + dim*kdtree->npts];
        val2 = kdtree->pts->data[i2 + dim*kdtree->npts];

        diff = val1-val2;
        sum += diff*diff;
    }
    return sqrt(sum);

}

// distance between the two points indicated by the indices If any of the
// elements differs, return diff, otherwise a large number
double KDTreeDistIndicesDeflarge(struct KDTree* kdtree, int i1, int i2) {
    double val1,val2;
    for (int dim=0; dim<NDIM; dim++) {
        val1 = kdtree->pts->data[i1 + dim*kdtree->npts];
        val2 = kdtree->pts->data[i2 + dim*kdtree->npts];
        if (val1 != val2) {
            return KDTreeDistIndices(kdtree, i1, i2);
        }
    }
    // if we get here, return a big number
    return BIGVAL*NDIM;
}



int KDTreePointsEqual(struct KDTree* kdtree, int point_index, double point[NDIM]) {
    double val;
    for (int dim=0; dim<NDIM; dim++) {
        val = kdtree->pts->data[point_index + dim*kdtree->npts];
        if (val != point[dim]) {
            return 0;
        }
    }
    return 1;
}

// distance between the point indicated by the point_index and the
// input point array
double KDTreePointsDist(struct KDTree* kdtree, int point_index, double point[NDIM]) {
    double sum=0, val, diff;
    for (int dim=0; dim<NDIM; dim++) {
        val = kdtree->pts->data[point_index + dim*kdtree->npts];
        diff = val - point[dim];
        sum += diff*diff;
    }
    return sqrt(sum);
}
// same as above but returns large number if equal
double KDTreePointsDistDeflarge(
        struct KDTree* kdtree, int point_index, double point[NDIM]) {
    double val;
    for (int dim=0; dim<NDIM; dim++) {
        val = kdtree->pts->data[point_index + dim*kdtree->npts];
        if (val != point[dim]) {
            return KDTreePointsDist(kdtree, point_index, point);
        }
    }
    // if we get here, return a big number
    return BIGVAL*NDIM;

}



// returns the index of the cube containig Point p
int KDTreeFindCube(struct KDTree* kdtree, double point[NDIM]) {

    // temporary pointer to nodes
    struct TreeNode* nodes = kdtree->nvec->nodes;

    int num = 0;
    int curdim = 0;
    int ldau;
    while(nodes[num].schild != 0) //if the node isn't a leaf
    {
        ldau = nodes[num].schild;
        if(point[curdim] <= nodes[ldau].hcube.high[curdim])
            num = ldau;
        else
            num = nodes[num].bchild;
        curdim = (curdim+1) % NDIM;
    }

    return num;

}

int KDTreeFindBox(struct KDTree* kdtree, int p)
{

    // temporary pointers
    struct TreeNode* nodes = kdtree->nvec->nodes;
	int* revind = kdtree->revind;

    int num = 0;
    int ind = revind[p];
    int ldau;
    int curdim = 0;
    while(nodes[num].schild > 0)
    {
        ldau = nodes[num].schild;
        if(ind <= nodes[ldau].highind)
            num = ldau;
        else 
            num = nodes[num].bchild;
        curdim = (curdim+1) % NDIM;
    }
    return num;
}


void KDTreeCopyPoint(struct KDTree* kdtree, int point_index, double out[NDIM]) {
    for (int dim=0; dim<NDIM; dim++) {
        out[dim] = kdtree->pts->data[point_index + dim*kdtree->npts];
    }
}

void KDTreeFree(struct KDTree* kdtree) {
    if (kdtree != NULL) {

        if (kdtree->nvec != NULL) {
            TreeNodeVectorFree(kdtree->nvec);
        }
        if (kdtree->ptind != NULL) {
            free(kdtree->ptind);
        }
        if (kdtree->revind != NULL) {
            free(kdtree->revind);
        }

        free(kdtree);
    }
}
