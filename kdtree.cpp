//
// Created by aash29 on 10/13/18.
//

#include "kdtree.h"
#include <vector>
#include <algorithm>

using namespace std;

template <typename T>
kdNode kdTree(vector<T>::iterator pointsBegin, vector<T>::iterator pointsEnd, int depth) {
    int axis = depth % 2;

    auto cmp = [&](T lhs, T rhs) {
        return lhs(axis)<rhs(axis);
    };

    sort(pointsBegin,pointsEnd,cmp);

    unsigned int medianIndex = distance(pointsBegin, pointsEnd) / 2;

    float median = points[medianIndex];

    kdNode result;

    result.location = median;
    result.left = kdTree<T>()





};