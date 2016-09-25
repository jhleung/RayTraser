#include "cubeMap.h"
#include "ray.h"
#include "../ui/TraceUI.h"
extern TraceUI* traceUI;

glm::dvec3 CubeMap::getColor(ray r) const {
	// YOUR CODE HERE
	// FIXME: Implement Cube Map here
	glm::dvec3 dir = r.d;
  	double maxAxis = std::max(std::max(std::fabs(dir[0]),std::fabs(dir[1]))
  							, std::fabs(dir[2]));

  	double u = 0.0;
  	double v = 0.0;
  	TextureMap* map;

  	if (dir[0]/maxAxis == 1) {
  		u = -dir[2];
  		v = dir[1];
  		map = tMap[0]; 
  	}
  	else if (dir[0]/maxAxis == -1) {
  		u = dir[2];
  		v = dir[1];
  		map = tMap[1];
  	}
  	else if (dir[1]/maxAxis == 1) {
  		u = dir[0];
  		v = -dir[2];
  		map = tMap[2];
  	}
  	else if (dir[1]/maxAxis == -1) {
  		u = dir[0];
  		v = dir[2];
  		map = tMap[3];
  	}
  	else if (dir[2]/maxAxis == 1) {
  		u = dir[0];
  		v = dir[1];
  		map = tMap[4];
  	}
  	else if (dir[2]/maxAxis == -1) {
  		u = -dir[0];
  		v = dir[1];
  		map = tMap[5];
  	}

  	u = 0.5f * (u / maxAxis + 1.0f);
  	v = 0.5f * (v / maxAxis + 1.0f);
  	glm::dvec2 coord = glm::dvec2(u,v);
  	return map->getMappedValue(coord);
}
