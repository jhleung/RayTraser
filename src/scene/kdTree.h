#pragma once

#include <vector>

// Note: you can put kd-tree here
class KDNode {

public:
	BoundingBox bbox; 
	KDNode *left;
	KDNode *right;
	vector <Geometry*> triangles;

public:
	KDNode();

	KDNode* buildKDT(vector<Geometry*> triangles, int depth) const;
	
}