#ifndef _TREENODE_H
#define _TREENODE_H

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dims.h"
#include "HCube.h"


struct TreeNode {

    struct HCube hcube;

	int schild;   // Left node or all values with greater mag in currentdim
	int bchild;   // right node
	int parent;   // parent node 
	int lowind;   // lowest number in array of indicies
	int highind;  // highesst number in array of indices

};

struct TreeNodeVector {
    size_t size;
    struct TreeNode* nodes;
};

struct TreeNode* TreeNodeAlloc();

struct TreeNode* TreeNodeAllocWithData(
        double low[NDIM],
        double high[NDIM],
        int lowind,
        int highind,
        int parent,
        int left,
        int right);

// copy the inputs into the TreeNode structure
void TreeNodeCopyData(
        struct TreeNode* node,
        double low[NDIM],
        double high[NDIM],
        int lowind,
        int highind,
        int parent,
        int left,
        int right);

void TreeNodeFree(struct TreeNode* node);


struct TreeNodeVector* TreeNodeVectorAlloc(size_t n);
void TreeNodeVectorFree(struct TreeNodeVector* node_vector);


double TreeNodeDist(struct TreeNode* node, double p[NDIM]);
double TreeNodeContains(struct TreeNode* node, double p[NDIM]);

#endif
