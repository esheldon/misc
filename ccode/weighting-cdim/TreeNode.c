#include "TreeNode.h"

struct TreeNode* TreeNodeAlloc() {
    struct TreeNode* node = malloc(sizeof(struct TreeNode));
    assert(node != NULL);
    return node;
}

struct TreeNode* TreeNodeAllocWithData(
        double low[NDIM],
        double high[NDIM],
        int lowind,
        int highind,
        int parent,
        int left,
        int right) {

    struct TreeNode* node = TreeNodeAlloc();
    TreeNodeCopyData(node,low,high,lowind,highind,parent,left,right);
    /*
    memcpy(&node->hcube.low, low, NDIM*sizeof(double));
    memcpy(&node->hcube.high, high, NDIM*sizeof(double));

    node->lowind = lowind;
    node->highind = highind;
    node->parent = parent;
    node->schild = left;
    node->bchild = right;
    */
    return node;
}

void TreeNodeCopyData(
        struct TreeNode* node,
        double low[NDIM],
        double high[NDIM],
        int lowind,
        int highind,
        int parent,
        int left,
        int right) {

    assert(node != NULL);

    memcpy(&node->hcube.low, low, NDIM*sizeof(double));
    memcpy(&node->hcube.high, high, NDIM*sizeof(double));

    node->lowind = lowind;
    node->highind = highind;
    node->parent = parent;
    node->schild = left;
    node->bchild = right;

}



void TreeNodeFree(struct TreeNode* node) {
    if (node != NULL) {
        free(node);
    }
}

struct TreeNodeVector* TreeNodeVectorAlloc(size_t n) {
    struct TreeNodeVector* node_vector = malloc(sizeof(struct TreeNodeVector));
    assert(node_vector != NULL);

    node_vector->size = n;
    node_vector->nodes = malloc(n*sizeof(struct TreeNode));
    assert(node_vector->nodes != NULL);
    return node_vector;
}

void TreeNodeVectorFree(struct TreeNodeVector* node_vector) {
    if (node_vector != NULL) {
        if (node_vector->nodes != NULL) {
            free(node_vector->nodes);
        }
        free(node_vector);
    }
}


double TreeNodeDist(struct TreeNode* node, double p[NDIM]) {
    assert(node != NULL);
    return HCubeDist(&node->hcube, p);
}

double TreeNodeContains(struct TreeNode* node, double p[NDIM]) {
    assert(node != NULL);
    return HCubeContains(&node->hcube, p);
}
