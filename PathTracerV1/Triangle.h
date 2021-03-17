#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Vec3f.h"
#include "Object.h"
#include <iostream>

class Triangle : public Object {
public:
	Vec3f v0, v1, v2;
	float D = 0;
	Vec3f normal;
	bool preComputedNormal = false;

	Vec3f e0;
	Vec3f e1;
	Vec3f e2;
	Vec3f eIn0;
	Vec3f eIn1;
	Vec3f eIn2;
	bool preComputedEdges = false;
	
	Triangle() : v0(0), v1(0), v2(0) { computeNormal(Vec3f(0)); }
	Triangle(Vec3f p0, Vec3f p1, Vec3f p2, Vec3f inColor, Vec3f inEmission) {
		v0 = p0;
		v1 = p1;
		v2 = p2;
		pos = (v0 + v1 + v2) / 3.0;
		color = inColor;
		emissionColor = inEmission;
		computeNormal(Vec3f(0));
		std::cout << "Triangle normal: " << normal << "\n";
	}


	Vec3f computeNormal(Vec3f intersectionPoint) {
		if (preComputedNormal) return normal;
		Vec3f a = v1 - v0; //might need to come back here to make sure the normal is computed properly
		Vec3f b = v2 - v0;
		normal = a.cross(b);
		normal.normalize();

		D = normal.dot(v0);


		preComputedNormal = true;
		return normal;
		
	}

	void flipNormal() {
		normal = normal * -1;
	}

	bool intersect(Vec3f rayOrigin, Vec3f rayDirection, float& p0, float& p1) {
		if (fabs(normal.dot(rayDirection)) < 0.001) return false;
		p0 = -(normal.dot(rayOrigin) - D) / normal.dot(rayDirection);
		//std::cout <<  "Triangle p0: " << p0 << "\n";
		if (p0 < 0) return false;
		Vec3f intersectionPoint = rayOrigin + rayDirection * p0;

		if (preComputedEdges) {
			Vec3f c0 = intersectionPoint - v0;
			Vec3f c1 = intersectionPoint - v1;
			Vec3f c2 = intersectionPoint - v2;
			float t0 = eIn0.dot(c0);
			float t1 = eIn1.dot(c1);
			float t2 = eIn2.dot(c2);
			if (t0 > 0 && t1 > 0 && t2 > 0) return true;
		}
		else {
			e0 = v1 - v0;
			e1 = v2 - v1;
			e2 = v0 - v2;
			Vec3f c0 = intersectionPoint - v0;
			Vec3f c1 = intersectionPoint - v1;
			Vec3f c2 = intersectionPoint - v2;
			eIn0 = normal.cross(e0);
			eIn1 = normal.cross(e1);
			eIn2 = normal.cross(e2);
			float t0 = eIn0.dot(c0);
			float t1 = eIn1.dot(c1);
			float t2 = eIn2.dot(c2);
			preComputedEdges = true;
			if (t0 > 0 && t1 > 0 && t2 > 0) return true;
			
		}

		

		return false;


	}

	Vec3f getNewDirectionTowardsLight(Vec3f shadowRay, Vec3f alignedNormal, Vec3f normalFromLight, float& angleToObject, Vec3f intersectionPoint) {
		//get a random point along both edges of the triangle going away from the first vertex
		Vec3f e0 = v1 - v0;
		Vec3f e1 = v2 - v0;
		float r0 = (float)rand() / RAND_MAX;
		float r1 = (float)rand() / RAND_MAX;
		Vec3f point0 = v0 + e0 * r0;
		Vec3f point1 = v1 + e1 * r1;

		//get the vector between those two random points
		Vec3f e2 = point1 - point0;
		float r2 = (float)rand() / RAND_MAX;

		//get a random point along that vector
		Vec3f randomPoint = point0 + e2 * r2;
		//congrats! you've sampled a random point in the triangle!

		Vec3f newShadowRay = randomPoint - intersectionPoint;
		newShadowRay.normalize();

		angleToObject = 0.9;

		return newShadowRay;
	}

	std::string toString() {
		return "TRIANGLE";
	}
};



#endif
