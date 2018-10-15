//
// Created by aash29 on 10/13/18.
//

#ifndef GLMAP_KDTREE_H
#define GLMAP_KDTREE_H

#include <vector>
#include "Box2D/Box2D.h"
#include <algorithm>

using namespace std;

class kdNode {
public:
    float location;
    kdNode* left;
    kdNode* right;
};

template <typename T>
kdNode* kdTree(typename vector<T>::iterator pointsBegin, typename vector<T>::iterator pointsEnd, int depth){
    int axis = depth % 2;

    auto cmp = [&](T lhs, T rhs) {
        return lhs(axis)<rhs(axis);
    };

    sort(pointsBegin,pointsEnd,cmp);

    unsigned int medianIndex = distance(pointsBegin, pointsEnd) / 2;

    float median = (*(pointsBegin+medianIndex))(axis);

    kdNode result;

    result.location = median;
    result.left = kdTree<T>(pointsBegin, pointsBegin+medianIndex, depth+1);
    result.right = kdTree<T>(pointsBegin + medianIndex + 1, pointsEnd, depth+1);

    return &result;

};
#endif //GLMAP_KDTREE_H
