#include "scene.h"
#include "kdTree.h"

using namespace std;

// KDNode* buildKDT(std::vector<Geometry*> triangles, int depth) {
// 	KDNode* node = new KDNode();
// 	node->triangles = triangles;
// 	node->left = NULL;
// 	node->right = NULL;
// 	node->bbox = BoundingBox();

// 	node->bbox = triangles[0]->getBoundingBox();
// 	for (int i = 1; i < triangles.size(); i++) {
// 		node->bbox.merge(triangles[i]->getBoundingBox());
// 	}



// }