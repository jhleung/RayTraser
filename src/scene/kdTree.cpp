#include "kdTree.h"

using namespace std;

KDNode* KDNode::buildKDT(std::vector<Geometry*> triangles, int depth) {
	KDNode* node = new KDNode();
	node->triangles = triangles;
	node->left = NULL;
	node->right = NULL;
	node->bbox = BoundingBox();

	if (triangles.size() == 0) {
		return node;
	}

	if (triangles.size() == 1) {
		node->bbox = triangles[0]->getBoundingBox();
		node->left = new KDNode();
		node->right = new KDNode();
		node->left->triangles = vector<Geometry*>();
		node->right->triangles = vector<Geometry*>();
		return node;
	}

	node->bbox = triangles[0]->getBoundingBox();
	for (int i = 1; i < triangles.size(); i++) {
		node->bbox.merge(triangles[i]->getBoundingBox());
	}

	glm::dvec3 midpoint;
	for (int i = 0; i < triangles.size(); i++) {
		BoundingBox bboxCurr = triangles[i]->getBoundingBox();
		midpoint += bboxCurr.midpoint() * (1.0 / triangles.size());
	}

	vector<Geometry*> left_triangles;
	vector<Geometry*> right_triangles;
	glm::dvec3 diagonal = node->bbox.getMax() - node->bbox.getMin();
	int axis;
	if (diagonal[0] >= diagonal[1] && diagonal[0] > diagonal[2])
		axis = 0;
	else if (diagonal[1] >= diagonal[0] && diagonal[1] > diagonal[2])
		axis = 1;
	else
		axis = 2;
	// int axis = 	std::max(std::max(std::fabs(node->bbox.getMax()[0]),std::fabs(node->bbox.getMax()[1]))
  							// , std::fabs(node->bbox.getMax()[2]));
	for (int i = 0; i < triangles.size(); i++) {

		BoundingBox bboxCurr = triangles[i]->getBoundingBox();
		switch (axis) {
			case 0:
				if (node->bbox.midpoint()[0] >= bboxCurr.midpoint()[0])
					right_triangles.push_back(triangles[i]);
				else left_triangles.push_back(triangles[i]);
				break;
			case 1:
				if (node->bbox.midpoint()[1] >= bboxCurr.midpoint()[1])
					right_triangles.push_back(triangles[i]);
				else left_triangles.push_back(triangles[i]);
				break;
			case 2:
				if (node->bbox.midpoint()[2] >= bboxCurr.midpoint()[2])
					right_triangles.push_back(triangles[i]);
				else left_triangles.push_back(triangles[i]);
				break;		
		}
	}

	if (left_triangles.size() == 0 && right_triangles.size() > 0) left_triangles = right_triangles;
	if (right_triangles.size() == 0 && left_triangles.size() > 0 ) right_triangles = left_triangles;

	// if (depth <= 0) {
	// 	node->left = buildKDT(left_triangles, depth-1);
	// 	node->right = buildKDT(right_triangles, depth-1);
	// }
	// else {
	// 	node->left = new KDNode();
	// 	node->right = new KDNode();
	// 	node->left->triangles = vector<Geometry*>();
	// 	node->right->triangles = vector<Geometry*>();
	// }

	int numMatches = 0;
	for (int i = 0; i < left_triangles.size(); i++) {
		for (int j = 0; j < right_triangles.size(); j++) {
			if (left_triangles[i] == right_triangles[i])
				numMatches++;
		}
	}

	if ((double)numMatches / left_triangles.size() < 0.5 && (double)numMatches / right_triangles.size() < 0.5) {
		node->left = buildKDT(left_triangles, depth+1);
		node->right = buildKDT(right_triangles, depth+1);
	}
	else {
		node->left = new KDNode();
		node->right = new KDNode();
		node->left->triangles = vector<Geometry*>();
		node->right->triangles = vector<Geometry*>();
	}

	return node;
}

bool KDNode::kdIntersect(KDNode* node, ray& r, isect i) {
	double tmin = 0.0;
	double tmax = 0.0;
	if (node->bbox.intersect(r,tmin,tmax)) {
		glm::dvec3 intersection_point, local_intersection_point;
		if (node->left->triangles.size() > 0 || node->right->triangles.size() > 0) {
			bool lefthit = KDNode::kdIntersect(node->left, r, i);
			bool righthit = KDNode::kdIntersect(node->right, r, i);
			return lefthit || righthit;
		}
		else {
			bool have_one = false;
			typedef vector<Geometry*>::const_iterator iter;
			for(iter j = node->triangles.begin(); j != node->triangles.end(); ++j) {
				isect cur;
				if( (*j)->intersect(r, cur) ) {
					if(!have_one || (cur.t < i.t)) {
						i = cur;
						have_one = true;
					}
				}
			}
			if(!have_one) i.setT(1000.0);
	// if debugging,
			//if (TraceUI::m_debug) intersectCache.push_back(std::make_pair(new ray(r), new isect(i)));
			return have_one;
		}
	}
	return false;
	
	
	


}