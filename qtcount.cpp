#include "qtcount.h"

bool qtcount::prunable(Node * root, int tol) {
    return prunable(root, tol, root->avg);
}

bool qtcount::prunable(Node * node, int tol, RGBAPixel & avg) {
    if (node == NULL) return true;
    if (isLeaf(node) && within(node->avg, avg, tol)) {
        return true;
    } else if (isLeaf(node)) {
        return false;
    } else {
        return prunable(node->NE, tol, avg)
            && prunable(node->NW, tol, avg)
            && prunable(node->SE, tol, avg)
            && prunable(node->SW, tol, avg);
    }
}

bool qtcount::within(RGBAPixel & p1, RGBAPixel & p2, int tol) {
    return pow(p1.r-p2.r, 2) + pow(p1.g-p2.g, 2) + pow(p1.b-p2.b, 2) <= tol;
}