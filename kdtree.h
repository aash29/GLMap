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
	vector<b2Vec2> contains;
};

template <typename T>
kdNode* kdTree(vector<T> points, int depth){

	int maxDepth = 50;

	int d1 = depth / 2;
	int axis = depth - d1 * 2;

	//int len1 = distance(pointsBegin, pointsEnd);
	int len1 = points.size();

	if (len1 == 1) {
		kdNode* result = new kdNode();

		result->location = points[0](axis);
		result->left = NULL;
		result->right = NULL;
		return result;
	};



	if (depth > maxDepth) {
		kdNode* result = new kdNode();

		result->location = points[0](axis);
		result->left = NULL;
		result->right = NULL;

		result->contains = points;

		return result;

	};

    auto cmp = [&](T lhs, T rhs) {
        return lhs(axis)<rhs(axis);
    };

    //sort(pointsBegin,pointsEnd,cmp);
	sort(points.begin(),points.end(), cmp);

	if (len1 == 2) {
		float median = (points[0](axis) + points[1](axis)) / 2;
		vector<T> h1(1,points[0]);
		vector<T> h2(1,points[1]);

		kdNode* result = new kdNode();

		result->location = median;
		result->left = kdTree<T>(h1, depth + 1);
		result->right = kdTree<T>(h2, depth + 1);


		return result;
	} 
	else {

		//unsigned int medianIndex = distance(pointsBegin, pointsEnd) / 2;
		unsigned int medianIndex = distance(points.begin(), points.end()) / 2;

		//float median = (*(pointsBegin+medianIndex))(axis);

		float median = points[medianIndex](axis);

		vector<T> h1(points.begin(), points.begin() + medianIndex + 1);
		vector<T> h2(points.begin() + medianIndex + 1, points.end());

		kdNode* result = new kdNode();

		result->location = median;
		result->left = kdTree<T>(h1, depth + 1);
		result->right = kdTree<T>(h2, depth + 1);

		return result;
	}




};
#endif //GLMAP_KDTREE_H
