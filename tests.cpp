
#include "kdtree.h"
#include "Box2D/Box2D.h"
#include <iostream>


using namespace std;


void testKDTree(){

    vector<b2Vec2> points = vector<b2Vec2>();
    points.push_back(b2Vec2(1.f,1.f));
    points.push_back(b2Vec2(0.f,5.f));
    points.push_back(b2Vec2(-3.f,18.f));

	kdNode* n1 = kdTree <b2Vec2> (points, 0);
	cout << n1->location;
    //testFun();
}

int main(int argc, char *argv[]) {
    testKDTree();


}