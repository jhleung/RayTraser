#pragma once

class BoundingBox {
	
	bool bEmpty;
	bool dirty;
	glm::dvec3 bmin;
	glm::dvec3 bmax;
	double bArea;
	double bVolume;

public:

	BoundingBox() : bEmpty(true) {}
	BoundingBox(glm::dvec3 bMin, glm::dvec3 bMax) : bmin(bMin), bmax(bMax), bEmpty(false), dirty(true) {}

	glm::dvec3 getMin() const { return bmin; }
	glm::dvec3 getMax() const { return bmax; }
	bool isEmpty() { return bEmpty; }

	void setMin(glm::dvec3 bMin) {
		bmin = bMin;
		dirty = true;
		bEmpty = false;
	}
	void setMax(glm::dvec3 bMax) {
		bmax = bMax;
		dirty = true;
		bEmpty = false;
	}
	void setMin(int i, double val) {
		if (i == 0) { bmin[0] = val; bEmpty = false; }
		else if (i == 1) { bmin[1] = val; bEmpty = false; }
			else if (i == 2) { bmin[2] = val; bEmpty = false; }
	}
	void setMax(int i, double val) {
		if (i == 0) { bmax[0] = val; bEmpty = false; }
		else if (i == 1) { bmax[1] = val; bEmpty = false; }
			else if (i == 2) { bmax[2] = val; bEmpty = false; }
	}
	void setEmpty() {
		bEmpty = true;
	}

	// Does this bounding box intersect the target?
		bool intersects(const BoundingBox &target) const {
			return ((target.getMin()[0] - RAY_EPSILON <= bmax[0]) && (target.getMax()[0] + RAY_EPSILON >= bmin[0]) &&
				(target.getMin()[1] - RAY_EPSILON <= bmax[1]) && (target.getMax()[1] + RAY_EPSILON >= bmin[1]) &&
				(target.getMin()[2] - RAY_EPSILON <= bmax[2]) && (target.getMax()[2] + RAY_EPSILON >= bmin[2]));
	}

	// does the box contain this point?
	bool intersects(const glm::dvec3& point) const {
		return ((point[0] + RAY_EPSILON >= bmin[0]) && (point[1] + RAY_EPSILON >= bmin[1]) && (point[2] + RAY_EPSILON >= bmin[2]) &&
			(point[0] - RAY_EPSILON <= bmax[0]) && (point[1] - RAY_EPSILON <= bmax[1]) && (point[2] - RAY_EPSILON <= bmax[2]));
	}

	// if the ray hits the box, put the "t" value of the intersection
	// closest to the origin in tMin and the "t" value of the far intersection
	// in tMax and return true, else return false.
	// Using Kay/Kajiya algorithm.
	bool intersect(const ray& r, double& tMin, double& tMax) const {
		glm::dvec3 R0 = r.getPosition();
		glm::dvec3 Rd = r.getDirection();
		tMin = -1.0e308; // 1.0e308 is close to infinity... close enough for us!
		tMax = 1.0e308;
		double ttemp;
	
		for (int currentaxis = 0; currentaxis < 3; currentaxis++) {
			double vd = Rd[currentaxis];
			// if the ray is parallel to the face's plane (=0.0)
			if( vd == 0.0 ) continue;
			double v1 = bmin[currentaxis] - R0[currentaxis];
			double v2 = bmax[currentaxis] - R0[currentaxis];
			// two slab intersections
			double t1 = v1/vd;
			double t2 = v2/vd;
			if ( t1 > t2 ) { // swap t1 & t2
				ttemp = t1;
				t1 = t2;
				t2 = ttemp;
			}
			if (t1 > tMin) tMin = t1;
			if (t2 < tMax) tMax = t2;
			if (tMin > tMax) return false; // box is missed
			if (tMax < RAY_EPSILON) return false; // box is behind ray
		}
		 return true; // it made it past all 3 axes.
	}

	void operator=(const BoundingBox& target) {
		bmin = target.bmin;
		bmax = target.bmax;
		bArea = target.bArea;
		bVolume = target.bVolume;
		dirty = target.dirty;
		bEmpty = target.bEmpty;
	}

	double area() {
		if (bEmpty) return 0.0;
		else if (dirty) {
			bArea = 2.0 * ((bmax[0] - bmin[0]) * (bmax[1] - bmin[1]) + (bmax[1] - bmin[1]) * (bmax[2] - bmin[2]) + (bmax[2] - bmin[2]) * (bmax[0] - bmin[0]));
			dirty = false;
		}
		return bArea;
	}

	double volume() {
		if (bEmpty) return 0.0;
		else if (dirty) {
			bVolume = ((bmax[0] - bmin[0]) * (bmax[1] - bmin[1]) * (bmax[2] - bmin[2]));
			dirty = false;
		}
		return bVolume;
	}

	glm::dvec3 midpoint() {
		return glm::dvec3(bmax[0]+bmin[0], bmax[1]+bmin[1],bmax[2]+bmin[2])/2.0;
	}
	void merge(const BoundingBox& bBox)	{
		if (bBox.bEmpty) return;
		for (int axis = 0; axis < 3; axis++) {
			if (bEmpty || bBox.bmin[axis] < bmin[axis]) bmin[axis] = bBox.bmin[axis];
			if (bEmpty || bBox.bmax[axis] > bmax[axis]) bmax[axis] = bBox.bmax[axis];
		}
		dirty = true;
		bEmpty = false;
	}
};
