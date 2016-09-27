#pragma once

#include <vector>

#include "scene.h"
#include "ray.h"
// Note: you can put kd-tree here
class Geometry;

class KDNode {

public:
	BoundingBox bbox; 
	KDNode* left;
	KDNode* right;
	std::vector<Geometry*> triangles;

public:
	// KDNode();

	static KDNode* buildKDT(std::vector<Geometry*> triangles, int depth);
	bool kdIntersect(KDNode* node, ray& ray, isect i);
};