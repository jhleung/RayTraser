#include <cmath>

#include "light.h"
#include <glm/glm.hpp>


using namespace std;

double DirectionalLight::distanceAttenuation(const glm::dvec3& P) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


glm::dvec3 DirectionalLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const
{
//	YOUR CODE HERE:
//	You should implement shadow-handling code here.

    Scene* scene = getScene();
	ray shadow = r;
	isect i;
	glm::dvec3 atten;
	if (scene->intersect(shadow,i)) {
		// assert(i.t <= 0);
        atten = glm::dvec3(0.0,0.0,0.0);
	}
	else {
		atten = glm::dvec3(1.0,1.0,1.0);
	}
	return atten;
}

glm::dvec3 DirectionalLight::getColor() const
{
	return color;
}

glm::dvec3 DirectionalLight::getDirection(const glm::dvec3& P) const
{
	return -orientation;
}

double PointLight::distanceAttenuation(const glm::dvec3& P) const
{

	// YOUR CODE HERE
	float d = glm::distance(P,position);//sqrt(pow(P[0]-position[0],2) + pow(P[1]-position[1],2) + pow(P[2]-position[2],2));

	return min(1.0, 1.0 / (constantTerm + linearTerm * d + quadraticTerm * d * d));
	// You'll need to modify this method to attenuate the intensity 
	// of the light based on the distance between the source and the 
	// point P.  For now, we assume no attenuation and just return 1.0
	//return 1.0;
}

glm::dvec3 PointLight::getColor() const
{
	return color;
}

glm::dvec3 PointLight::getDirection(const glm::dvec3& P) const
{
	return glm::normalize(position - P);
}


glm::dvec3 PointLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const
{
    // YOUR CODE HERE:
    // You should implement shadow-handling code here.
    Scene* scene = getScene();
    ray shadow = r;
    isect i;
    glm::dvec3 atten;

    if (scene->intersect(shadow,i) && i.t < glm::distance(p, position)) {
    	atten = glm::dvec3(0.0,0.0,0.0);
    }
    else {
        atten = glm::dvec3(1.0,1.0,1.0);
    }
    return atten;
}

