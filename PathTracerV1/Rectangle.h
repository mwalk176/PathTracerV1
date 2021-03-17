#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "Object.h"

class Rectangle : public Object {
public:
	Vec3f v0, v1, v2, v3;
	float D = 0;
	Vec3f normal;
	bool preComputed = false;
	//Vec3f color = Vec3f(1);
	//Vec3f emissionColor = Vec3f(0);

	Vec3f e0;
	Vec3f e1;
	Vec3f e2;
	Vec3f e3;
	Vec3f eIn0;
	Vec3f eIn1;
	Vec3f eIn2;
	Vec3f eIn3;
	bool preComputedEdges = false;

	Rectangle() : v0(0), v1(0), v2(0), v3(0) { computeNormal(Vec3f(0)); }
	Rectangle(Vec3f p0, Vec3f p1, Vec3f p2, Vec3f p3, Vec3f inColor, Vec3f inEmission) {
		v0 = p0;
		v1 = p1;
		v2 = p2;
		v3 = p3;
		pos = (v0 + v1 + v2 + v3) / 4.0;
		color = inColor;
		emissionColor = inEmission;
		computeNormal(Vec3f(0));
		std::cout << "Rectangle normal: " << normal << "\n";
	}


	Vec3f computeNormal(Vec3f intersectionPoint) {
		if (preComputed) return normal;
		Vec3f a = v1 - v0; //might need to come back here to make sure the normal is computed properly
		Vec3f b = v3 - v0;
		normal = a.cross(b);
		normal.normalize();

		D = normal.dot(v0);


		preComputed = true;
		return normal;

	}

	void flipNormal() {
		normal = normal * -1;
	}

	bool intersect(Vec3f rayOrigin, Vec3f rayDirection, float& p0, float& p1) {
		if (fabs(normal.dot(rayDirection)) < 0.001) return false;
		p0 = -(normal.dot(rayOrigin) - D) / normal.dot(rayDirection);
		//std::cout <<  "Rectangle p0: " << p0 << "\n";
		if (p0 < 0) return false;
		Vec3f intersectionPoint = rayOrigin + rayDirection * p0;

		if (preComputedEdges) {
			Vec3f c0 = intersectionPoint - v0;
			Vec3f c1 = intersectionPoint - v1;
			Vec3f c2 = intersectionPoint - v2;
			Vec3f c3 = intersectionPoint - v3;
			float t0 = eIn0.dot(c0);
			float t1 = eIn1.dot(c1);
			float t2 = eIn2.dot(c2);
			float t3 = eIn3.dot(c3);
			if (t0 > 0 && t1 > 0 && t2 > 0 && t3 > 0) return true;
		}
		else {
			e0 = v1 - v0;
			e1 = v2 - v1;
			e2 = v3 - v2;
			e3 = v0 - v3;
			Vec3f c0 = intersectionPoint - v0;
			Vec3f c1 = intersectionPoint - v1;
			Vec3f c2 = intersectionPoint - v2;
			Vec3f c3 = intersectionPoint - v3;
			eIn0 = normal.cross(e0);
			eIn1 = normal.cross(e1);
			eIn2 = normal.cross(e2);
			eIn3 = normal.cross(e3);
			float t0 = eIn0.dot(c0);
			float t1 = eIn1.dot(c1);
			float t2 = eIn2.dot(c2);
			float t3 = eIn3.dot(c3);
			if (t0 > 0 && t1 > 0 && t2 > 0 && t3 > 0) return true;
			preComputedEdges = true;
		}

		return false;


	}

	Vec3f getNewDirectionTowardsLight(Vec3f shadowRay, Vec3f alignedNormal, Vec3f normalFromLight, float& angleToObject, Vec3f intersectionPoint) {

		return shadowRay;
	}

	std::string toString() {
		return "RECTANGLE";
	}

};





#endif
