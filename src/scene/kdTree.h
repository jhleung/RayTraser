#pragma once

#include <vector>

#include "scene.h"

// Note: you can put kd-tree here
class KDNode {

public:
	BoundingBox bbox; 
	KDNode *left;
	KDNode *right;
	std::vector <Geometry*> triangles;

public:
	KDNode();

	KDNode* buildKDT(std::vector<Geometry*> triangles, int depth) const;

};