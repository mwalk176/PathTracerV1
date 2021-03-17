#ifndef OBJECT_H
#define OBJECT_H

#include "Vec3f.h"
#include <string>

class Object {
public:
	Vec3f pos = Vec3f();
	Vec3f color = Vec3f(1);
	Vec3f emissionColor = Vec3f(0);
	float refl = 0;
	float refract = 0;
	float IOR = 1.5;
	float gammaBoost = 1.0;
	int type = 0; //0 diffuse, 1 pure specular, 2 glass

	virtual bool intersect(Vec3f rayOrigin, Vec3f rayDirection, float& p0, float& p1) = 0;
	virtual Vec3f getNewDirectionTowardsLight(Vec3f shadowRay, Vec3f alignedNormal, Vec3f normalFromLight, float& angleToObject, Vec3f intersectionPoint) = 0;
	virtual Vec3f computeNormal(Vec3f intersectionPoint) = 0;
	virtual std::string toString() = 0;
	

};



#endif

